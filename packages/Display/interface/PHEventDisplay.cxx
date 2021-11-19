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

#include <interface_main/SQEvent.h>


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
#include "TGLCameraOverlay.h"
#include "TGLAnnotation.h"
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
  v->SetElementName(TString::Format("Viewer - %s",name));
  s->SetElementName(TString::Format("Scene - %s",name));

  s->AddElement(element);

  TGeoNode *node_c = gGeoManager->GetCurrentNode();
  TEveGeoTopNode* tnode_c = new TEveGeoTopNode(gGeoManager, node_c);
  s->AddElement(tnode_c);

  auto vv = v->GetGLViewer();

  double mean_z = 0.5*(start_z + end_z);
  double extd_z = end_z - start_z;

  double clip_par[] = {0,0,mean_z,300,300,extd_z};
  vv->GetClipSet()->SetClipType(TGLClip::kClipBox);
  vv->UpdateScene();
  vv->GetClipSet()->SetClipState(TGLClip::kClipBox, clip_par);
  vv->GetClipSet()->GetCurrentClip()->SetMode(TGLClip::kOutside);
  vv->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);

  TGLCameraOverlay* co = vv->GetCameraOverlay();
  co->SetShowOrthographic(kTRUE);
  co->SetOrthographicMode(TGLCameraOverlay::kGridFront);

  return 0;
}


TEveElement* DrawHodo (
TEveWindowSlot* slot,
TEveElement* element,
const double zoom = 0.1,
const double hrot = 0,
const double vrot = 0,
const char* name = "Hodo"
) {

  TEveViewer* v; TEveScene* s;
  MakeViewerScene(slot, v, s);
  v->SetElementName(TString::Format("Viewer - %s",name));
  s->SetElementName(TString::Format("Scene - %s",name));

  s->AddElement(element);

  TGeoNode *node_c = gGeoManager->GetCurrentNode();
  TEveGeoTopNode* tnode_c = new TEveGeoTopNode(gGeoManager, node_c);
  s->AddElement(tnode_c);

  auto vv = v->GetGLViewer();

  TGLViewer::ECameraType cam_type = TGLViewer::kCameraOrthoZOX;
  double cent[3] = {0, 0, 1000};
  vv->SetOrthoCamera(cam_type, zoom, 0, cent, hrot, vrot);
  vv->SetCurrentCamera(cam_type);
  TGLCameraOverlay* co = vv->GetCameraOverlay();
  co->SetShowOrthographic(kTRUE);
  co->SetOrthographicMode(TGLCameraOverlay::kGridFront);

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

    if (verbosity)
      std::cout << "_modules.size()" << _modules.size() << std::endl;

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
  update_scene(topNode);

  return 0;
}

int PHEventDisplay::End(PHCompositeNode *topNode)
{
  return 0;
}

void PHEventDisplay::set_view_top()
{
  TGLViewer* v = gEve->GetDefaultGLViewer();
  v->ResetCurrentCamera();
  v->CurrentCamera().RotateRad(-3.14/2.0, 0);
  v->CurrentCamera().Zoom(200, 0, 0); // (400, 0, 0);
  v->CurrentCamera().Truck(500, 0); // (2800,0);
  v->DoDraw();
}

void PHEventDisplay::set_view_side()
{
  TGLViewer* v = gEve->GetDefaultGLViewer();
  v->ResetCurrentCamera();
  v->CurrentCamera().Zoom(200, 0, 0); // (400, 0, 0);
  v->CurrentCamera().Truck(500, 0); // (2800,0);
  v->DoDraw();
}

void PHEventDisplay::set_view_3d()
{
  TGLViewer* v = gEve->GetDefaultGLViewer();
  v->ResetCurrentCamera();
  v->CurrentCamera().RotateRad(-3.14/4., -3.14/4.);
  v->CurrentCamera().Zoom(180, 0, 0); // (350, 0, 0);
  v->CurrentCamera().Truck(1000, -500); // (2000,-1500);
  v->DoDraw();
}


void PHEventDisplay::update_scene(PHCompositeNode *topNode)
{
  if (verbosity) std::cout << "PHEventDisplay - update_scene() nevent = " <<nevent<<std::endl;
  TThread::Lock(); // Keep the autorotator from segfaulting
  draw_default(topNode);
  TThread::UnLock();
}

//void PHEventDisplay::run_evt_in_thread()
//{
//  if (verbosity)
//    std::cout << "PHEventDisplay - run_evt_in_thread() nevent = " << nevent << std::endl;
//  if (!pthread_mutex_trylock(&_mutex)) {
//    if (_pending_update) {
//      update_scene();
//    }
//    _pending_update = false;
//    _update_thread = boost::make_shared<boost::thread>(bind(&PHEventDisplay::reco_thread, this));
//  }
//}


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

void PHEventDisplay::draw_default(PHCompositeNode *topNode)
{
  if (verbosity) std::cout<<"PHEventDisplay - draw_default() begins."<<std::endl;

  if(nevent==1) {

    // Extra tabs
    _slot_dc  = TEveWindow::CreateWindowInTab(gEve->GetBrowser()->GetTabRight());
    auto pack_dc = _slot_dc->MakePack();
    pack_dc->SetElementName("Station View");
    pack_dc->SetHorizontal();
    pack_dc->SetShowTitleBar(kFALSE);
    _slot_dc = pack_dc->NewSlot();
    auto pack0  = _slot_dc->MakePack();
    pack0->SetShowTitleBar(kFALSE);
    _slot_dc_00 = pack0->NewSlot();
    _slot_dc_10 = pack0->NewSlot();
    _slot_dc = pack_dc->NewSlot();
    auto pack1  = _slot_dc->MakePack();
    pack1->SetShowTitleBar(kFALSE);
    _slot_dc_01 = pack1->NewSlot();
    _slot_dc_11 = pack1->NewSlot();

    // 1970 | 2085 separate by absorber
    // 2185 logic separation
    MakeProjection(_slot_dc_00, _PHEveDisplay->get_top_list(), 550,  700,  "ST1");
    MakeProjection(_slot_dc_01, _PHEveDisplay->get_top_list(), 1300, 1450, "ST2");
    MakeProjection(_slot_dc_10, _PHEveDisplay->get_top_list(), 1870, 2185, "ST3");
    MakeProjection(_slot_dc_11, _PHEveDisplay->get_top_list(), 2185, 2450, "ST4");

    _slot_hodo  = TEveWindow::CreateWindowInTab(gEve->GetBrowser()->GetTabRight());
    auto pack_hodo = _slot_hodo->MakePack();
    pack_hodo->SetElementName("Hodo View");
    pack_hodo->SetVertical();
    pack_hodo->SetShowTitleBar(kFALSE);
    _slot_hodo = pack_hodo->NewSlot();
    auto pack_hodo_0  = _slot_hodo->MakePack();
    pack_hodo_0->SetShowTitleBar(kFALSE);
    _slot_hodo_xz = pack_hodo_0->NewSlot();
    _slot_hodo = pack_hodo->NewSlot();
    auto pack_hodo_1  = _slot_hodo->MakePack();
    pack_hodo_1->SetShowTitleBar(kFALSE);
    _slot_hodo_yz = pack_hodo_1->NewSlot();

    DrawHodo(_slot_hodo_xz, _PHEveDisplay->get_hodo_list(), 0.25,        0.05, 3.14, "XZ");
    DrawHodo(_slot_hodo_yz, _PHEveDisplay->get_hodo_list(), 0.25, 3.14/2-0.05, 3.14, "YZ");
  }

  if (verbosity>1) std::cout<<"draw_default() FullRedraw3D." <<std::endl;
  gEve->FullRedraw3D(); // (kTRUE);

  if (verbosity>1) std::cout<<"draw_default() TGLViewer." <<std::endl;
  TGLViewer*  v = gEve->GetDefaultGLViewer();
  //v->UpdateScene();

  //v->ColorSet().Background().SetColor(kMagenta+4);
  //v->SetGuideState(TGLUtil::kAxesOrigin, kTRUE, kFALSE, 0);
  //v->RefreshPadEditor(v);

  if(nevent==1) {
    v->UpdateScene();
    set_view_3d();
  }
  //v->DoDraw();

  // Annotation

  SQEvent* _sqevent = findNode::getClass<SQEvent>(topNode,"SQEvent");
  if (_sqevent) {
    if(verbosity) std::cout<<"PHEventDisplay - SQEvent nodes found."<<std::endl;
    v->DeleteOverlayAnnotations();
    TGLAnnotation* ann = new TGLAnnotation(v, TString::Format("Run: %d, Spill: %d, Event: %d", _sqevent->get_run_id(), _sqevent->get_spill_id(), _sqevent->get_event_id()), 0.55, 0.95);
    ann->SetTextSize(0.04);
    ann = new TGLAnnotation(v,
        TString::Format("NIM: {%d, %d, %d, %d, %d}  MATRIX: {%d, %d, %d, %d, %d}",
            _sqevent->get_trigger(SQEvent::NIM1),
            _sqevent->get_trigger(SQEvent::NIM2),
            _sqevent->get_trigger(SQEvent::NIM3),
            _sqevent->get_trigger(SQEvent::NIM4),
            _sqevent->get_trigger(SQEvent::NIM5),
            _sqevent->get_trigger(SQEvent::MATRIX1),
            _sqevent->get_trigger(SQEvent::MATRIX2),
            _sqevent->get_trigger(SQEvent::MATRIX3),
            _sqevent->get_trigger(SQEvent::MATRIX4),
            _sqevent->get_trigger(SQEvent::MATRIX5)
            ), 0.55, 0.90);
    ann->SetTextSize(0.04);
  }

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

