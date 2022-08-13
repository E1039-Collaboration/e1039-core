R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libktracker)

// St. ID: 1=D0, 2=D1, 3=D2, 4=D3p, 5=D3m, 6=D23, 7=Global
int AnaData(const int run_id, TString infile, TString outfile)
{
  TFile* f_out = new TFile(outfile, "RECREATE");
  TH1* h1_st_id = new TH1D("h1_st_id", ";Station ID;N of tracklets", 7, -0.5, 6.5);
  TH2* h2_ntrk  = new TH2D("h2_ntrk" , ";N of tracklets/event;Station ID",  100, -0.5, 199.5,  7, 0.5, 7.5);
  TH2* h2_nhit  = new TH2D("h2_nhit" , ";N of hits/tracklet;Station ID",  20, 0.5, 20.5,  7, 0.5, 7.5);
  TH2* h2_rchi2 = new TH2D("h2_chi2" , ";Tracklet #chi^{2}/NDF;Station ID" , 100, 0, 10,  7, 0.5, 7.5);

  TFile* f_in = new TFile(infile);
  TTree* t_in = (TTree*)f_in->Get("save");
  if (! t_in) return 1;

  //SRawEvent* raw = 0;
  SRecEvent* rec = 0;
  TClonesArray* arr_trk = new TClonesArray("Tracklet",1000);

  t_in->SetBranchAddress("recEvent"       , &rec);
  t_in->SetBranchAddress("outputTracklets", &arr_trk);

  int n_evt = t_in->GetEntries();
  for (int i_evt = 0; i_evt < n_evt; i_evt++) {
    t_in->GetEntry(i_evt);
    int n_trk = arr_trk->GetEntries();

    map<int, int> list_n_trk; // <station ID, N of tracks>
    for (int i_trk = 0; i_trk < n_trk; i_trk++) {
      Tracklet* trk = (Tracklet*)arr_trk->At(i_trk);
      int st_id = trk->stationID;
      int n_hit = trk->getNHits();
      int ndf = n_hit - 4; // Correct only when KMag is off
      double rchi2 = trk->chisq / ndf;

      list_n_trk[st_id]++;

      h1_st_id->Fill(st_id);
      h2_nhit ->Fill(n_hit, st_id);
      h2_rchi2->Fill(rchi2, st_id);
    }

    for (int st_id = 1; st_id <= 7; st_id++) {
      h2_ntrk->Fill(list_n_trk[st_id], st_id);
    }
  }

  f_out->Write();
  f_out->Close();

  return 0;
}
