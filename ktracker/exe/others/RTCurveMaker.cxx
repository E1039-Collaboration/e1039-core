#include <iomanip>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TClonesArray.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TGraph.h>
#include <TLegend.h>

#include "GlobalConsts.h"
#include "GeomSvc.h"
#include "JobOptsSvc.h"
#include "SRawEvent.h"
#include "FastTracklet.h"
#include "RTCurveFitter.h"

using namespace std;

string DIR_NAME_IN, DIR_NAME_OUT, FN_IN;
int RUN, ITER;

///Retrieve the maximum and minimum of tdc time spectra
double TMIN[24], TMAX[24];
double RMIN[24], RMAX[24];
int NRTPOINT[24];
const int NBINT=30;
const int NBINR=100;

TFile* dataFile;
TTree* dataTree;

//TH2 for RT
TH2* h2_RT[24];
TH2* h2_TR[24];
TH2* h2_TR_ex[24];
TH2* h2_Resi[24];
TH2* h2_td_dd[24]; // x-axis: track distance, y-axis: drift distance
TGraph* gr_t2r_in[24];
TGraph* gr_t2dr  [24];
TSpline3* spl_t2r[24];

GeomSvc *p_geomSvc;

//// Prototypes
void Init();
bool acceptTracklet(Tracklet& tracklet);
void FillRT();
void LoadInputRT();
void ExtractRT();
void DrawBasicPlots();
void DrawFitResults();

////////////////////////////////////////////////////////////////
////
//// Main 
////
int main(int argc, char *argv[])
{
  ostringstream oss;
  ostringstream oss2;
  gErrorIgnoreLevel = 1111;
  gROOT->SetStyle("Plain");

  JobOptsSvc* jobOptsSvc = JobOptsSvc::instance();

  RUN   = atoi(argv[1]);
  ITER  = atoi(argv[2]);
  FN_IN = argv[3];
  Init();
  FillRT();
  DrawBasicPlots();
  ExtractRT();
  DrawFitResults();
  return 0;
}

////////////////////////////////////////////////////////////////
////
//// Functions
////
void Init()
{
  ostringstream oss, oss2;  
  p_geomSvc = GeomSvc::instance();
  p_geomSvc->init();

  oss.str(""); oss << "run/"<< RUN << "/rt_curve_iter" << ITER ; 
  DIR_NAME_OUT = oss.str();
  gSystem->mkdir(DIR_NAME_OUT.c_str(), kTRUE);

  if(ITER > 1){
    oss.str(""); oss << "run/" << RUN << "/rt_curve_iter" << ITER -1; 
    DIR_NAME_IN = oss.str();
  }

  for (int ip = 0; ip < 24; ip++) {
    gr_t2r_in[ip] = new TGraph();
    gr_t2dr  [ip] = new TGraph();
  }

  LoadInputRT();

  //TH2 of R-T
  for (int ip=0; ip < 24; ip++) {
    oss .str("");   oss  << "h2_rt_" << p_geomSvc->getDetectorName(ip+1);
    oss2.str("");   oss2 << "RT curve for "<< p_geomSvc->getDetectorName(ip+1) << " (#" << ip+1 << ");TDC time (ns);R (cm);";
    h2_RT[ip] = new TH2D(oss.str().c_str(), oss2.str().c_str(),  NBINT, TMIN[ip], TMAX[ip], NBINR, RMIN[ip], RMAX[ip]*1.3 );

    oss .str("");   oss  << "h2_resi_" << p_geomSvc->getDetectorName(ip+1);
    oss2.str("");   oss2 << "Residual vs TDC time for "<< p_geomSvc->getDetectorName(ip+1) << ";TDC time (ns);Residual (cm);";
    h2_Resi[ip] = new TH2D(oss.str().c_str(), oss2.str().c_str(),  NBINT, TMIN[ip], TMAX[ip], NBINR, -0.5, 0.5);

    oss .str("");   oss  << "h2_tr_" << p_geomSvc->getDetectorName(ip+1);
    oss2.str("");   oss2 << "TR curve for "<< p_geomSvc->getDetectorName(ip+1) << ";TDC time (ns);R (cm);";
    h2_TR[ip] = new TH2D(oss.str().c_str(), oss2.str().c_str(),  NBINR, RMIN[ip], RMAX[ip]*1.3, NBINT, TMIN[ip], TMAX[ip] );

    oss .str("");   oss  << "h2_tr_ex_" << p_geomSvc->getDetectorName(ip+1);
    oss2.str("");   oss2 << "Expanded TR curve for "<< p_geomSvc->getDetectorName(ip+1) << ";R (cm);Drift time (=TDC-T0) (ns);";
    h2_TR_ex[ip] = new TH2D(oss.str().c_str(), oss2.str().c_str(),  NBINR*2, -RMAX[ip]*1.3, RMAX[ip]*1.3, NBINT, 0.0, TMAX[ip]-TMIN[ip] );

    oss .str("");   oss  << "h2_td_dd_" << p_geomSvc->getDetectorName(ip+1);
    oss2.str("");   oss2 << p_geomSvc->getDetectorName(ip+1) << ";Track distance (cm);Drift distance (cm);Yields";
    h2_td_dd[ip] = new TH2D(oss.str().c_str(), oss2.str().c_str(),  NBINR*2, -RMAX[ip]*1.3, RMAX[ip]*1.3,    NBINR, RMIN[ip], RMAX[ip]*1.3);
  }
}

bool acceptTracklet(Tracklet& tracklet)
{
  if(tracklet.getNHits() <  18) return false;
  //  if(tracklet.getProb() < 0.8) return false;
  if(tracklet.chisq / (tracklet.getNHits()-4) > 2.0) return false;
  //  if(1./tracklet.invP   < 25.) return false;
  return true; 
}

void FillRT()
{
  cout << "Input: " << FN_IN << endl;
  dataFile = new TFile(FN_IN.c_str(), "READ");
  dataTree = (TTree*)dataFile->Get("save");

  TClonesArray* tracklets = new TClonesArray("Tracklet");
  tracklets->Clear();
  dataTree->SetBranchAddress("tracklets", &tracklets);

  int nEvtMax = dataTree->GetEntries();
  cout << "nEvtMax = " << nEvtMax << endl;
  for(int i = 0; i < nEvtMax; i++)
    {
      dataTree->GetEntry(i);

      int nTracklets = tracklets->GetEntries();
      for(int j = 0; j < nTracklets; j++)
	{
	  Tracklet* trk = (Tracklet*)tracklets->At(j);
	  if(!acceptTracklet(*trk)) continue;

	  for(std::list<SignedHit>::iterator iter = trk->hits.begin(); iter != trk->hits.end(); ++iter)
	    {
	      if(iter->hit.index < 0) continue;
//	      if(iter->hit.detectorID == 3 && 65 <= iter->hit.elementID  && iter->hit.elementID <= 80 ) continue;
	      int detectorID = iter->hit.detectorID;
	      double r = iter->hit.driftDistance;
	      double t = iter->hit.tdcTime;
	      double r_exp = trk->getExpPositionW(detectorID) - iter->hit.pos;
              //if (detectorID == 1) cout << " Z " << trk->getExpPositionW(detectorID) << " " << iter->hit.pos << " " << r << " " << t << " " << TMAX[detectorID-1]-t << endl;

	      h2_RT   [detectorID-1]->Fill(t, fabs(r_exp));
	      h2_Resi [detectorID-1]->Fill(t, fabs(r) - fabs(r_exp) );
	      h2_TR   [detectorID-1]->Fill(fabs(r_exp), t);
	      h2_TR_ex[detectorID-1]->Fill(r_exp, TMAX[detectorID-1]-t);
              h2_td_dd[detectorID-1]->Fill(r_exp, r);
	    }
	}       

      tracklets->Clear();
    }
  dataFile->Close();
}

/**
 * Read & set NRTPOINT, TMIN, TMAX, RMIN, RMAX & gr_t2r_in.
 */
void LoadInputRT()
{
  cout << "LoadInputRT()" << endl;
  ostringstream oss;
  if (ITER == 0) oss << "calibration.txt";
  else           oss << "calibration_"<< RUN <<"/calibration_"<< ITER << ".txt";
  string fname = oss.str();
  cout << "  Load the input parameters from '" << fname << "'." << endl; 
  fstream inputRT(fname.c_str(), ios::in);
  if (! inputRT) {
     cerr << "!!ERROR!!  The input file '" << fname << "' does not exist.\n"
          << "           It is necessary to set the initial fit parameters." << endl;
     exit(1);
  }

  char buf[300];
  int iBin, nBin, detectorID;
  double tmin_temp, tmax_temp;
  string det_name;
  while (inputRT.getline(buf, 100)) {
     istringstream detector_info(buf);
     detector_info >> detectorID >> nBin >> tmin_temp >> tmax_temp >> det_name;
     if (detectorID > 24) break;
     
     NRTPOINT[detectorID - 1] = nBin;
     for (int ii = 0; ii < nBin; ii++) {
        double R, T;
        inputRT.getline(buf, 100);
        istringstream cali_line(buf);
        cali_line >> iBin >> T >> R;
        gr_t2r_in[detectorID - 1]->SetPoint(ii, T, R);
        if (ii == 0       ) { TMIN[detectorID - 1] = T;   RMAX[detectorID - 1] = R; }
        if (ii == nBin - 1) { TMAX[detectorID - 1] = T;   RMIN[detectorID - 1] = R; }
     }
  }
  inputRT.close();
}

void ExtractRT()
{
  cout << "ExtractRT()" << endl;
  ostringstream oss;

  oss.str(""); oss << "calibration_" << RUN << "/calibration_"<< ITER + 1 << ".txt";
  ofstream ofs1(oss.str().c_str()); 
  ofs1.setf(ios_base::fixed, ios_base::floatfield);

  oss.str(""); oss << "calibration_" << RUN << "/calibration_res_"<< ITER + 1 << ".txt";
  ofstream ofs2(oss.str().c_str());
  ofs2.setf(ios_base::fixed, ios_base::floatfield);
  
  //save file for rt
  oss.str(""); oss << DIR_NAME_OUT << "/rt_graph.root";
  TFile* saveFile = new TFile( oss.str().c_str(), "RECREATE" );

  AnaFit* ana = new AnaFit();
  for (int ip = 0; ip < 24; ip++) {
     cout << "  Plane " << ip+1 << endl;
     ana->InitInput(h2_RT[ip], RMAX[ip], gr_t2r_in[ip]);
     ana->DoFit();

     spl_t2r[ip] = new TSpline3(*ana->GetResultSpline());
     double t1 = ana->GetResultT1();
     //double t0 = ana->GetResultT0();
     double dr = ana->GetResultDR();
     gr_t2dr[ip]->SetPoint(0, t1, dr);
     ofs2 << setprecision(3) << dr << endl;

     //// Output to calibration file
     ofs1 << ip+1 <<"  "<<  NRTPOINT[ip] <<"  "<< fixed << setprecision(1) << TMIN[ip] <<" "<< TMAX[ip]<<" "<< p_geomSvc->getDetectorName(ip+1) <<endl;
     for (int ipt = 0; ipt < NRTPOINT[ip]; ipt++) {
        double t = TMIN[ip] + 2.5*ipt;
        double r = ana->EvalR(t);
        ofs1 <<"0   "<< setprecision(1) << t <<"  "<< setprecision(4) << r << endl;
     }
  }

  ifstream ifs( "calibration_prop.txt" );
  if(!ifs){
    cerr <<"!!ERROR!!  Cannot open the input file 'calibration_prop.txt'.  Abort." << endl;
    exit(1);
  }    
  string buffer;
  while (getline(ifs, buffer)) ofs1 << buffer << endl;
  ifs .close();
  ofs1.close();
  ofs2.close();

  for(int ip = 0; ip < 24; ip++){
    oss.str(""); oss << "gr_t2r_in_"  << p_geomSvc->getDetectorName(ip+1);
    gr_t2r_in[ip]->SetName(oss.str().c_str());
    gr_t2r_in[ip]->Write();

    oss.str(""); oss << "gr_t2dr_" << p_geomSvc->getDetectorName(ip+1);
    gr_t2dr[ip]->SetName(oss.str().c_str());
    gr_t2dr[ip]->Write();

    oss.str(""); oss << "spl_t2r_" << p_geomSvc->getDetectorName(ip+1);
    spl_t2r[ip]->SetName(oss.str().c_str());
    spl_t2r[ip]->Write();
  }

  saveFile->Write();
  saveFile->Close();
}

void DrawBasicPlots()
{
   cout << "DrawBasicPlots()" << endl;
   ostringstream oss;
   TCanvas* c1 = new TCanvas("c1", "", 800, 600);
   c1->SetGrid();

   for (int ip = 0; ip < 24; ip++) {
      //cout << "  Plane " << ip+1 << endl;

      //oss.str(""); oss << "RT curve for "<< p_geomSvc->getDetectorName(ip+1) << ";TDC time (ns);R (cm);";
      //h2_RT[ip]->SetTitle(oss.str().c_str());
      h2_RT[ip]->Draw("colz");
      oss.str(""); oss << DIR_NAME_OUT << "/rt_" << p_geomSvc->getDetectorName(ip+1) << ".png";
      c1->SaveAs(oss.str().c_str());

      //h2_TR[ip]->Draw("colz");
      //oss.str(""); oss << DIR_NAME_OUT << "/tr_" << p_geomSvc->getDetectorName(ip+1) << ".png";
      //c1->SaveAs(oss.str().c_str());

      h2_TR_ex[ip]->Draw("colz");
      oss.str(""); oss << DIR_NAME_OUT << "/tr_ex_" << p_geomSvc->getDetectorName(ip+1) << ".png";
      c1->SaveAs(oss.str().c_str());

      h2_Resi[ip]->Draw("colz");
      oss.str(""); oss << DIR_NAME_OUT << "/resi_" << p_geomSvc->getDetectorName(ip+1) << ".png";
      c1->SaveAs(oss.str().c_str());

      h2_td_dd[ip]->Draw("colz");
      oss.str(""); oss << DIR_NAME_OUT << "/h2_td_dd_" << p_geomSvc->getDetectorName(ip+1) << ".png";
      c1->SaveAs(oss.str().c_str());

      TH1* h1_td = h2_td_dd[ip]->ProjectionX("h1_td");
      h1_td->Draw();
      oss.str(""); oss << DIR_NAME_OUT << "/h1_td_" << p_geomSvc->getDetectorName(ip+1) << ".png";
      c1->SaveAs(oss.str().c_str());
      delete h1_td;

      TH1* h1_dd = h2_td_dd[ip]->ProjectionY("h1_dd");
      h1_dd->Draw();
      oss.str(""); oss << DIR_NAME_OUT << "/h1_dd_" << p_geomSvc->getDetectorName(ip+1) << ".png";
      c1->SaveAs(oss.str().c_str());
      delete h1_dd;
    }
   delete c1;
}

void DrawFitResults()
{
   cout << "DrawFitResults()" << endl;
   ostringstream oss;
   TCanvas* c1 = new TCanvas("c1", "", 800, 600);
   c1->SetGrid();
   // c1->SetMargin(0.08, 0.02, 0.08, 0.02); // l, r, b, t
   gStyle->SetOptStat(0);
//       gStyle->SetOptFit(kTRUE);
   TLegend* leg = new TLegend(0.7, 0.7, 0.9, 0.9);
   leg->SetTextFont(22);
   leg->SetBorderSize(1);
   leg->SetFillColor(0);

   for(int ip = 0; ip < 24; ip++) {
      //cout << "  Plane " << ip+1 << endl;
      leg->Clear();

      h2_RT[ip]->Draw("colz");
      
      spl_t2r[ip]->SetMarkerStyle(10);
      spl_t2r[ip]->SetLineWidth(2);
      spl_t2r[ip]->Draw("sameCP"); // LPE1

      gr_t2dr[ip]->SetMarkerColor(kGreen);
      gr_t2dr[ip]->SetMarkerStyle(10);
      gr_t2dr[ip]->SetLineWidth(2);
      gr_t2dr[ip]->Draw("sameP");

      gr_t2r_in[ip]->SetLineStyle(2);
      gr_t2r_in[ip]->SetLineWidth(2);
      gr_t2r_in[ip]->Draw("sameC"); // LP

      leg->AddEntry(spl_t2r  [ip], "Output", "l");
      leg->AddEntry(gr_t2r_in[ip], "Input" , "l");
      leg->Draw();
    
      oss.str(""); oss << DIR_NAME_OUT << "/rt_result_" << p_geomSvc->getDetectorName(ip+1) << ".png";
      c1->SaveAs(oss.str().c_str());

    }
   delete leg;
   delete c1;

   const char* NAME_GROUP[4] = {"D1", "D2", "D3p", "D3m"};
   const double   RES_MAX[4] = {0.1, 0.1, 0.1, 0.1};
   for (int ig = 0; ig < 4; ig++) {
      TCanvas* c1 = new TCanvas("c1", "", 800, 600);
      TLegend* leg = new TLegend(0.75, 0.75, 0.99, 0.99);
      leg->SetTextFont(22);
      leg->SetBorderSize(1);
      leg->SetFillColor(0);
      TH1* fr = c1->DrawFrame(TMIN[ig*6], 0, TMAX[ig*6], RES_MAX[ig], "Resolution vs TDC ;TDC time (ns);Resolution (cm)");
      
      for (int ip = 0; ip < 6; ip++) {
         gr_t2dr[ ig*6 +ip ]->SetLineColor  (ip+1);
         gr_t2dr[ ig*6 +ip ]->SetMarkerColor(ip+1);
         gr_t2dr[ ig*6 +ip ]->Draw("sameCP"); // LPE1

         oss.str(""); oss << p_geomSvc->getDetectorName( ig*6 + ip + 1);
         leg->AddEntry(gr_t2dr[ ig*6 + ip ], oss.str().c_str(), "l");
      }
      
      c1->SetGrid();
      leg->Draw();
      oss.str(""); oss << DIR_NAME_OUT << "/res_" << NAME_GROUP[ig] <<".png";
      c1->SaveAs(oss.str().c_str());
      
      delete c1;
      delete leg;
   }
}
