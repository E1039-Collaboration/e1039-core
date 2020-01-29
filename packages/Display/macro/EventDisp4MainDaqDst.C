#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include "G4_SensitiveDetectors.C"
#include "G4_Target.C"
R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libdecoder_maindaq)
R__LOAD_LIBRARY(libg4testbench)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libevt_filter)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libonlmonserver)

R__LOAD_LIBRARY(libpheve_display)
R__LOAD_LIBRARY(libpheve_modules)
R__LOAD_LIBRARY(libpheve_interface)
#endif

int run;
int n_evt;
int i_evt;

EvtFilter* evt_filter;
TGLViewer*  glv;
TGNumberEntry *ne_evt_id;
TGNumberEntry *ne_trig;

TGLabel* lbl_run;
TGLabel* lbl_n_evt;

string GetDstPath(const int run)
{
  ostringstream oss;
  oss << setfill('0') << "/data2/e1039/onlmon/evt_disp/run_" << setw(6) << run << "_evt_disp.root";
  return oss.str();
}

class EvNavHandler
{
  public:
    int GetNumEvents(const int run)
    {
      int ret = -1;
      TFile* file = new TFile(GetDstPath(run).c_str());
      if (file->IsOpen()) {
        TTree* tree = (TTree*)file->Get("T");
        if (tree) ret = tree->GetEntries();
      }
      delete file;
      return ret;
    }

    void UpdateLabels()
    {
      ostringstream oss;
      oss << "Run " << run;
      lbl_run->SetText(oss.str().c_str());
      oss.str("");
      oss << "Event " << i_evt << " / " << n_evt;
      lbl_n_evt->SetText(oss.str().c_str());
    }
  
    int OpenRun(const int run)
    {
      n_evt = GetNumEvents(run);
      i_evt = 0;
      UpdateLabels();
      Fun4AllInputManager *in = Fun4AllServer::instance()->getInputManager("DSTIN");
      return in->fileopen(GetDstPath(run));
    }

    void NextEvent()
    {
      if (i_evt >= n_evt) {
        cout << "NextEvent(): No next event." << endl;
        return;
      }
      Fun4AllServer* se = Fun4AllServer::instance();
      i_evt++;
      cout << "Next: " << i_evt << " / " << n_evt << endl;
      UpdateLabels();
      se->run(1, true);
    }
    void PrevEvent()
    {
      if (i_evt < 2) {
        cout << "PrevEvent(): No previous events." << endl;
        return;
      }
      int i_new = i_evt - 1;
      cout << "Prev: " << i_new << " / " << n_evt << endl;
      if (OpenRun(run) != 0) {
        cout << "PrevEvent(): Cannot reopen DST." << endl;
        return;
      }
      i_evt = i_new;
      UpdateLabels();
      Fun4AllServer* se = Fun4AllServer::instance();
      if (i_evt > 1) se->skip(i_evt - 1); // First move to the previous-to-previous event.
      se->run(1, true); // Then read the previous event.
    }
    void ReqEvtID() {
      printf("ReqEvtID: %ld\n",ne_evt_id->GetNumberEntry()->GetIntNumber());
      evt_filter->set_event_id_req((int)(ne_evt_id->GetNumberEntry()->GetIntNumber()));
    }
    void ReqTrig() {
      printf("ReqTrig: %ld\n",ne_trig->GetNumberEntry()->GetIntNumber());
      evt_filter->set_trigger_req((int)(ne_trig->GetNumberEntry()->GetIntNumber()));
    }
    void TopView()
    {
      printf("Top View\n");
      glv->ResetCurrentCamera();
      glv->CurrentCamera().RotateRad(-3.14/2.0, 0);
      glv->CurrentCamera().Zoom(200, 0, 0); // (400, 0, 0);
      glv->CurrentCamera().Truck(500, 0); // (2800,0);
      glv->DoDraw();
    }
    void SideView()
    {
      printf("Side View\n");
      glv->ResetCurrentCamera();
      glv->CurrentCamera().Zoom(200, 0, 0); // (400, 0, 0);
      glv->CurrentCamera().Truck(500, 0); // (2800,0);
      glv->DoDraw();
    }
    void View3D()
    {
      printf("3D View\n");
      glv->ResetCurrentCamera();
      glv->CurrentCamera().RotateRad(-3.14/4., -3.14/4.);
      glv->CurrentCamera().Zoom(180, 0, 0); // (350, 0, 0);
      glv->CurrentCamera().Truck(1000, -500); // (2000,-1500);
      glv->DoDraw();
    }

};

EvNavHandler* handler;

bool FindNewRuns(vector<int>& list_run)
{
  int nn = list_run.size();
  int run = nn > 0  ? list_run[nn-1] : 0;
  while (++run < 4000) {
    string fname = GetDstPath(run);
    if (! gSystem->AccessPathName(fname.c_str())) { // if exists
      list_run.push_back(run);
    }
  }
  return list_run.size() > nn;
}

void make_fun4all()
{
  Fun4AllServer* se = Fun4AllServer::instance();
  se->Verbosity(0);

  JobOptsSvc *jobopt_svc = JobOptsSvc::instance();
  jobopt_svc->init("run6_data.opts");

  // Fun4All G4 module
  PHG4Reco *g4Reco = new PHG4Reco();
  se->registerSubsystem(g4Reco);
  g4Reco->SetWorldSizeX(1000);
  g4Reco->SetWorldSizeY(1000);
  g4Reco->SetWorldSizeZ(5000);
  g4Reco->SetWorldShape("G4BOX");
  g4Reco->SetWorldMaterial("G4_AIR"); //G4_Galactic, G4_AIR
  g4Reco->SetPhysicsList("FTFP_BERT");

  // insensitive elements of the spectrometer
  PHG4E1039InsensSubsystem* insens = new PHG4E1039InsensSubsystem("Insens");
  g4Reco->registerSubsystem(insens);

  SetupTarget(g4Reco);

  SetupSensitiveDetectors(g4Reco);

  /// (width, height, use_fieldmap, use_geofile, field-map name, geo-file name)
  PHEventDisplay* event_display = new PHEventDisplay(1920, 1080, false, false, "", "geom.root");
  event_display->set_verbosity(3);
  se->registerSubsystem(event_display);

  PHG4Reco *g4 = (PHG4Reco *) se->getSubsysReco("PHG4RECO");
  PHEventDisplay *eve = (PHEventDisplay *) se->getSubsysReco("PHEventDisplay");
  evt_filter = (EvtFilter*) se->getSubsysReco("EvtFilter");

  g4->InitRun(se->topNode());
  eve->InitRun(se->topNode());

  Fun4AllInputManager *in = new Fun4AllDstInputManager("DSTIN");
  se->registerInputManager(in);
}

void make_gui()
{
  // Create minimal GUI for event navigation.

  handler = new EvNavHandler;

  TEveBrowser* browser = gEve->GetBrowser();
  browser->StartEmbedding(TRootBrowser::kLeft);

  TGMainFrame* frmMain = new TGMainFrame(gClient->GetRoot(), 1000, 600);
  frmMain->SetWindowName("Event Display");
  frmMain->SetCleanup(kDeepCleanup);

  TGVerticalFrame* frmVert = new TGVerticalFrame(frmMain);
  {
    lbl_run = new TGLabel(frmVert, "Run ??????");
    frmVert->AddFrame(lbl_run, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX | kLHintsLeft, 2,2,2,2));

    lbl_n_evt = new TGLabel(frmVert, "N of events = ??");
    frmVert->AddFrame(lbl_n_evt, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX | kLHintsLeft, 2,2,2,2));

    TGTextButton* b = 0;
    TGHorizontalFrame* frm1 = 0;

    TGLabel* lab = 0;

    frm1 = new TGHorizontalFrame(frmVert);
    lab = new TGLabel(frm1, "Event ID");
    frm1->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 3, 4));
    ne_evt_id = new TGNumberEntry(frm1, -1, 9, 999, TGNumberFormat::kNESInteger,
        TGNumberFormat::kNEAAnyNumber,
        TGNumberFormat::kNELLimitMinMax,
        -999999, 999999);
    ne_evt_id->Connect("ValueSet(Long_t)", "EvNavHandler", handler, "ReqEvtID()");
    frm1->AddFrame(ne_evt_id, new TGLayoutHints(kLHintsCenterY | kLHintsRight, 5, 5, 5, 5));
    frmVert->AddFrame(frm1);


    frm1 = new TGHorizontalFrame(frmVert);
    lab = new TGLabel(frm1, "Trigger");
    frm1->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 3, 4));
    ne_trig = new TGNumberEntry(frm1, -1, 9, 999, TGNumberFormat::kNESInteger,
        TGNumberFormat::kNEAAnyNumber,
        TGNumberFormat::kNELLimitMinMax,
        -999, 999);
    ne_trig->Connect("ValueSet(Long_t)", "EvNavHandler", handler, "ReqTrig()");
    frm1->AddFrame(ne_trig, new TGLayoutHints(kLHintsCenterY | kLHintsRight, 5, 5, 5, 5));
    frmVert->AddFrame(frm1);


    b = new TGTextButton(frmVert, "Next Event");
    frmVert->AddFrame(b, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    b->Connect("Clicked()", "EvNavHandler", handler, "NextEvent()");

    b = new TGTextButton(frmVert, "Previous Event");
    frmVert->AddFrame(b, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    b->Connect("Clicked()", "EvNavHandler", handler, "PrevEvent()");

    b = new TGTextButton(frmVert, "Top View");
    frmVert->AddFrame(b, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    b->Connect("Clicked()", "EvNavHandler", handler, "TopView()");

    b = new TGTextButton(frmVert, "Side View");
    frmVert->AddFrame(b, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    b->Connect("Clicked()", "EvNavHandler", handler, "SideView()");

    b = new TGTextButton(frmVert, "3D View");
    frmVert->AddFrame(b, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    b->Connect("Clicked()", "EvNavHandler", handler, "View3D()");

  }
  frmMain->AddFrame(frmVert);

  frmMain->MapSubwindows();
  frmMain->Resize();
  frmMain->MapWindow();

  browser->StopEmbedding();
  browser->SetTabTitle("Event Control", 0);

  glv = gEve->GetDefaultGLViewer();
}


void EventDisp4MainDaqDst()
{
  vector<int> list_run;
  if (! FindNewRuns(list_run)) {
    cout << "Found no run.  Do nothing." << endl;
    exit(1);
  }
  run = list_run.back();
  cout << "Run = " << run << endl;
  run = 1543; // for test

  make_fun4all();

  make_gui();

  handler->OpenRun(run);
  handler->NextEvent();
}
