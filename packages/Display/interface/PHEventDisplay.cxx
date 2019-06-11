/*!
        \file PHEventDisplay.cxx
        \author Sookhyun Lee
        \brief event display interface,
	       set parameters/switches, call detector modules, control display.
        \version $Revision: 1.2 $
        \date    $Date: 07/26/2016
*/

// STL and BOOST includes
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

// PHOOL and Fun4All includes
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllServer.h>
#include <phool/getClass.h>
#include <phool/phool.h>
#include <phool/PHIODataNode.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHTimer.h>
#include <phool/PHCompositeNode.h>
//#include <TMutNode.h>
#include <phool/PHTimeServer.h>
#include <phgeom/PHGeomUtility.h>
#include <phgeom/PHGeomTGeo.h>
#include <phfield/PHFieldUtility.h>
#include <phfield/PHField.h>


// EVE framework includes
#include "TEveManager.h"
#include "TApplication.h"
#include "TEveBrowser.h"
#include "TEveWindow.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGLAutoRotator.h"
#include "TGLViewer.h"
#include "TEveViewer.h"
#include "TEveScene.h"
#include "TEveProjectionManager.h"
#include "TEveProjectionAxes.h"
#include "TEveGeoNode.h"
#include "TFile.h"


// sPHENIX Event Display
#include <pheve_display/PHEveDisplay.h>

// Trk display
#include <pheve_modules/mTrkEveDisplay.h>

#include <TThread.h>
#include <TStyle.h>

#include "PHEventDisplay.h"

using boost::bind;

void MakeViewerScene(TEveWindowSlot* slot, TEveViewer*& v, TEveScene*& s)
{
  // Create a scene and a viewer in the given slot.
  v = new TEveViewer("Viewer");
  v->SpawnGLViewer((TGedEditor*)gEve->GetEditor());
  slot->ReplaceWindow(v);
  gEve->GetViewers()->AddElement(v);
  s = gEve->SpawnNewScene("Scene");
  v->AddScene(s);
}

TEveElement* MakeProjection (
TEveWindowSlot* slot,
TEveElement* element,
const double start_z,
const double end_z = 200,
const char* name = "Projection"
) {

  TEveViewer* v; TEveScene* s;
  MakeViewerScene(slot, v, s);
  v->SetElementName(Form("Viewer - %s",name));
  s->SetElementName(Form("Scene - %s",name));
  auto mng = new TEveProjectionManager();
  mng->SetProjection(TEveProjection::kPT_RPhi);
  auto axes = new TEveProjectionAxes(mng);
  mng->ImportElements(element);
  s->AddElement(axes);
  s->AddElement(element);

  TGeoNode *node_c = gGeoManager->GetCurrentNode();
  TEveGeoTopNode* tnode_c = new TEveGeoTopNode(gGeoManager, node_c);
  s->AddElement(tnode_c);

  gEve->AddToListTree(axes, kTRUE);
  gEve->AddToListTree(mng, kTRUE);

  auto vv = v->GetGLViewer();

  double mean_z = 0.5*(start_z + end_z);
  double extd_z = end_z - start_z;

  double clip_par[] = {0,0,mean_z,300,300,extd_z};
  vv->GetClipSet()->SetClipType(TGLClip::kClipBox);
  vv->UpdateScene();
  vv->GetClipSet()->SetClipState(TGLClip::kClipBox, clip_par);
  vv->GetClipSet()->GetCurrentClip()->SetMode(TGLClip::kOutside);
  //vv->DoDraw();
  vv->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);

  return 0;
}

PHEventDisplay::PHEventDisplay(int w = 1920,
  int h = 1080,
  bool _use_fieldmap = false,
  bool _use_geofile = false,
  const std::string& _mapname = "sPHEBIX.2d.root",
  const std::string& _geoname = "geo.root") :
  SubsysReco("PHEventDisplay"),
  _pending_update(false),
  _modules(),
  _mutex(PTHREAD_MUTEX_INITIALIZER),
  _update_thread(),
  _PHEveDisplay(new PHEveDisplay(w, h, _use_fieldmap, _use_geofile, _mapname, _geoname, verbosity)),
  _rot(NULL),
  _status_thread(),
  jet_pt_threshold(5.),
  jet_e_scale(30.),
  calo_e_threshold(0.2),
  is_dc_on(true),
  is_hodo_on(true),
  is_prop_on(true),
  is_truth_on(true),
  use_fieldmap(_use_fieldmap),
  use_geofile(_use_geofile),
  width(w),
  height(h),
  mapname(_mapname),
  geoname(_geoname),
  nevent(0),
  verbosity(0)
{

  // Alan's method
  //TEveManager::Create(kTRUE,"V");
  //TEveWindow::SetMainFrameDefWidth(width);
  //TEveWindow::SetMainFrameDefHeight(height);

  // Haiwang's method
  TApplication::NeedGraphicsLibs();
  gApplication->InitializeGraphics();
  TEveUtil::SetupEnvironment();
  TEveUtil::SetupGUI();
  gEve = new TEveManager(width, height, true, "V");

  gEve->GetBrowser()->HideBottomTab();
  TFile::SetCacheFileDir(".");

  gEve->FullRedraw3D(kTRUE);  
  TGLViewer*  v = gEve->GetDefaultGLViewer();
  //v->ColorSet().Background().SetColor(kMagenta+4);
  //v->SetGuideState(TGLUtil::kAxesOrigin, kTRUE, kFALSE, 0);
  v->RefreshPadEditor(v);
  v->DoDraw();

  _PHEveDisplay->set_eve_manager(gEve);

  if (verbosity) std::cout << "PHEventDisplay instantiated."<<std::endl;  
}

PHEventDisplay::~PHEventDisplay()
{
}

int PHEventDisplay::Init(PHCompositeNode *topNode)
{
  SubsysReco::Init(topNode);

  _PHEveDisplay->set_jet_pt_threshold(jet_pt_threshold);
  _PHEveDisplay->set_jet_e_scale(jet_e_scale);
  _PHEveDisplay->set_calo_e_threshold(calo_e_threshold);

  std::cout.precision(5);
  std::cout
  << "dc :" << is_dc_on
  << ", hodo : "<< is_hodo_on
  << ", prop : "<< is_prop_on
  << ", truth : " << is_truth_on
  << ", verbosity : " << verbosity <<std::endl;

  return 0;
}
 
int PHEventDisplay::InitRun(PHCompositeNode *topNode)
{

  if (verbosity)
    std::cout << "PHEventDisplay - initialize run. " << std::endl;

  try {
    _PHEveDisplay->set_verbosity(verbosity);
    if (verbosity)
      std::cout << "PHEventDisplay - add_elements() begins." << std::endl;
    _PHEveDisplay->add_elements(gEve);
    if (verbosity)
      std::cout << "PHEventDisplay - load_geometry() begins. " << std::endl;
    _PHEveDisplay->load_geometry(topNode, gEve);
    if (verbosity)
      std::cout << "PHEventDisplay - config_bfield() begins." << std::endl;
    PHField* field = PHFieldUtility::GetFieldMapNode(nullptr, topNode);
    _PHEveDisplay->config_bfields(field);

    register_module<mTrkEveDisplay>();
    if (verbosity)
      std::cout << "PHEventDisplay - mSvtxEveDisplay module registered." << std::endl;

    if (is_dc_on) {
    }

    std::for_each(_modules.begin(), _modules.end(), bind(&mPHEveModuleBase::init_run, _1, topNode));
    if (verbosity)
      std::cout << "PHEventDisplay - all modules registered. " << std::endl;

  } catch (std::exception &e) {
    std::cout << "Exception caught while initializing sPHENIX event display: " << e.what() << std::endl;
  }

  return 0;
}

int PHEventDisplay::process_event(PHCompositeNode *topNode)
{
  if (verbosity) std::cout<<"PHEventDisplay - setting up to process event." <<std::endl;
  nevent++;

  // Clean up TEveElement list drawn from last events
  if (verbosity) std::cout<<"PHEventDisplay - clear" <<std::endl;
  std::for_each(_modules.begin(),_modules.end(),
    bind(&mPHEveModuleBase::clear,_1));

  // Process this event, reco objects to TEve objects
  if (verbosity) std::cout<<"PHEventDisplay - process" <<std::endl;
  std::for_each(_modules.begin(),
		_modules.end(),
		bind(&mPHEveModuleBase::event,
		_1,
		topNode));

  // Draw this event
  if (verbosity) std::cout<<"PHEventDisplay - draw" <<std::endl;
  update_scene();

  return 0;
}

int PHEventDisplay::End(PHCompositeNode *topNode)
{
  return 0;
}

void PHEventDisplay::update_scene()
{
  if (verbosity) std::cout << "PHEventDisplay - update_scene() nevent = " <<nevent<<std::endl;
  TThread::Lock(); // Keep the autorotator from segfaulting
  draw_default();
  TThread::UnLock();
}

void PHEventDisplay::run_evt_in_thread()
{
  if (verbosity)
    std::cout << "PHEventDisplay - run_evt_in_thread() nevent = " << nevent << std::endl;
  if (!pthread_mutex_trylock(&_mutex)) {
    if (_pending_update) {
      update_scene();
    }
    _pending_update = false;
    _update_thread = boost::make_shared<boost::thread>(bind(&PHEventDisplay::reco_thread, this));
  }
}


void PHEventDisplay::reco_thread()
{
  if (verbosity) std::cout<< "PHEventDisplay - reco_thread() nevent = "<<nevent<<std::endl;
  Fun4AllServer* se = Fun4AllServer::instance();
  se->run(1);
  if (verbosity>1) std::cout <<"reco_thread() run(1) complete."<<std::endl;
  _pending_update = true;
  if (verbosity>1) std::cout<< "reco_thread() pendinding update "<<std::endl;
  pthread_mutex_unlock(&_mutex);
  if (verbosity>1) std::cout <<"reco_thread() mutex unlock()"<<std::endl;

}

void PHEventDisplay::draw_default()
{
  if (verbosity) std::cout<<"PHEventDisplay - draw_default() begins."<<std::endl;

  if(nevent==1) {

    // Extra tabs
    _slot_dc  = TEveWindow::CreateWindowInTab(gEve->GetBrowser()->GetTabRight());
    auto packH = _slot_dc->MakePack();
    packH->SetElementName("2D View");
    packH->SetHorizontal();
    packH->SetShowTitleBar(kFALSE);
    _slot_dc = packH->NewSlot();
    auto pack0  = _slot_dc->MakePack();
    pack0->SetShowTitleBar(kFALSE);
    _slot_dc_00 = pack0->NewSlot();
    _slot_dc_10 = pack0->NewSlot();
    _slot_dc = packH->NewSlot();
    auto pack1  = _slot_dc->MakePack();
    pack1->SetShowTitleBar(kFALSE);
    _slot_dc_01 = pack1->NewSlot();
    _slot_dc_11 = pack1->NewSlot();

    MakeProjection(_slot_dc_00, _PHEveDisplay->get_top_list(), 550,  700,  "ST1");
    MakeProjection(_slot_dc_01, _PHEveDisplay->get_top_list(), 1300, 1450, "ST2");
    MakeProjection(_slot_dc_10, _PHEveDisplay->get_top_list(), 1870, 1970, "ST3");
    MakeProjection(_slot_dc_11, _PHEveDisplay->get_top_list(), 2085, 2450, "ST4");
  }

  if (verbosity>1) std::cout<<"draw_default() FullRedraw3D." <<std::endl;
  gEve->FullRedraw3D(kTRUE);

  if (verbosity>1) std::cout<<"draw_default() TGLViewer." <<std::endl;
  TGLViewer*  v = gEve->GetDefaultGLViewer();
  v->UpdateScene();
  //v->ColorSet().Background().SetColor(kMagenta+4);
  //v->SetGuideState(TGLUtil::kAxesOrigin, kTRUE, kFALSE, 0);
  //v->RefreshPadEditor(v);

  // top view
//  v->CurrentCamera().RotateRad(-3.14/2,0);
//  v->CurrentCamera().Zoom(400, 0, 0);
//  v->CurrentCamera().Truck(3000,0);

  // 3D view
  v->CurrentCamera().RotateRad(-3.14/4., -3.14/4.);
  v->CurrentCamera().Zoom(350, 0, 0);
  v->CurrentCamera().Truck(2000,-1500);
  v->DoDraw();

  //gEve->Redraw3D(kTRUE);
  gEve->DoRedraw3D();
}


void PHEventDisplay::start_rotation()
{
  _rot = gEve->GetDefaultGLViewer()->GetAutoRotator();
  _rot->SetADolly(0.0);
  _rot->SetATheta(0.0);
  _rot->SetWTheta(0.075);
  _rot->SetWPhi(0.1);
  _rot->Start();
}

void
PHEventDisplay::go_fullscreen()
{
  _PHEveDisplay->go_fullscreen(gEve);
}

