#include <iostream>
#include <phool/PHTimer.h>
#include <phool/recoConsts.h>
#include <fun4all/Fun4AllBase.h>
#include <phfield/PHField.h>
#include "KalmanFastTrackletting.h"
using namespace std;

KalmanFastTrackletting::KalmanFastTrackletting(const PHField* field, const TGeoManager* geom, bool flag, const int verb)
  : KalmanFastTracking(field, geom, flag, verb)
{
  recoConsts* rc = recoConsts::instance();
  TX_MAX = rc->get_DoubleFlag("TX_MAX");
  TY_MAX = rc->get_DoubleFlag("TY_MAX");
  X0_MAX = rc->get_DoubleFlag("X0_MAX");
  Y0_MAX = rc->get_DoubleFlag("Y0_MAX");
}

KalmanFastTrackletting::~KalmanFastTrackletting()
{
  ;
}

int KalmanFastTrackletting::setRawEvent(SRawEvent* event_input)
{
  int ret = setRawEventPrep(event_input);
  if (ret != 0) return ret;
  
  //Build tracklets in station 2, 3+, 3-
  _timers["st2"]->restart();
  buildTrackletsInStation(3, 1);   //3 for station-2, 1 for list position 1
  _timers["st2"]->stop();
  if(verbosity >= 2) LogInfo("NTracklets in St2: " << trackletsInSt[1].size());
  
  _timers["st3"]->restart();
  buildTrackletsInStation(4, 2);   //4 for station-3+
  buildTrackletsInStation(5, 2);   //5 for station-3-
  _timers["st3"]->stop();
  if(verbosity >= 2) LogInfo("NTracklets in St3: " << trackletsInSt[2].size());

  _timers["global_st1"]->restart(); // Not global but be diverted
  buildTrackletsInStation(1, 0); // 1 for D0
  _timers["global_st1"]->stop();
  if(verbosity >= 2) LogInfo("NTracklets in St1: " << trackletsInSt[0].size());

  //Build back partial tracks in station 2, 3+ and 3-
  _timers["st23"]->restart();
  buildBackPartialTracks();
  _timers["st23"]->stop();
  if(verbosity >= 2) LogInfo("NTracklets St2+St3: " << trackletsInSt[3].size());
  
  if(verbosity >= 3) {
    for(int i = 0; i <= 4; i++)
    {
      std::cout << "=======================================================================================" << std::endl;
      LogInfo("Tracklets in station: " << i+1 << " is " << trackletsInSt[i].size());
      for(std::list<Tracklet>::iterator tracklet = trackletsInSt[i].begin(); tracklet != trackletsInSt[i].end(); ++tracklet)
      {
        tracklet->print();
      }
      std::cout << "=======================================================================================" << std::endl;
    }
  }
  
  return TFEXIT_SUCCESS;
}

void KalmanFastTrackletting::buildTrackletsInStation(int stationID, int listID, double* pos_exp, double* window)
{
  //actuall ID of the tracklet lists
  int sID = stationID - 1;

  //Extract the X, U, V hit pairs
  std::list<SRawEvent::hit_pair> pairs_X, pairs_U, pairs_V;
  if(pos_exp == nullptr)
  {
    pairs_X = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][0]);
    pairs_U = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][1]);
    pairs_V = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][2]);
  }
  else
  {
    //Note that in pos_exp[], index 0 stands for X, index 1 stands for U, index 2 stands for V
    pairs_X = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][0], pos_exp[0], window[0]);
    pairs_U = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][1], pos_exp[1], window[1]);
    pairs_V = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][2], pos_exp[2], window[2]);
  }
  
  if(pairs_X.empty() || pairs_U.empty() || pairs_V.empty()) return;

  //X-U combination first, then add V pairs
  for(std::list<SRawEvent::hit_pair>::iterator xiter = pairs_X.begin(); xiter != pairs_X.end(); ++xiter)
  {
    bool has_x_2nd = xiter->second >= 0;
    double x_pos = has_x_2nd ? 0.5*(hitAll[xiter->first].pos + hitAll[xiter->second].pos) : hitAll[xiter->first].pos;
    double u_min = x_pos*u_costheta[sID] - u_win[sID];
    double u_max = u_min + 2.*u_win[sID];
    
    for(std::list<SRawEvent::hit_pair>::iterator uiter = pairs_U.begin(); uiter != pairs_U.end(); ++uiter)
    {
      bool has_u_2nd = uiter->second >= 0;
      double u_pos = has_u_2nd ? 0.5*(hitAll[uiter->first].pos + hitAll[uiter->second].pos) : hitAll[uiter->first].pos;
      if(u_pos < u_min || u_pos > u_max) continue;
      
      //V projections from X and U plane
      double z_x = has_x_2nd ? z_plane_x[sID] : z_plane[hitAll[xiter->first].detectorID];
      double z_u = has_u_2nd ? z_plane_u[sID] : z_plane[hitAll[uiter->first].detectorID];
      double z_v = z_plane_v[sID];
      double v_win1 = spacing_plane[hitAll[uiter->first].detectorID]*2.*u_costheta[sID];
      double v_win2 = fabs((z_u + z_v - 2.*z_x)*u_costheta[sID]*TX_MAX);
      double v_win3 = fabs((z_v - z_u)*u_sintheta[sID]*TY_MAX);
      double v_win = v_win1 + v_win2 + v_win3 + 2.*spacing_plane[hitAll[uiter->first].detectorID];
      double v_min = 2*x_pos*u_costheta[sID] - u_pos - v_win;
      double v_max = v_min + 2.*v_win;

      for(std::list<SRawEvent::hit_pair>::iterator viter = pairs_V.begin(); viter != pairs_V.end(); ++viter)
      {
        bool has_v_2nd = viter->second >= 0;
        double v_pos = has_v_2nd ? 0.5*(hitAll[viter->first].pos + hitAll[viter->second].pos) : hitAll[viter->first].pos;
        if(v_pos < v_min || v_pos > v_max) continue;

        //Now add the tracklet, using all L-R combinations.
        for (int LR_X1 =              -1      ; LR_X1 <= +1; LR_X1 += 2) {
        for (int LR_X2 = (has_x_2nd ? -1 : +1); LR_X2 <= +1; LR_X2 += 2) {
        for (int LR_U1 =              -1      ; LR_U1 <= +1; LR_U1 += 2) {
        for (int LR_U2 = (has_u_2nd ? -1 : +1); LR_U2 <= +1; LR_U2 += 2) {
        for (int LR_V1 =              -1      ; LR_V1 <= +1; LR_V1 += 2) {
        for (int LR_V2 = (has_v_2nd ? -1 : +1); LR_V2 <= +1; LR_V2 += 2) {
          Tracklet tracklet_new;
          tracklet_new.stationID = stationID;
        
          tracklet_new.hits.push_back(SignedHit(hitAll[xiter->first], LR_X1));
          tracklet_new.nXHits++;

          if(has_x_2nd) {
            tracklet_new.hits.push_back(SignedHit(hitAll[xiter->second], LR_X2));
            tracklet_new.nXHits++;
          }
        
          tracklet_new.hits.push_back(SignedHit(hitAll[uiter->first], LR_U1));
          tracklet_new.nUHits++;

          if(has_u_2nd) {
            tracklet_new.hits.push_back(SignedHit(hitAll[uiter->second], LR_U2));
            tracklet_new.nUHits++;
          }
        
          tracklet_new.hits.push_back(SignedHit(hitAll[viter->first], LR_V1));
          tracklet_new.nVHits++;

          if(has_v_2nd) {
            tracklet_new.hits.push_back(SignedHit(hitAll[viter->second], LR_V2));
            tracklet_new.nVHits++;
          }
        
          tracklet_new.sortHits();
          if(tracklet_new.isValid() == 0) {
            fitTracklet(tracklet_new);
            trackletsInSt[listID].push_back(tracklet_new);
          }
        }}}}}}
      }
    }
  }
  
  for(std::list<Tracklet>::iterator iter = trackletsInSt[listID].begin(); iter != trackletsInSt[listID].end(); ++iter)
  {
    iter->addDummyHits();
  }
}
