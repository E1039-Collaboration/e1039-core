/// OnlMonTrigRS.C
#include <sstream>
#include <iomanip>
#include <cstring>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include <UtilAna/UtilSQHit.h>
#include <UtilAna/UtilHist.h>
#include <rs_Reader/rs_Reader.h>
#include "OnlMonTrigRS.h"
using namespace std;

OnlMonTrigRS::OnlMonTrigRS(const char* rs_top_0, const char* rs_top_1, const char* rs_bot_0, const char* rs_bot_1)
{
  NumCanvases(4);
  Name("OnlMonTrigRS" ); 
  Title("Road Set Trigger Analysis" );

  is_rs_t[0] = (strcmp(rs_top_0,"") == 0) ? false : true;
  is_rs_t[1] = (strcmp(rs_top_1,"") == 0) ? false : true;
  is_rs_b[0] = (strcmp(rs_bot_0,"") == 0) ? false : true;
  is_rs_b[1] = (strcmp(rs_bot_1,"") == 0) ? false : true;
 
  rs_top_0_ = rs_top_0;
  rs_top_1_ = rs_top_1;
  rs_bot_0_ = rs_bot_0;
  rs_bot_1_ = rs_bot_1;

  rs_path = "/seaquest/users/hazeltet/E1039_ana/e1039-core/online/onlmonserver/rs/";
  
  char result[150];   // array to hold the result.

  if(is_rs_t[0]){
    strcpy(result,rs_path);
    rs_top[0] = new rs_Reader(strcat(result,rs_top_0_));
  }
  if(is_rs_t[1]){
    strcpy(result,rs_path);
    rs_top[1] = new rs_Reader(strcat(result,rs_top_1_));
  }
  if(is_rs_b[0]){
    strcpy(result,rs_path);
    rs_bot[0] = new rs_Reader(strcat(result,rs_bot_0_));
  }
  if(is_rs_b[1]){
    strcpy(result,rs_path);
    rs_bot[1] = new rs_Reader(strcat(result,rs_bot_1_));
  }
  event_cnt = 1;
}

int OnlMonTrigRS::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigRS::InitRunOnlMon(PHCompositeNode* topNode)
{
  SetDet();

  GeomSvc* geom = GeomSvc::instance();
  ostringstream oss;
  
  int num_tot_ele = 0;
  //Loop through hodoscopes 
  for (int i_det = 0; i_det < N_DET; i_det++) {
    string name = list_det_name[i_det];
    int  det_id = list_det_id  [i_det];
    int n_ele  = geom->getPlaneNElements(det_id);
    num_tot_ele += n_ele;
    if (det_id <= 0 || n_ele <= 0) {
      cout << "OnlMonTrigRS::InitRunOnlMon():  Invalid det_id or n_ele: " 
           << det_id << " " << n_ele << " at name = " << name << "." << endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }

  }
    int rs_hist_range;
    for(int i = 0; i < 2; i++){ 
      
      oss.str("");
      oss << "h1_rs_top_mult_" << i;
      h1_rs_top_mult[i] = new TH1D(oss.str().c_str(), "",1001,-0.5, 1001 + 0.5);
      oss.str("");
      if(i == 0){
        oss << rs_top_0_ << " Multiplicity" << ";Road ID;N of Hits";
      }else{
        oss << rs_top_1_ << " Multiplicity" << ";Road ID;N of Hits";
      }
      h1_rs_top_mult[i]->SetTitle(oss.str().c_str());

      rs_hist_range = (is_rs_t[i]) ? rs_top[i]->roads.size() : 100;
      oss.str("");
      oss << "h1_rs_top_" << i;
      h1_rs_top[i] = new TH1D(oss.str().c_str(), "",rs_hist_range + 1,-0.5, rs_hist_range + 0.5);
      oss.str("");
      if(i == 0){
        oss << rs_top_0_ << ";Road ID;N of Hits";
      }else{
        oss << rs_top_1_ << ";Road ID;N of Hits";
      }
      h1_rs_top[i]->SetTitle(oss.str().c_str());

      rs_hist_range = (is_rs_b[i]) ? rs_bot[i]->roads.size() : 100;
      oss.str("");
      oss << "h1_rs_bot_" << i;
      h1_rs_bot[i] = new TH1D(oss.str().c_str(), "", rs_hist_range + 1,-0.5, rs_hist_range + 0.5);
      oss.str("");
      if(i == 0){
        oss << rs_bot_0_ << ";Road ID;N of Hits";
      }else{
        oss << rs_bot_1_ << ";Road ID;N of Hits";
      }
      h1_rs_bot[i]->SetTitle(oss.str().c_str());

      oss.str("");
      oss << "h1_rs_bot_mult_" << i;
      h1_rs_bot_mult[i] = new TH1I(oss.str().c_str(), "",1001,-0.5, 1001 + 0.5);
      oss.str("");
      if(i == 0){
        oss << rs_bot_0_ << " Multiplicity" << ";Road ID;N of Hits";
      }else{
        oss << rs_bot_1_ << " Multiplicity" << ";Road ID;N of Hits";
      }
      h1_rs_bot_mult[i]->SetTitle(oss.str().c_str());

      RegisterHist(h1_rs_top_mult[i]);
      RegisterHist(h1_rs_bot_mult[i]);
      RegisterHist(h1_rs_top[i]);
      RegisterHist(h1_rs_bot[i]);
    }
  
 // cout << "REGISTERING HISTOGRAMS" << endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigRS::ProcessEventOnlMon(PHCompositeNode* topNode)
{ 

  SQEvent*      evt     = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*  hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  SQHitVector*  trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!evt || !hit_vec  || !trig_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  //Determine whether event is FPGA1-4 
  int is_FPGA_event = (evt->get_trigger(SQEvent::MATRIX1) || evt->get_trigger(SQEvent::MATRIX2) ||evt->get_trigger(SQEvent::MATRIX3)||evt->get_trigger(SQEvent::MATRIX4) ) ? 1 : 0; 

//ROAD ID Logic  *************************************************************************** 
//TOP####
  if(is_FPGA_event){
    auto vecH1T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[0]);
    auto vecH2T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[2]);
    auto vecH3T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[4]);
    auto vecH4T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[6]);
    for(int j = 0; j < 2; j++){
      if(is_rs_t[j]){
        RoadHits(vecH1T,vecH2T,vecH3T,vecH4T,rs_top[j],h1_rs_top[j],h1_rs_top_mult[j]);
      }
    }
//BOTTOM####
    auto vecH1B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[1]);
    auto vecH2B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[3]);
    auto vecH3B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[5]);
    auto vecH4B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[7]);
    for(int j = 0; j < 2; j++){
      if(is_rs_b[j]){
        RoadHits(vecH1B,vecH2B,vecH3B,vecH4B,rs_bot[j],h1_rs_bot[j],h1_rs_bot_mult[j]);
      }
    } 
  }
  if(event_cnt == 10000 || event_cnt == 1){
 //   cout << "NEW EVENT " << event_cnt << endl;
  }
  event_cnt++;
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigRS::EndOnlMon(PHCompositeNode* topNode)
{ 
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigRS::FindAllMonHist()
{

 // cout << "FIND ALL MON HIST PART" << endl;
  ostringstream oss;
  
  for(int i = 0; i < 2; i++){
    oss.str("");
    oss << "h1_rs_top_" << i;
    h1_rs_top[i] = FindMonHist(oss.str().c_str());
    if (! h1_rs_top[i]) return 1;

    oss.str("");
    oss << "h1_rs_bot_" << i;
    h1_rs_bot[i] = FindMonHist(oss.str().c_str());
    if (! h1_rs_bot[i]) return 1;

    oss.str("");
    oss << "h1_rs_top_mult_" << i;
    h1_rs_top_mult[i] = FindMonHist(oss.str().c_str());
    if (! h1_rs_top_mult[i]) return 1;

    oss.str("");
    oss << "h1_rs_bot_mult_" << i;
    h1_rs_bot_mult[i] = FindMonHist(oss.str().c_str());
    if (! h1_rs_bot_mult[i]) return 1;

  }
  return 0;
}

int OnlMonTrigRS::DrawMonitor()
{
  //DRAWING HISTOGRAMS ON .PNG FILES ******************************************
 // cout << "DRAWING HISTOGRAMS ON PNG" << endl;

  OnlMonCanvas* can6 = GetCanvas(0);
  TPad* pad6 = can6->GetMainPad();
  pad6->Divide(1,2);
  TVirtualPad* pad60 = pad6->cd(1);
  pad60->SetGrid();
  h1_rs_top[0]->SetLineColor(kBlue);
  h1_rs_top[0]->Draw();

  TVirtualPad* pad61 = pad6->cd(2);
  pad61->SetGrid();
  h1_rs_top_mult[0]->SetLineColor(kBlue);
  h1_rs_top_mult[0]->Draw();

  OnlMonCanvas* can7 = GetCanvas(1);
  TPad* pad7 = can7->GetMainPad();
  pad7->Divide(1,2);
  TVirtualPad* pad70 = pad7->cd(1);
  pad70->SetGrid();
  h1_rs_bot[0]->SetLineColor(kBlue);
  h1_rs_bot[0]->Draw();

  TVirtualPad* pad71 = pad7->cd(2);
  pad71->SetGrid();
  h1_rs_bot_mult[0]->SetLineColor(kBlue);
  h1_rs_bot_mult[0]->Draw();

  OnlMonCanvas* can8 = GetCanvas(2);
  TPad* pad8 = can8->GetMainPad();
  pad8->Divide(1,2);
  TVirtualPad* pad80 = pad8->cd(1);
  pad80->SetGrid();
  h1_rs_top[1]->SetLineColor(kBlue);
  h1_rs_top[1]->Draw();

  TVirtualPad* pad81 = pad8->cd(2);
  pad81->SetGrid();
  h1_rs_top_mult[1]->SetLineColor(kBlue);
  h1_rs_top_mult[1]->Draw();

  OnlMonCanvas* can9 = GetCanvas(3);
  TPad* pad9 = can9->GetMainPad();
  pad9->Divide(1,2);
  TVirtualPad* pad90 = pad9->cd(1);
  pad90->SetGrid();
  h1_rs_bot[1]->SetLineColor(kBlue);
  h1_rs_bot[1]->Draw();

  TVirtualPad* pad91 = pad9->cd(2);
  pad91->SetGrid();
  h1_rs_bot_mult[1]->SetLineColor(kBlue);
  h1_rs_bot_mult[1]->Draw();

  return 0;
}

void OnlMonTrigRS::SetDet()
{
  list_det_name[0] = "H1T";
  list_det_name[1] = "H1B";
  list_det_name[2] = "H2T";
  list_det_name[3] = "H2B";
  list_det_name[4] = "H3T";
  list_det_name[5] = "H3B";
  list_det_name[6] = "H4T";
  list_det_name[7] = "H4B";
   
  GeomSvc* geom = GeomSvc::instance();
  for (int ii = 0; ii < N_DET; ii++) {
    list_det_id[ii] = geom->getDetectorID(list_det_name[ii]);
  }
}

void OnlMonTrigRS::RoadHits(vector<SQHit*>* H1X, vector<SQHit*>* H2X, vector<SQHit*>* H3X, vector<SQHit*>* H4X,rs_Reader* rs_obj, TH1* hist_rs, TH1* hist_mult)
{

  int count_rd = 0;
  
  for(size_t i=0; i<rs_obj->roads.size();i++){
    
    for (auto it = H1X->begin(); it != H1X->end(); it++) {
      if ((*it)->get_level() != 1) continue; //switched m_lvl for 1
      int eleH1X  = (*it)->get_element_id();
      //double time = (*it)->get_tdc_time();
    
      for (auto it1 = H2X->begin(); it1 != H2X->end(); it1++) {
        if ((*it1)->get_level() != 1) continue; //switched m_lvl for 1
        int eleH2X  = (*it1)->get_element_id();
        //double time = (*it)->get_tdc_time();
    
        for (auto it2 = H3X->begin(); it2 != H3X->end(); it2++) {
          if ((*it2)->get_level() != 1) continue; //switched m_lvl for 1
          int eleH3X  = (*it2)->get_element_id();
          //double time = (*it)->get_tdc_time();  
    
          for (auto it0 = H4X->begin(); it0 != H4X->end(); it0++) {
            if ((*it0)->get_level() != 1) continue; //switched m_lvl for 1
            int eleH4X  = (*it0)->get_element_id();
            //double time = (*it)->get_tdc_time();
            int is_H1X = ((eleH1X == rs_obj->roads[i].H1X) || rs_obj->roads[i].H1X == -1 ) ? 1 : 0;
            int is_H2X = ((eleH2X == rs_obj->roads[i].H2X) || rs_obj->roads[i].H2X == -1 ) ? 1 : 0;
            int is_H3X = ((eleH3X == rs_obj->roads[i].H3X) || rs_obj->roads[i].H3X == -1 ) ? 1 : 0;
            int is_H4X = ((eleH4X == rs_obj->roads[i].H4X) || rs_obj->roads[i].H4X == -1 ) ? 1 : 0;
   
            if(is_H1X && is_H2X && is_H3X && is_H4X){
              hist_rs->Fill(rs_obj->roads[i].road_id);         
              //cout << rs_top_0.get_road_id[i] << endl;
              count_rd++;
            }
            if(rs_obj->roads[i].H4X == -1){break;} 
          }
          if(rs_obj->roads[i].H3X == -1){break;}
        }
        if(rs_obj->roads[i].H2X == -1){break;}
      }
      if(rs_obj->roads[i].H1X == -1){break;}
    }
  }
  if(count_rd != 0){
      cout << "Number of roads: " << count_rd << endl;
     hist_mult->Fill(count_rd);
  }
  //hist_mult->Fill(count_rd);

}
 
