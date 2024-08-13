/// OnlMonTrigV1495.C
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
#include "OnlMonTrigV1495.h"
using namespace std;

//Arguments = name of road set .txt files
OnlMonTrigV1495::OnlMonTrigV1495(const std::string rs_top_0, const std::string rs_top_1, const std::string rs_bot_0, const std::string rs_bot_1, const std::string rs_path)
  : rs_top_0_(rs_top_0)
  , rs_top_1_(rs_top_1)
  , rs_bot_0_(rs_bot_0)
  , rs_bot_1_(rs_bot_1)
  , rs_path_ (rs_path)
  , top   (0) // should be const?
  , bottom(1) // should be const?
{
  NumCanvases(4);
  Name("OnlMonTrigV1495" ); 
  Title("V1495 Trigger Analysis" );

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

int OnlMonTrigV1495::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigV1495::InitRunOnlMon(PHCompositeNode* topNode)
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
      cout << "OnlMonTrigV1495::InitRunOnlMon():  Invalid det_id or n_ele: " 
           << det_id << " " << n_ele << " at name = " << name << "." << endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }

  }
  int rs_hist_range;
  for(int i = 0; i < 2; i++){ 
 
      rs_hist_range = (is_rs_t[i]) ? rs_top[i]->roads.size() : 100;

      oss.str("");
      oss << "h1_rs_top_" << i;
      h1_rs_top[i] = new TH1D(oss.str().c_str(), "",rs_hist_range ,-0.5, rs_hist_range - 0.5);
      oss.str("");
      if(i == 0){
        oss << rs_top_0_ << ";Road Index;Counts";
      }else{
        oss << rs_top_1_ << ";Road Index;Counts";
      }
      h1_rs_top[i]->SetTitle(oss.str().c_str());

      rs_hist_range = (is_rs_b[i]) ? rs_bot[i]->roads.size() : 100;
      oss.str("");
      oss << "h1_rs_bot_" << i;
      h1_rs_bot[i] = new TH1D(oss.str().c_str(), "", rs_hist_range,-0.5, rs_hist_range - 0.5);
      oss.str("");
      if(i == 0){
        oss << rs_bot_0_ << ";Road Index;Counts";
      }else{
        oss << rs_bot_1_ << ";Road Index;Counts";
      }
      h1_rs_bot[i]->SetTitle(oss.str().c_str());

      RegisterHist(h1_rs_top[i]);
      RegisterHist(h1_rs_bot[i]);
  }
  const double DT = 40/9.0; // 4/9 ns per single count of Taiwan TDC
  const int NT    = 200;
  const double T0 = 100.5*DT;
  const double T1 = 300.5*DT; 

  oss.str("");
  oss << "h2_trig_time_" << 1;
  h2_trig_time = new TH2D(oss.str().c_str(), "",NT, T0, T1,10, 0.5, 10.5);
  oss.str("");
  oss << "Trigger Timing After Inh" << ";Trigger;tdcTime;Hit count";
  h2_trig_time->SetTitle(oss.str().c_str());

  h2_trig_time->GetYaxis()->SetBinLabel( 1, "FPGA1");
  h2_trig_time->GetYaxis()->SetBinLabel( 2, "FPGA2");
  h2_trig_time->GetYaxis()->SetBinLabel( 3, "FPGA3");
  h2_trig_time->GetYaxis()->SetBinLabel( 4, "FPGA4");
  h2_trig_time->GetYaxis()->SetBinLabel( 5, "FPGA5");
  h2_trig_time->GetYaxis()->SetBinLabel( 6, "NIM1");
  h2_trig_time->GetYaxis()->SetBinLabel( 7, "NIM2");
  h2_trig_time->GetYaxis()->SetBinLabel( 8, "NIM3");
  h2_trig_time->GetYaxis()->SetBinLabel( 9, "NIM4");
  h2_trig_time->GetYaxis()->SetBinLabel(10, "NIM5");

  const double DT2 = 1.0; // 1 ns per single count of v1495 TDC
  const int NT2 = 300;
  const double T02 = 300.5 * DT2;
  const double T12 = 900.5;

  oss.str("");
  oss << "h2_RF_" << 1;
  h2_RF = new TH2D(oss.str().c_str(), "",NT2, T02, T12,  9, 0.5, 9.5);
  oss.str("");
  oss << "RF TDC" << ";tdcTime;RF Board;Hit count";
  h2_RF->SetTitle(oss.str().c_str());

  oss.str("");
  oss << "h2_fpga_nim_time_af_" << 1;
  h2_fpga_nim_time_af = new TH2D(oss.str().c_str(), "", 100, 1000.5, 1100.5, 100, 1000.5, 1100.5);
  oss.str("");
  oss << "FPGA 1 & NIM 4 After Inh Timing" << ";NIM tdcTime;FPGA tdcTime;Hit count";
  h2_fpga_nim_time_af->SetTitle(oss.str().c_str());

  oss.str("");
  oss << "h1_trig_diff_TS_" << 0;
  h1_trig_diff_TS = new TH1D(oss.str().c_str(), "", NT+1, -0.5, 30.5);
  oss.str("");
  oss << "FPGA1 NIM4 Timing Difference TS constrained" << ";TDC time diff;Hit count";
  h1_trig_diff_TS->SetTitle(oss.str().c_str());

  RegisterHist(h1_trig_diff_TS);
  RegisterHist(h2_trig_time);
  RegisterHist(h2_fpga_nim_time_af);   
  RegisterHist(h2_RF); 
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigV1495::ProcessEventOnlMon(PHCompositeNode* topNode)
{ 

  SQEvent*      evt     = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*  hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  SQHitVector*  trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!evt || !hit_vec  || !trig_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  //Determine whether event is FPGA1 
  int is_FPGA1 = (evt->get_trigger(SQEvent::MATRIX1)) ? 1 : 0; 
 
 
//RF *************************************************************************************** 
  auto vec1 = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, "RF");
  int count = 0;
  for(auto it = vec1->begin(); it != vec1->end(); it++){
    double tdc_time = (*it)->get_tdc_time();
    int element = (*it)->get_element_id();       

    //Determining RF buckets for road set timing constraints 
    if(is_FPGA1){
      h2_RF->Fill(tdc_time,element);
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

//TW TDC ************************************************************************************
  auto vec_FPGA_af = UtilSQHit::FindHitsFast(evt, hit_vec, "AfterInhMatrix");
  for (auto it = vec_FPGA_af->begin(); it != vec_FPGA_af->end(); it++) {
    h2_trig_time->Fill((*it)->get_tdc_time(),(*it)->get_element_id());
  }

  auto vec_NIM_af = UtilSQHit::FindHitsFast(evt, hit_vec, "AfterInhNIM");
  for (auto it = vec_NIM_af->begin(); it != vec_NIM_af->end(); it++) {
    h2_trig_time->Fill((*it)->get_tdc_time(),(*it)->get_element_id()+5); // element +5 so nim index start at 6 in histo
  }

  if(evt->get_trigger(SQEvent::MATRIX1) && evt->get_trigger(SQEvent::NIM4)){

    FPGA_NIM_Time(vec_FPGA_af, vec_NIM_af, 4, 1,h2_fpga_nim_time_af,h1_trig_diff_TS);

  }else{

  }

//ROAD ID Logic  *************************************************************************** 
 if(is_FPGA1){
   vecH1T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[0]);
   vecH2T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[2]);
   vecH3T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[4]);
   vecH4T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[6]);
   
   vecH1B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[1]);
   vecH2B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[3]);
   vecH3B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[5]);
   vecH4B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[7]);
       
   //debug_print(DEBUG_LVL);
//TOP####
   for(int j = 0; j < 2; j++){
      if(is_rs_t[j]){
        RoadHits(vecH1T,vecH2T,vecH3T,vecH4T,rs_top[j],h1_rs_top[j],top);
      }
    }
//BOTTOM####
   for(int j = 0; j < 2; j++){
      if(is_rs_b[j]){
        RoadHits(vecH1B,vecH2B,vecH3B,vecH4B,rs_bot[j],h1_rs_bot[j],bottom);
      }
    } 
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigV1495::EndOnlMon(PHCompositeNode* topNode)
{ 
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigV1495::FindAllMonHist()
{

 // cout << "FIND ALL MON HIST PART" << endl;
  ostringstream oss; 

  oss.str("");
  oss << "h1_trig_diff_TS_" << 0;
  h1_trig_diff_TS = FindMonHist(oss.str().c_str());
  if (! h1_trig_diff_TS) return 1; 

  for(int i = 0; i < 2; i++){
    oss.str("");
    oss << "h1_rs_top_" << i;
    h1_rs_top[i] = FindMonHist(oss.str().c_str());
    if (! h1_rs_top[i]) return 1;

    oss.str("");
    oss << "h1_rs_bot_" << i;
    h1_rs_bot[i] = FindMonHist(oss.str().c_str());
    if (! h1_rs_bot[i]) return 1;

  }

  oss.str("");
  oss << "h2_RF_" << 1;
  h2_RF = (TH2*)FindMonHist(oss.str().c_str());
  if (! h2_RF) return 1; 
 
  oss.str("");
  oss << "h2_trig_time_" << 1;
  h2_trig_time = (TH2*)FindMonHist(oss.str().c_str());
  if (! h2_trig_time) return 1;

  oss.str("");
  oss << "h2_fpga_nim_time_af_" << 1;
  h2_fpga_nim_time_af = (TH2*)FindMonHist(oss.str().c_str());
  if (! h2_fpga_nim_time_af) return 1; 


  return 0;
}

int OnlMonTrigV1495::DrawMonitor()
{
  //DRAWING HISTOGRAMS ON .PNG FILES ******************************************
  UtilHist::AutoSetRangeX(h2_trig_time);
  UtilHist::AutoSetRangeX(h2_fpga_nim_time_af); 
  UtilHist::AutoSetRangeY(h2_fpga_nim_time_af); 
  UtilHist::AutoSetRangeX(h2_RF);

  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->Divide(1,2);
  TVirtualPad* pad00 = pad0->cd(1);
  pad00->SetGrid();
  DrawTH2WithPeakPos(h2_trig_time);
  /*ostringstream oss;
  oss << "pr_" << h2_trig_time->GetName();
  TProfile* pr = h2_trig_time->ProfileX(oss.str().c_str());
  pr->SetLineColor(kBlack);
  pr->Draw("E1same");
*/

  TVirtualPad* pad01 = pad0->cd(2);
  pad01->SetGrid();
  h2_RF->Draw("colz");  

  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->Divide(1,2);
  TVirtualPad* pad10 = pad1->cd(1);
  pad10->SetGrid();
  h1_trig_diff_TS->Draw();

  TVirtualPad* pad11 = pad1->cd(2);
  pad11->SetGrid();
  h2_fpga_nim_time_af->Draw("colz");
  ostringstream oss11;
  oss11 << "pr_" << h2_fpga_nim_time_af->GetName();
  TProfile* pr11 = h2_fpga_nim_time_af->ProfileX(oss11.str().c_str());
  pr11->SetLineColor(kBlack);
  pr11->Draw("E1same");

  OnlMonCanvas* can2 = GetCanvas(2);
  TPad* pad2 = can2->GetMainPad();
  pad2->Divide(1,2);
  TVirtualPad* pad20 = pad2->cd(1);
  pad20->SetGrid();
  h1_rs_top[0]->SetLineColor(kBlue);
  h1_rs_top[0]->Draw();

  TVirtualPad* pad21 = pad2->cd(2);
  pad21->SetGrid();
  h1_rs_bot[0]->SetLineColor(kBlue);
  h1_rs_bot[0]->Draw();

  OnlMonCanvas* can3 = GetCanvas(3);
  TPad* pad3 = can3->GetMainPad();
  pad3->Divide(1,2);
  TVirtualPad* pad30 = pad3->cd(1);
  pad30->SetGrid();
  h1_rs_top[1]->SetLineColor(kBlue);
  h1_rs_top[1]->Draw();

  TVirtualPad* pad31 = pad3->cd(2);
  pad31->SetGrid();
  h1_rs_bot[1]->SetLineColor(kBlue);
  h1_rs_bot[1]->Draw();

  return 0;
}

void OnlMonTrigV1495::SetDet()
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

void OnlMonTrigV1495::RoadHits(vector<SQHit*>* H1X, vector<SQHit*>* H2X, vector<SQHit*>* H3X, vector<SQHit*>* H4X,rs_Reader* rs_obj, TH1* hist_rs, int top0_or_bot1)
{
  //Fills histogram with number of road hits for each road index
  int count_rd = 0;

  int H_not_neg[4];
  int hod_hits[4] = {0,0,0,0}; 
  int rd_hits = 1; 
 
  // First loop through road indices
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

            
    //Fills histogram with road hits
    if(rd_hits != 0){   
      hist_rs->Fill(rs_obj->roads[i].road_id);         
      count_rd++;     
    }
  }
    
}

void OnlMonTrigV1495::FPGA_NIM_Time(vector<SQHit*>* FPGA,vector<SQHit*>* NIM, int NIM_trig_num, int FPGA_trig_num, TH2* h2, TH1* h1){
  //Fill 2D histo with TDC timing for NIM & FPGA trigger
  //Fill 1D histo with TDC timing difference for NIM & FPGA trigger
  for(auto it0 = FPGA->begin(); it0 != FPGA->end(); it0++){ //FPGA
      double ele_FPGA = (*it0)->get_element_id();
      double time_FPGA = (*it0)->get_tdc_time();
      for(auto it1 = NIM->begin(); it1 != NIM->end(); it1++){//NIM
        double ele_NIM = (*it1)->get_element_id();
        double time_NIM = (*it1)->get_tdc_time();
        if(ele_FPGA == FPGA_trig_num && ele_NIM == NIM_trig_num){
          if(h2 != NULL){
            h2->Fill(time_NIM,time_FPGA);
          }
          double time_diff = Abs(time_FPGA,time_NIM);
          h1->Fill(time_diff);
        }
      }
  }

}

void OnlMonTrigV1495::DrawTH2WithPeakPos(TH2* h2, const double cont_min)
{
  h2->Draw("colz");
  int ny = h2->GetNbinsY();
  for (int iy = 1; iy <= ny; iy++) {
    TH1* h1 = h2->ProjectionX("h1_draw_th2", iy, iy);
    ostringstream oss;
    if (h1->GetMaximum() >= cont_min) {
      oss << "Peak @ " << h1->GetXaxis()->GetBinCenter(h1->GetMaximumBin());
    } else {
      oss << "No sizable peak";
    }
    TText* text = new TText();
    text->SetNDC(true);
    text->SetTextAlign(22);
    text->DrawText(0.3, 0.1+(iy-0.5)*0.8/ny, oss.str().c_str());
    // The y-position above assumes that the top & bottom margins are 0.1 each.
  }
}

void OnlMonTrigV1495:: debug_print(int debug_lvl){
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

double OnlMonTrigV1495:: Abs(double var0, double var1){
  //get absolute value of difference between var0 and var1
  if(var0 > var1){ 
    return (var0 - var1);
  }else{
    return (var1 - var0);
  }


}
