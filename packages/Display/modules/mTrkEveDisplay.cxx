/*!
        \file mTrkEveDisplay.cxx
        \author Sookhyun Lee
        \brief reconstructed charged tracks and their clusters
        \version $Revision: 1.1 $
        \date    $Date: 07/26/2016
*/

// STL and BOOST includes
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <boost/bind.hpp>

// PHENIX includes
#include <phool/PHCompositeNode.h>
//#include <PHPoint.h>
#include <phool/getClass.h>

// ROOT and EVE includes
#include <TEveManager.h>
#include <TEveTrackPropagator.h>
#include <TEveTrack.h>
#include <TEvePointSet.h>
#include <TEveElement.h>
#include <TEveCaloData.h>
#include <TEveCalo.h>
#include <TEveQuadSet.h>
#include <TEveTrans.h>
#include <TH2F.h>
#include <TNamed.h>

#include <interface_main/SQHitVector.h>
#include <interface_main/SQHit.h>
#include <ktracker/SRecEvent.h>
#include <geom_svc/GeomSvc.h>

#include <pheve_display/PHEveDisplay.h>

#include "mTrkEveDisplay.h"

using boost::bind;


mTrkEveDisplay::mTrkEveDisplay(boost::shared_ptr<PHEveDisplay> dispin) :
  mPHEveModuleBase(),
  _evedisp(dispin),
  _prop(NULL),
  _reco_tracks(NULL)
{

  verbosity = _evedisp->get_verbosity();
  _evemanager = _evedisp->get_eve_manager();
  _prop = _evedisp->get_cnt_prop();
  _reco_tracks = new TEveTrackList("Svtx Tracks");

  _geom_svc = GeomSvc::instance();
  TEveRGBAPalette *pal = new TEveRGBAPalette(0, 100, true, true, true);
  for(int i=0; i<NDETPLANES; ++i) {
    _hit_wires[i] = new TEveQuadSet(std::to_string(i).c_str());
    _hit_wires[i]->SetOwnIds(kTRUE);
    //_hit_wires[i]->SetPalette(pal);
    _hit_wires[i]->Reset(TEveQuadSet::kQT_LineXYFixedZ, true, 32);
    _hit_wires[i]->RefitPlex();
    _hit_wires[i]->SetPickable(true);
    TEveTrans& t =  _hit_wires[i]->RefMainTrans();
    t.SetPos(0, 0, _geom_svc->getPlaneCenterZ(i));
    if(i<=30)       _evemanager->AddElement(_hit_wires[i],_evedisp->get_dc_list());
    else if(i<=46)  _evemanager->AddElement(_hit_wires[i],_evedisp->get_hodo_list());
    else if(i<=54)  _evemanager->AddElement(_hit_wires[i],_evedisp->get_prop_list());
    else if(i<=62)  _evemanager->AddElement(_hit_wires[i],_evedisp->get_dp_list());
  }

  _evemanager->AddElement(_reco_tracks,_evedisp->get_top_list());

}

mTrkEveDisplay::~mTrkEveDisplay()
{
}

void
mTrkEveDisplay::init(PHCompositeNode* topNode)
{  
}

void 
mTrkEveDisplay::init_run(PHCompositeNode* topNode)
{
  //_geom_svc = GeomSvc::instance();
}

bool
mTrkEveDisplay::event(PHCompositeNode* topNode)
{
  clear();

  if (verbosity)
    std::cout << "mTrkEveDisplay - event() begins." << std::endl;

  try {
    get_nodes(topNode);

    if(verbosity > 0) LogInfo("draw_hits()");
    draw_hits();

    if(verbosity > 0) LogInfo("draw_tracks()");
    draw_tracks();
  } catch (std::exception& e) {
    static bool first(true);
    if (first)
      std::cout << "mTrkEveDisplay::event Exception: " << e.what() << std::endl;
    first = false;
  }

  return true;

}

void
mTrkEveDisplay::end(PHCompositeNode* topNode)
{
}

void 
mTrkEveDisplay::get_nodes(PHCompositeNode* topNode)
{
  _sqhit_col = findNode::getClass<SQHitVector>(topNode,"SQHitVector");
  if(!_sqhit_col) std::cout<< "SQHitVector node not found!!"<<std::endl;
  if(verbosity) std::cout<<"mTrkEveDisplay - SQHitVector nodes found."<<std::endl;

  _recEvent = findNode::getClass<SRecEvent>(topNode, "SRecEvent");
  if(!_recEvent) std::cout<< "SRecEvent node not found!!"<<std::endl;
  if(verbosity) std::cout<<"mTrkEveDisplay - SRecEvent nodes found."<<std::endl;
}

int mTrkEveDisplay::hit_to_wire(const int det_id, const int elm_id, double& x, double& y, double& dx, double& dy) {

  if (!_geom_svc) return -1;

  //double pos = _geom_svc->getMeasurement(det_id, elm_id) - _geom_svc->getPlane(det_id).deltaW - _geom_svc->getPlane(det_id).xoffset;
  double pos = _geom_svc->getMeasurement(det_id, elm_id);

  double cos_theta = _geom_svc->getCostheta(det_id);
  double sin_theta = _geom_svc->getSintheta(det_id);
  double n[2] = {cos_theta, sin_theta};

  // Some ref point
  double xc = 0; //_geom_svc->getPlane(det_id).x0;
  double yc = 0; //_geom_svc->getPlane(det_id).y0;
  double vc[2] = {xc, yc};

  double y1 = _geom_svc->getPlane(det_id).y1;
  double y2 = _geom_svc->getPlane(det_id).y2;
  double x1 = _geom_svc->getPlane(det_id).x1;
  double x2 = _geom_svc->getPlane(det_id).x2;

  // (v - v_c) dot n = pos
  // v0*n0 +v1*n1 = pos + v_c0*n0+v_c1*n1
  if(fabs(n[0])<0.01) { // horizontal wire
    y1 = (pos + xc*n[0] + yc*n[1] - x1*n[0])/n[1];
    y2 = (pos + xc*n[0] + yc*n[1] - x2*n[0])/n[1];
  } else { // non-horizontal wire
    x1 = (pos + xc*n[0] + yc*n[1] - y1*n[1])/n[0];
    x2 = (pos + xc*n[0] + yc*n[1] - y2*n[1])/n[0];
  }

  x = x1;
  y = y1;
  dx = x2 - x1;
  dy = y2 - y1;

  return 0;
}


void
mTrkEveDisplay::draw_hits()
{
  if(!_sqhit_col) {
    if(verbosity > 0)
      LogInfo(!_sqhit_col);
    return;
  }

  for (SQHitVector::ConstIter it = _sqhit_col->begin(); it != _sqhit_col->end(); it++) {
    try {
      SQHit * hit = *it;
      if(!hit) {
        if(verbosity > 1) LogInfo("!hit");
        continue;
      }

      auto det_id = hit->get_detector_id();
      auto elm_id = hit->get_element_id();
      double x, y, dx, dy;

      if(verbosity > 1) {
        std::cout
        << "mTrkEveDisplay::draw_hits: {"
        << det_id << ", " << elm_id << "} "
        << std::endl;
      }

      if(det_id <= 0 or det_id >= NDETPLANES) {
         if(verbosity > 2) LogInfo(det_id);
         continue;
      }

      int ret_code = hit_to_wire(det_id, elm_id, x, y, dx, dy);
      if(ret_code!=0) {
        if(verbosity > 1) LogInfo("hit_to_wire failed");
        continue;
      }

      const float geom_limit = 1000;
      if(
          !(fabs(x) < geom_limit) or
          !(fabs(y) < geom_limit) or
          !(fabs(dx) < geom_limit) or
          !(fabs(dy) < geom_limit)
          ) {
         if(verbosity > 2) LogInfo(" {" << x << ", " << y << ", " << dx << ", " << dy << "}");
         continue;
      }

      if(verbosity > 1) {
        std::cout
        << "mTrkEveDisplay::draw_hits: {"
        << det_id << ", " << elm_id << "} => "
        << " {" << x << ", " << y << ", " << dx << ", " << dy << "}"
        << std::endl;
      }

      _hit_wires[det_id]->AddLine(x, y, dx, dy);

      _hit_wires[det_id]->QuadId(new TNamed(TString::Format("%d",elm_id).Data(),"element id"));

      if(det_id<31)      _hit_wires[det_id]->DigitColor(kBlue);
      else if(det_id<47) _hit_wires[det_id]->DigitColor(kGreen);
      else if(det_id<55) _hit_wires[det_id]->DigitColor(kRed);
      else if(det_id<63) _hit_wires[det_id]->DigitColor(kMagenta);
    } catch (std::exception &e ) {
      std::cout << "Exception caught mTrkEveDisplay::draw_hits " << e.what() << std::endl;
    }
  }
}
void
mTrkEveDisplay::draw_tracks()
{
  if(!_recEvent) {
    if(verbosity > 0)
      LogInfo(!_recEvent);
    return;
  }

  if(verbosity > 0)
    LogInfo(_recEvent->getNTracks());
  if(_recEvent->getNTracks()<=0) return;

  _geom_svc = GeomSvc::instance();
  double vz_st1 = _geom_svc->getPlaneCenterZ(_geom_svc->getDetectorID("D1X"));

  std::map<int, int> hitID_ihit;
  // If using vector, index it first
  for(int ihit=0; ihit<_sqhit_col->size(); ++ihit) {
    SQHit *hit = _sqhit_col->at(ihit);
    int hitID = hit->get_hit_id();
    hitID_ihit[hitID] = ihit;
  }

  for(int itrack=0; itrack<_recEvent->getNTracks(); ++itrack){
    SRecTrack srecTrack = _recEvent->getTrack(itrack);

    TEveRecTrackT<double>* eve_rec_trk = new TEveRecTrackT<double>();

    int charge = srecTrack.getCharge();

    double vx, vy, vz;
    double px, py, pz;
    srecTrack.getMomentumSt1(px, py, pz);
    srecTrack.getPositionSt1(vx, vy);
    vz = vz_st1;

    eve_rec_trk->fV.Set(vx, vy, vz);
    eve_rec_trk->fP.Set(px, py, pz);

    if(verbosity) {
      std::cout << "--------- itrack: " << itrack << " ---------"<< std::endl;
      std::cout
      << " { " << vx << "," << vy << "," << vz << " } "
      << " { " << px << "," << py << "," << pz << " } "
      << std::endl;
    }


    TVector3 rec_vtx = srecTrack.getTargetPos();
    TVector3 rec_mom = srecTrack.getTargetMom();
    eve_rec_trk->fV.Set(rec_vtx.x(), rec_vtx.y(), rec_vtx.z());
    eve_rec_trk->fP.Set(rec_mom.x(), rec_mom.y(), rec_mom.z());

    eve_rec_trk->fSign = charge;
    TEveTrack* trk = new TEveTrack(eve_rec_trk, _prop);
    trk->SetSmooth(kTRUE);
    if (charge > 0) {
      trk->SetPdg(-13);
      trk->SetLineColor(kYellow);
    }
    else {
      trk->SetPdg(13);
      trk->SetLineColor(kCyan);
    }
    trk->SetLineWidth(1);

    for (int iz = 2; iz < 26; ++iz) {
      double z = iz * 100;
      double x, y;
      srecTrack.getExpPositionFast(z, x, y);

      trk->AddPathMark(TEvePathMarkD(
          TEvePathMarkD::kDaughter,
          TEveVectorD(x, y, z)
          ));
    }

//    for(int ihit=0; ihit<srecTrack.getNHits();++ihit) {
//      int hitID = srecTrack.getHitIndex(ihit);
//
//      // signed hitID to hitID
//      hitID = abs(hitID);
//
//      if(hitID_ihit.find(hitID)==hitID_ihit.end()) continue;
//
//      SQHit *hit = _sqhit_col->at(hitID_ihit[hitID]);
//      if(!hit) {LogInfo("!hit");}
//
//      double wx, wy, wz;
//      double wdx, wdy;
//
//      auto det_id = hit->get_detector_id();
//      //if(det_id>30) continue;
//      auto elm_id = hit->get_element_id();
//      double x, y, dx, dy;
//      hit_to_wire(det_id, elm_id, wx, wy, wdx, wdy);
//      wz = _geom_svc->getPlaneCenterZ(det_id);
//
//      trk->AddPathMark(
//          TEvePathMarkD(TEvePathMarkD::kLineSegment,
//              TEveVectorD(wx, wy, wz),
//              TEveVectorD(0, 0, 1),
//              TEveVectorD(wdx, wdy, 0)));
//    }

    trk->SetRnrPoints(kTRUE);
    trk->SetMarkerStyle(1);
    trk->SetMarkerSize(1);
    if (charge > 0)
      trk->SetMarkerColor(kYellow);
    else
      trk->SetMarkerColor(kCyan);

    trk->MakeTrack();
    if (verbosity > 2)
      std::cout << "mTrkEveDisplay - track made. " << std::endl;
    _reco_tracks->AddElement(trk);
  }
}

bool 
mTrkEveDisplay::pid_cut(int pid)
{
if(fabs(pid)==211 || fabs(pid)==321 || fabs(pid)==11 || fabs(pid)==13 || fabs(pid)==15 || fabs(pid)==17) return true;

if (pid == 0) return true;//always return true
return false;
}


void 
mTrkEveDisplay::clear()
{
  _prop->IncDenyDestroy(); // Keep the track propagator from being destroyed

  for(int i=0; i<NDETPLANES; ++i) {
    //_hit_wires[i]->DestroyElements();
    _hit_wires[i]->Reset(TEveQuadSet::kQT_LineXYFixedZ, true, 32);
    _hit_wires[i]->RefitPlex();
    TEveTrans& t =  _hit_wires[i]->RefMainTrans();
    t.SetPos(0, 0, _geom_svc->getPlaneCenterZ(i));
  }
  _reco_tracks->DestroyElements();
}
