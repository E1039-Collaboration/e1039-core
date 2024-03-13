/*
        \file PHEveDisplay.cxx
        \author Sookhyun Lee
        \brief main display module, 
	       load geometry, configure b-field, add elements.
        \version $Revision: 1.2 $
        \date    $Date: 07/26/2016
*/

// STL and BOOST includes
#include <iostream>
#include <string>
#include <stdexcept>
//#include <boost/shared_ptr.hpp>
#include <fun4all/Fun4AllServer.h>
#include <phool/PHCompositeNode.h>
#include <phool/phool.h>
#include <phgeom/PHGeomUtility.h>

// EVE class includes
#include "TEveManager.h"
#include "TEveVSDStructs.h"
#include "TEveGeoNode.h"
#include "TEveVector.h"
#include "TEveTrack.h"
#include "TEveTrackPropagator.h"
#include "TEvePointSet.h"
#include "TEveWindowManager.h"
#include "TEveWindow.h"
#include "TEveViewer.h"
#include "TEveBrowser.h"

#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TGeoMedium.h"

#include "TGLViewer.h"
#include "TGPack.h"

#include <TVirtualX.h>
#include "TSystem.h"
#include "TStyle.h"
#include "TFile.h"
#include "TMath.h"

#include <phfield/PHField.h>
#include "PHEveDisplay.h"

using namespace CLHEP;

PHEveDisplay::PHEveDisplay(int w,
			   int h,
			   bool use_fieldmap,
			   bool use_geofile,
			   const std::string& mapname,
			   const std::string& filename,
			   int verb) :
  _top_list(NULL),
  _dc_list(NULL),
  _hodo_list(NULL),
  _prop_list(NULL),
  _dp_list(NULL),
  _true_list(NULL),
  cnt_prop(NULL),
  mapped_field(NULL),
  _width(w),
  _height(h),
  _use_fieldmap(use_fieldmap),
  _use_geofile(use_geofile),
  _jet_pt_threshold(5.0),
  _calo_e_threshold(0.2),
  map_filename(mapname),
  geo_filename(filename),
  verbosity(verb)
{
  if (verbosity) std::cout << "PHEveDisplay initialized. " << std::endl;

  if (verbosity>1){
  Fun4AllServer* se = Fun4AllServer::instance();
  se->Print("NODETREE");}

}  

PHEveDisplay::~PHEveDisplay()
{
  try {
    TEveManager::Terminate();
  } catch (std::exception &e) {
    std::cout << "Exception caught during deconstruction: " << e.what() << std::endl;
  }
}

void 
PHEveDisplay::load_geometry(PHCompositeNode *topNode, TEveManager* geve)
{
  TFile* geom = new TFile();
  if (_use_geofile) {
    geom = TFile::Open(geo_filename.c_str());
    if (!geom)
      throw std::runtime_error("Could not open sphenix_geo.root geometry file, aborting.");
    geve->GetGeometry(geo_filename.c_str());
    //gGeoManager->DefaultColors();
  } else {
    if(verbosity) {
      std::cout << "PHEveDisplay::load_geometry:" << " Using PHGeomUtility for Geometry" << std::endl;
    }
    PHGeomUtility::GetTGeoManager(topNode);
    assert(gGeoManager);
  }

  gStyle->SetPalette(1);
  TGeoVolume* top = gGeoManager->GetTopVolume();
  const int nd = top->GetNdaughters();
  std::cout << "nd= " << nd << std::endl;
  TGeoNode* node[nd];

  for (int i = 0; i < nd; i++) {
    node[i] = top->GetNode(i);
    std::cout << "Node " << i << " : " << node[i]->GetName() << std::endl;

    node[i]->GetVolume()->SetTransparency(95); // 0: opaque, 100: transparent

    std::string name(node[i]->GetName());
    if (name.find("fmag") < 5 || name.find("kmag") < 5) { // make fmag
      TGeoVolume* subvol = node[i]->GetVolume();
      const int nsub = subvol->GetNdaughters();
      TGeoNode* subnod[nsub];
      for (int j = 0; j < nsub; j++) {
        subnod[j] = subvol->GetNode(j);
        subnod[j]->GetVolume()->SetTransparency(100);
      }
    }
  }

  TGeoNode *node_c = gGeoManager->GetCurrentNode();
  TEveGeoTopNode* tnode_c = new TEveGeoTopNode(gGeoManager, node_c);
  gEve->AddGlobalElement(tnode_c);


  if (_use_geofile) {
    geom->Close();
    delete geom;
  }
}

void 
PHEveDisplay::add_elements(TEveManager* geve)
{

  _top_list  = new TEveElementList("TOP");
  _dc_list   = new TEveElementList("DC");
  _hodo_list = new TEveElementList("HODO");
  _prop_list = new TEveElementList("PROP");
  _dp_list   = new TEveElementList("DP");
  _true_list = new TEveElementList("TRUE"); 
  
  geve->AddElement(_top_list);
  geve->AddElement(_dc_list,  _top_list);
  geve->AddElement(_hodo_list,_top_list);
  geve->AddElement(_prop_list,_top_list);
  geve->AddElement(_dp_list,  _top_list);
  geve->AddElement(_true_list,_top_list);
}

void
PHEveDisplay::config_bfields(const PHField *field)
{
  if (_use_fieldmap) {
    if (verbosity > 1)
      std::cout << "PHEveDisplay::config_bfields:" << " Field from file deprecated!!" << std::endl;
  } else {
    if (verbosity > 1)
      std::cout << "PHEveDisplay::config_bfields:" << " Using PHField fields for track propagation" << std::endl;
    cnt_prop = new TEveTrackPropagator("cnt_prop", "Current Propagator", new MappedField(field));
    cnt_prop->SetMaxStep(2);
  }

  cnt_prop->SetStepper(TEveTrackPropagator::kRungeKutta);
  cnt_prop->SetMaxR(300);
  cnt_prop->SetMaxZ(2500);
}

void
PHEveDisplay::go_fullscreen(TEveManager* geve)
{
  TEveViewer* cur_win = geve->GetDefaultViewer();
  TEveCompositeFrame* fEveFrame = cur_win->GetEveFrame();
  TEveWindow* return_cont = fEveFrame->GetEveParentAsWindow();

  if (return_cont && ! return_cont->CanMakeNewSlots())
  return_cont = 0;

  TEveCompositeFrameInPack* packframe = dynamic_cast<TEveCompositeFrameInPack*>(fEveFrame);
  if (packframe) {
    TGPack* pack = (TGPack*)(packframe->GetParent());
    pack->HideFrame(fEveFrame);
  }

  TGMainFrame* mf = new TGMainFrame(gClient->GetRoot(),_width,_height);
  gVirtualX->SetMWMHints(mf->GetId(),0,0,0);
  mf->SetCleanup(kLocalCleanup);

  TEveCompositeFrameInMainFrame *slot = new TEveCompositeFrameInMainFrame(mf, 0, mf);
  TEveWindowSlot* ew_slot = TEveWindow::CreateDefaultWindowSlot();
  ew_slot->PopulateEmptyFrame(slot);

  mf->AddFrame(slot, new TGLayoutHints(kLHintsNormal | kLHintsExpandX | kLHintsExpandY));
  slot->MapWindow();

  mf->Layout();
  mf->MapWindow();

  TEveWindow::SwapWindows(ew_slot, cur_win);

  ((TEveCompositeFrameInMainFrame*) fEveFrame)->
    SetOriginalSlotAndContainer(ew_slot, return_cont);

  geve->GetWindowManager()->HideAllEveDecorations();
  geve->GetWindowManager()->WindowUndocked(cur_win);

  int offset = -8;

  gVirtualX->MoveResizeWindow(mf->GetId(), 0, offset, _width, _height);
}

PHEveDisplay::MappedField::MappedField(const PHField *field) :
  TEveMagField(),
  _fieldmap(field)
{
}

TEveVectorD
PHEveDisplay::MappedField::GetFieldD(Double_t x, Double_t y, Double_t z) const
{
  // input pos in cm; PHFiled used CLHEP units
  double loc[4];
  loc[0]=x*cm; loc[1]=y*cm; loc[2]=z*cm; loc[3]=0;
  double bvec[3];
  //_fieldmap->get_bfield(&loc[0],&bvec[0]);
  _fieldmap->GetFieldValue(loc, bvec);
  const double unit = tesla; // gauss, tesla
  TEveVectorD vec(
      bvec[0]/unit,
		  bvec[1]/unit,
		  bvec[2]/unit); // unit is Gauss  (1Tesla = 10000 Gauss)
//  std::cout
//  << "GetFieldD: "
//  << " { " << x << ", " << y << ", " << z << " } "
//  << " { " << bvec[0]/tesla << ", " << bvec[1]/tesla << ", " << bvec[2]/tesla << " } "
//  << std::endl;
  //TEveVectorD vec(0, 0, 0); // unit is Gauss  (1Tesla = 10000 Gauss)

  return vec;
}

