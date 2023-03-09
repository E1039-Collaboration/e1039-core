/// OnlMonTrigEP.C
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
#include "OnlMonTrigEP.h"
using namespace std;

OnlMonTrigEP::OnlMonTrigEP(const char* rs_top_0, const char* rs_top_1, const char* rs_bot_0, const char* rs_bot_1)
{
  NumCanvases(2);
  Name("OnlMonTrigEP" ); 
  Title("FPGA1 Purity & Efficiency" );

  top = 0; 
  bottom = 1;

  rs_pur_num = 0.0;
  FPGA1_num = 0.0;
  purity = 0.0;
  
  eff = 0;
  eff_den = 0;
  NIM4_FPGA1_num = 0;

  eff_TW = 0;
  eff_den_TW = 0;
  eff_num_TW = 0; 
 
  rs_top_check_p[0] = 0;
  rs_top_check_p[1] = 0;
  rs_bot_check_p[0] = 0;
  rs_bot_check_p[1] = 0;

  rs_top_check_e[0] = 0;
  rs_top_check_e[1] = 0;
  rs_bot_check_e[0] = 0;
  rs_bot_check_e[1] = 0;

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
  h1_purity = new TH1D(oss.str().c_str(), "", 2, 0.5, 2.5);
  oss.str("");
  oss << "FGPA1 Purity" << ";;Hit count";
  h1_purity->SetTitle(oss.str().c_str());


  h1_purity->GetXaxis()->SetBinLabel( 1, "FPGA1 && rd hit");
  h1_purity->GetXaxis()->SetBinLabel( 2, "FPGA1 && no rd hit");

  RegisterHist(h1_purity);

  oss.str("");
  oss << "h1_eff_" << 0;
  h1_eff = new TH1D(oss.str().c_str(), "", 2, 0.5, 2.5);
  oss.str("");
  oss << "FPGA1 Efficiency (NIM4)" << ";;Hit count";
  h1_eff->SetTitle(oss.str().c_str());
  
  h1_eff->GetXaxis()->SetBinLabel( 1, "NIM4 && rd hit && FPGA1");
  h1_eff->GetXaxis()->SetBinLabel( 2, "NIM4 && rd hit && NO FPGA1");
  
  RegisterHist(h1_eff);

  oss.str("");
  oss << "h1_TW_TDC_" << 0;
  h1_TW_TDC = new TH1D(oss.str().c_str(), "", 2, 0.5, 2.5);
  oss.str("");
  oss << "FPGA1 Efficiency (TW TDC)" << ";;Hit count";
  h1_TW_TDC->SetTitle(oss.str().c_str());

  h1_TW_TDC->GetXaxis()->SetBinLabel( 1, "TW TDC FPGA1 && FPGA1");
  h1_TW_TDC->GetXaxis()->SetBinLabel( 2, "TW TDC FPGA1 && NO FPGA1");
  
  RegisterHist(h1_TW_TDC);
 
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigEP::ProcessEventOnlMon(PHCompositeNode* topNode)
{ 
  SQEvent*      evt     = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*  hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  SQHitVector*  trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!evt || !hit_vec  || !trig_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  //Determine whether event is FPGA1-4 
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
    //int element = (*it)->get_element_id();       
 
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

//ROAD ID Logic  *************************************************************************** 
  auto vecH1T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[0]);
  auto vecH2T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[2]);
  auto vecH3T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[4]);
  auto vecH4T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[6]);
  
  auto vecH1B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[1]);
  auto vecH2B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[3]);
  auto vecH3B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[5]);
  auto vecH4B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[7]);

  if(evt->get_trigger(SQEvent::NIM4)){
   /* cout << endl; 
    cout << "NIM4 Event" << endl;
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
*/

  }

//Any Event type 
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
      eff_den += 1.0; 
      if(is_FPGA1){
        cout << "FPGA1 event" << endl;
        NIM4_FPGA1_num += 1.0;
        eff_den_TW += 1.0;
        eff_num_TW +=1.0;
        h1_eff->Fill(1);
        h1_TW_TDC->Fill(1);
      }else{
        auto vec_FPGA_af = UtilSQHit::FindHitsFast(evt, hit_vec, "AfterInhMatrix");
        for (auto it = vec_FPGA_af->begin(); it != vec_FPGA_af->end(); it++) {
          if((*it)->get_element_id()==1 ){
            cout << "Trigger = "<< (*it)->get_element_id() << endl;
            eff_den_TW += 1.0; 
            h1_TW_TDC->Fill(2);
          }
        } 
        h1_eff->Fill(2);
      }       
    }
  }
//FPGA1 event only  
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
      //cout << "Road Hit" << endl;
      h1_purity->Fill(1);
      rs_pur_num += 1.0;
    }else{
      h1_purity->Fill(2);
    }
    FPGA1_num += 1.0;
    
    } 

 
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigEP::EndOnlMon(PHCompositeNode* topNode)
{ 
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigEP::FindAllMonHist()
{

 // cout << "FIND ALL MON HIST PART" << endl;
  ostringstream oss; 

  oss.str("");
  oss << "h1_purity_" << 0;
  h1_purity = FindMonHist(oss.str().c_str());
  if (! h1_purity) return 1;

  oss.str("");
  oss << "h1_eff_" << 0;
  h1_eff = FindMonHist(oss.str().c_str());
  if (! h1_eff) return 1;

  oss.str("");
  oss << "h1_TW_TDC_" << 0;
  h1_TW_TDC = FindMonHist(oss.str().c_str());
  if (! h1_TW_TDC) return 1;

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
  
  purity = rs_pur_num/FPGA1_num;
  oss << "Purity = " << purity;
  TText* text = new TText();
  text->SetNDC(true);
  text->SetTextAlign(22);
  text->DrawText(0.3, 0.5, oss.str().c_str());
    // The y-position above assumes that the top & bottom margins are 0.1 each.
  
  eff = NIM4_FPGA1_num/eff_den; 
  ostringstream oss0; 
  TVirtualPad* pad01 = pad0->cd(2);
  pad01->SetGrid();
  h1_eff->Draw();
  oss0 << "Efficiency = " << eff;
  TText* text0 = new TText();
  text0->SetNDC(true);
  text0->SetTextAlign(22);
  text0->DrawText(0.3, 0.5, oss0.str().c_str());
 
  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->Divide(1,2);
  TVirtualPad* pad10 = pad1->cd(1);
  pad10->SetGrid();
  h1_TW_TDC->Draw();

  eff_TW = eff_num_TW/eff_den_TW;
  ostringstream oss1;
  oss1 << "Efficiency = " << eff_TW;
  TText* text1 = new TText();
  text1->SetNDC(true);
  text1->SetTextAlign(22);
  text1->DrawText(0.3, 0.5, oss1.str().c_str());

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

  int count_rd = 0;

  int H_not_neg[4];
  int hod_hits[4] = {0,0,0,0}; 
  int rd_hits = 1; 
 
  for(size_t i=0; i<rs_obj->roads.size();i++){
  
    H_not_neg[0] = (rs_obj->roads[i].H1X != -1) ? 1 : 0;  
    H_not_neg[1] = (rs_obj->roads[i].H2X != -1) ? 1 : 0;
    H_not_neg[2] = (rs_obj->roads[i].H3X != -1) ? 1 : 0;   
    H_not_neg[3] = (rs_obj->roads[i].H4X != -1) ? 1 : 0;
    
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
    
    rd_hits = 1;
    for(int j = 0; j < 4; j++){
      if(H_not_neg[j]){// && hod_hits[j] > 0){
        rd_hits *= hod_hits[j];
      }
      hod_hits[j] = 0;
    }

            

    if(rd_hits != 0){   
    //  cout << "Road ID: " << rs_obj->roads[i].road_id << endl;i
      count_rd++;     
      return 1;
    }
  }
  //cout << "Roads hit per event: " << count_rd <<endl;
  return 0;  
}


