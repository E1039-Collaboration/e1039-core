/// OnlMonTrigEP.C
#include <sstream>
#include <iomanip>
#include <cstring>
#include <TSystem.h>
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
#include "OnlMonTrigEP.h"
using namespace std;

//Arguments = name of road set .txt files
OnlMonTrigEP::OnlMonTrigEP(const std::string rs_top_0, const std::string rs_top_1, const std::string rs_bot_0, const std::string rs_bot_1, const std::string rs_path)
  : rs_top_0_(rs_top_0)
  , rs_top_1_(rs_top_1)
  , rs_bot_0_(rs_bot_0)
  , rs_bot_1_(rs_bot_1)
  , rs_path_ (rs_path)
  , top   (0) // should be const?
  , bottom(1) // should be const?
{
  NumCanvases(1);
  Name("OnlMonTrigEP" ); 
  Title("FPGA1 Purity & Efficiency" );

  rs_top_check_p[0] = 0;
  rs_top_check_p[1] = 0;
  rs_bot_check_p[0] = 0;
  rs_bot_check_p[1] = 0;

  rs_top_check_e[0] = 0;
  rs_top_check_e[1] = 0;
  rs_bot_check_e[0] = 0;
  rs_bot_check_e[1] = 0;

  is_rs_t[0] = rs_top_0 != "";
  is_rs_t[1] = rs_top_1 != "";
  is_rs_b[0] = rs_bot_0 != "";
  is_rs_b[1] = rs_bot_1 != "";

  if (rs_path_ == "") {
    rs_path_ = gSystem->Getenv("E1039_RESOURCE");
    rs_path_ += "/trigger/rs";
  }

  //calling rs_Reader class for each road set file
  if(is_rs_t[0]) rs_top[0] = new rs_Reader( rs_path_+"/"+rs_top_0_ );
  if(is_rs_t[1]) rs_top[1] = new rs_Reader( rs_path_+"/"+rs_top_1_ );
  if(is_rs_b[0]) rs_bot[0] = new rs_Reader( rs_path_+"/"+rs_bot_0_ );
  if(is_rs_b[1]) rs_bot[1] = new rs_Reader( rs_path_+"/"+rs_bot_1_ );
}

int OnlMonTrigEP::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigEP::InitRunOnlMon(PHCompositeNode* topNode)
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
      cout << "OnlMonTrigEP::InitRunOnlMon():  Invalid det_id or n_ele: " 
           << det_id << " " << n_ele << " at name = " << name << "." << endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }

  }
 
  oss.str("");
  oss << "h1_purity_" << 0;
  h1_purity = new TH1D(oss.str().c_str(), "", 2, -0.5, 1.5);
  oss.str("");
  oss << "FGPA1 Purity" << ";;Hit count";
  h1_purity->SetTitle(oss.str().c_str());


  h1_purity->GetXaxis()->SetBinLabel( 2, "FPGA1 && rd hit");
  h1_purity->GetXaxis()->SetBinLabel( 1, "FPGA1 && no rd hit");

  RegisterHist(h1_purity);

  oss.str("");
  oss << "h1_eff_NIM4_" << 0;
  h1_eff_NIM4 = new TH1D(oss.str().c_str(), "", 2, -0.5, 1.5);
  oss.str("");
  oss << "FPGA1 Efficiency (NIM4 + Event type)" << ";;Hit count";
  h1_eff_NIM4->SetTitle(oss.str().c_str());
  
  h1_eff_NIM4->GetXaxis()->SetBinLabel( 2, "NIM4 && rd hit && FPGA1");
  h1_eff_NIM4->GetXaxis()->SetBinLabel( 1, "NIM4 && rd hit && NO FPGA1");
  
  RegisterHist(h1_eff_NIM4);
 
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigEP::ProcessEventOnlMon(PHCompositeNode* topNode)
{ 
  SQEvent*      evt     = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*  hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  SQHitVector*  trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!evt || !hit_vec  || !trig_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  //Determine whether event is FPGA1 
  int is_FPGA1 = (evt->get_trigger(SQEvent::MATRIX1)) ? 1 : 0; 
 
  rs_top_check_p[0] = 0;
  rs_top_check_p[1] = 0;
  rs_bot_check_p[0] = 0;
  rs_bot_check_p[1] = 0; 

  rs_top_check_e[0] = 0;
  rs_top_check_e[1] = 0;
  rs_bot_check_e[0] = 0;
  rs_bot_check_e[1] = 0;

//RF *************************************************************************************** 
  auto vec1 = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, "RF");
  int count = 0;
  for(auto it = vec1->begin(); it != vec1->end(); it++){
    double tdc_time = (*it)->get_tdc_time();
  //  int element = (*it)->get_element_id();       

    //Determining RF buckets for road set timing constraints 
    if(is_FPGA1){
      if(count == 3){
        RF_edge_low[top] = tdc_time;
      }else if(count == 4){
        RF_edge_up[top] = tdc_time;
      }else if(count == 11){
        RF_edge_low[bottom] = tdc_time;
      }else if(count == 12){
        RF_edge_up[bottom] = tdc_time;
      }else{
      }
    }
    count ++;
  }

//ROAD SET Logic  *************************************************************************** 
  vecH1T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[0]);
  vecH2T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[2]);
  vecH3T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[4]);
  vecH4T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[6]);
  
  vecH1B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[1]);
  vecH2B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[3]);
  vecH3B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[5]);
  vecH4B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[7]);

//Checking for FPGA1 road hits on NIM4 TS triggers 
  if(evt->get_trigger(SQEvent::NIM4)){
    for(int j = 0; j < 2; j++){
        if(is_rs_t[j]){
          int z = RoadCheck(vecH1T,vecH2T,vecH3T,vecH4T,rs_top[j],top);
          if(z > 0){
            rs_top_check_e[j] = 1;
          }
        }

        if(is_rs_b[j]){
          int l = RoadCheck(vecH1B,vecH2B,vecH3B,vecH4B,rs_bot[j],bottom);
          if(l > 0){
            rs_bot_check_e[j] = 1;
          }
        }
    }
    
    if(rs_top_check_e[0] || rs_bot_check_e[0] || rs_top_check_e[1] || rs_bot_check_e[1]){
      //Filling histogram for FPGA1 efficiency
      if(is_FPGA1){
        h1_eff_NIM4->Fill(1);
      }else{
        h1_eff_NIM4->Fill(0);
      }       
    }
  }
//Checking for road hits in FPGA1 events   
  if(is_FPGA1){
    for(int j = 0; j < 2; j++){
      if(is_rs_t[j]){
        int y = RoadCheck(vecH1T,vecH2T,vecH3T,vecH4T,rs_top[j],top);
        if(y > 0){
          rs_top_check_p[j] = 1;
        }
      }
      
      if(is_rs_b[j]){
        int x = RoadCheck(vecH1B,vecH2B,vecH3B,vecH4B,rs_bot[j],bottom);
        if(x > 0){
          rs_bot_check_p[j] = 1;
        }
      }
    } 
    
    if(rs_top_check_p[0] || rs_bot_check_p[0] || rs_top_check_p[1] || rs_bot_check_p[1]){
      //Filling histogram for FPGA1 purity    
      h1_purity->Fill(1);
   /* }else if((vecH2T->size() > 0) && (vecH4B->size() > 0)){
      h1_purity->Fill(1);
    }else if((vecH2B->size() > 0) && (vecH4T->size() > 0)){
      h1_purity->Fill(1);*/
    }else{
      h1_purity->Fill(0);
     // debug_print(DEBUG_LVL);

    }
    
  } 

 
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigEP::EndOnlMon(PHCompositeNode* topNode)
{ 
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigEP::FindAllMonHist()
{

  ostringstream oss; 

  oss.str("");
  oss << "h1_purity_" << 0;
  h1_purity = FindMonHist(oss.str().c_str());
  if (! h1_purity) return 1;

  oss.str("");
  oss << "h1_eff_NIM4_" << 0;
  h1_eff_NIM4 = FindMonHist(oss.str().c_str());
  if (! h1_eff_NIM4) return 1;

  return 0;
}

int OnlMonTrigEP::DrawMonitor()
{
  //DRAWING HISTOGRAMS ON .PNG FILES ******************************************


  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->Divide(1,2);
  TVirtualPad* pad00 = pad0->cd(1);
  pad00->SetGrid();
  h1_purity->Draw();
  ostringstream oss;
  
  double pur = h1_purity->GetMean();
  oss << "Purity = " << pur;
  TText* text = new TText();
  text->SetNDC(true);
  text->SetTextAlign(22);
  text->DrawText(0.3, 0.5, oss.str().c_str());
  
  double eff_NIM4 = h1_eff_NIM4->GetMean();
  ostringstream oss0; 
  TVirtualPad* pad01 = pad0->cd(2);
  pad01->SetGrid();
  h1_eff_NIM4->Draw();
  oss0 << "Efficiency = " << eff_NIM4;
  TText* text0 = new TText();
  text0->SetNDC(true);
  text0->SetTextAlign(22);
  text0->DrawText(0.3, 0.5, oss0.str().c_str());
 
  return 0;
}

void OnlMonTrigEP::SetDet()
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

int OnlMonTrigEP::RoadCheck(vector<SQHit*>* H1X, vector<SQHit*>* H2X, vector<SQHit*>* H3X, vector<SQHit*>* H4X,rs_Reader* rs_obj, int top0_or_bot1)
{
  //Returns 1 if a road on one of the road sets was hit 
  int count_rd = 0;

  int H_not_neg[4];
  int hod_hits[4] = {0,0,0,0}; 
  int rd_hits = 1; 
  
  //First loop through road indices
  for(size_t i=0; i<rs_obj->roads.size();i++){
  
    H_not_neg[0] = (rs_obj->roads[i].H1X != -1) ? 1 : 0;  
    H_not_neg[1] = (rs_obj->roads[i].H2X != -1) ? 1 : 0;
    H_not_neg[2] = (rs_obj->roads[i].H3X != -1) ? 1 : 0;   
    H_not_neg[3] = (rs_obj->roads[i].H4X != -1) ? 1 : 0;
    
    //Loop through H1X hits and compare with road index to find matches
    if(H_not_neg[0] && H1X->size() > 0){
      for (auto it = H1X->begin(); it != H1X->end(); it++) {
        if ((*it)->get_level() != 1) continue; //switched m_lvl for 1
        int eleH1X  = (*it)->get_element_id();
        double timeH1X = (*it)->get_tdc_time();
        if((eleH1X == rs_obj->roads[i].H1X) && (timeH1X > RF_edge_low[top0_or_bot1]) && (timeH1X < RF_edge_up[top0_or_bot1])){
          hod_hits[0] = 1;
          break;
        }
      }
   
    }else{
      hod_hits[0]=0;
    }

    //Loop through H2X hits and compare with road index to find matches
    if(H_not_neg[1] && H2X->size() > 0){
      for (auto it = H2X->begin(); it != H2X->end(); it++) {
        if ((*it)->get_level() != 1) continue; //switched m_lvl for 1
        int eleH2X  = (*it)->get_element_id();
        double timeH2X = (*it)->get_tdc_time();
        if((eleH2X == rs_obj->roads[i].H2X) && (timeH2X > RF_edge_low[top0_or_bot1]) && (timeH2X < RF_edge_up[top0_or_bot1])){
          hod_hits[1] = 1;  
          break;
        }
      }
    }else{
      hod_hits[1]=0;
    }
    
    //Loop through H3X hits and compare with road index to find matches   
    if(H_not_neg[2] && H3X->size() > 0){
      for (auto it = H3X->begin(); it != H3X->end(); it++) {
        if ((*it)->get_level() != 1) continue; //switched m_lvl for 1
        int eleH3X  = (*it)->get_element_id();
        double timeH3X = (*it)->get_tdc_time();
        if((eleH3X == rs_obj->roads[i].H3X) && (timeH3X > RF_edge_low[top0_or_bot1]) && (timeH3X < RF_edge_up[top0_or_bot1])){
          hod_hits[2] = 1;  
          break;
        }   
      }
    }else{
      hod_hits[2] = 0;

    }

    //Loop through H4X hits and compare with road index to find matches
    if(H_not_neg[3] && H4X->size() > 0){
      for (auto it = H4X->begin(); it != H4X->end(); it++) {
        if ((*it)->get_level() != 1) continue; //switched m_lvl for 1
        int eleH4X  = (*it)->get_element_id();
        double timeH4X = (*it)->get_tdc_time();
        if((eleH4X == rs_obj->roads[i].H4X) && (timeH4X > RF_edge_low[top0_or_bot1]) && (timeH4X < RF_edge_up[top0_or_bot1])){
          hod_hits[3] = 1;  
          break;
        }   
      }
    }else{
      hod_hits[3] = 0;

    }
   
    //Checks if hodoscope hits constitute a road hit 
    rd_hits = 1;
    for(int j = 0; j < 4; j++){
      if(H_not_neg[j]){// && hod_hits[j] > 0){
        rd_hits *= hod_hits[j];
      }
      hod_hits[j] = 0;
    }

            

    if(rd_hits != 0){   
      count_rd++;     
      //returns 1 if there were any road hits and 0 otherwise
      return 1;
    }
  }
  
  return 0;  
}

void OnlMonTrigEP:: debug_print(int debug_lvl){
  //debug function
  if(debug_lvl == 0){
    cout << endl; 
    cout << "New Event" << endl;
    cout << "H1T: ";
    for (auto it = vecH1T->begin(); it != vecH1T->end(); it++) {
        double ele1 = (*it)->get_element_id();
        cout  << ele1 << ", ";
    }
    cout << endl;

    cout << "H2T: ";
    for (auto it = vecH2T->begin(); it != vecH2T->end(); it++) {
        double ele2 = (*it)->get_element_id();
        cout  << ele2 << ", ";
    }
    cout << endl;

    cout << "H3T: ";
    for (auto it = vecH3T->begin(); it != vecH3T->end(); it++) {
        double ele3 = (*it)->get_element_id();
        cout  << ele3 << ", ";
    }
    cout << endl;

    cout << "H4T: ";
    for (auto it = vecH4T->begin(); it != vecH4T->end(); it++) {
        double ele4 = (*it)->get_element_id();
        cout  << ele4 << ", ";
    }
    cout << endl;
    cout << endl;
 
    cout << "H1B: ";
    for (auto it = vecH1B->begin(); it != vecH1B->end(); it++) {
        double ele1 = (*it)->get_element_id();
        cout  << ele1 << ", ";
    }
    cout << endl;

    cout << "H2B: ";
    for (auto it = vecH2B->begin(); it != vecH2B->end(); it++) {
        double ele2 = (*it)->get_element_id();
        cout  << ele2 << ", ";
    }
    cout << endl;

    cout << "H3B: ";
    for (auto it = vecH3B->begin(); it != vecH3B->end(); it++) {
        double ele3 = (*it)->get_element_id();
        cout  << ele3 << ", ";
    }
    cout << endl;

    cout << "H4B: ";
    for (auto it = vecH4B->begin(); it != vecH4B->end(); it++) {
        double ele4 = (*it)->get_element_id();
        cout  << ele4 << ", ";
    }
    cout << endl;

    
  }


}
