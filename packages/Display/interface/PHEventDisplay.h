/*!
        \file PHEventDisplay.h
        \author Sookhyun Lee
        \brief event display interface,
              set parameters/switches, call detector modules, control display.
        \version $Revision: 1.2 $
        \date    $Date: 07/26/2016
*/

#ifndef __PHEVENTDISPLAY_H__
#define __PHEVENTDISPLAY_H__

#include <fun4all/SubsysReco.h>
#include <string>
#include <vector>

#ifndef __CINT__
#include<boost/shared_ptr.hpp>
#include<boost/thread/thread.hpp>
#else
class shared_ptr;
#endif

#include <pthread.h>

class PHEveDisplay;
class PHCompositeNode;
class TEveManager;
class TGLViewer;
class TGLAutoRotator;
class mPHEveModuleBase;
class TEveWindowSlot;

class PHEventDisplay : public SubsysReco
{
public:
  PHEventDisplay(int w, 
		  int h,
		  bool _use_fieldmap,
		  bool _use_geofile,
		  const std::string& _mapname,
		  const std::string& _geoname);

  ~PHEventDisplay();

  /// Module initialization
  int Init(PHCompositeNode *topNode);
  /// Run initialization
  int InitRun(PHCompositeNode *topNode);
  /// Event processing
  int process_event(PHCompositeNode *topNode);
  /// End of process
  int End(PHCompositeNode *topNode);

  /// Threaded access to Fun4All server
  //void run_evt_in_thread();
  void start_rotation();
  void go_fullscreen();
  void set_jet_pt_threshold(float pt){jet_pt_threshold = pt;}
  void set_jet_e_scale(float e_scale){jet_e_scale = e_scale;}
  void set_calo_e_threshold(float e){calo_e_threshold = e;}
  void set_dc_on(bool dc_on) {is_dc_on = dc_on;}
  void set_hodo_on(bool hodo_on) {is_hodo_on = hodo_on;}
  void set_prop_on(bool prop_on){is_prop_on = prop_on;}
  void set_truth_on(bool truth_on) {is_truth_on = truth_on;}
  void set_verbosity(int verb) {verbosity = verb;}

  void set_view_top();
  void set_view_side();
  void set_view_3d();

private:
  void reco_thread();
  void draw_default(PHCompositeNode *topNode);
  void update_scene(PHCompositeNode *topNode);

#ifndef _CLING__
  typedef boost::shared_ptr<mPHEveModuleBase> pBase;

  template<typename T> boost::shared_ptr<T>
    register_module()
    {
	pBase ptr = pBase(new T(_PHEveDisplay)); // Store in a base type shared_ptr
	_modules.push_back(ptr);
	return boost::dynamic_pointer_cast<T>(ptr); // Cast back to derived for return
    }
#endif

  bool _pending_update;

  //! Reconstruction mutex
#ifndef _CLING__
  std::vector<boost::shared_ptr<mPHEveModuleBase> > _modules;
  pthread_mutex_t _mutex;
  boost::shared_ptr<boost::thread> _update_thread;
  boost::shared_ptr<PHEveDisplay> _PHEveDisplay;
#endif

  TGLAutoRotator* _rot;

#ifndef _CLING__
  boost::shared_ptr<boost::thread> _status_thread;
#endif
  float jet_pt_threshold;
  float jet_e_scale;
  float calo_e_threshold;

  bool is_dc_on;
  bool is_hodo_on;
  bool is_prop_on;
  bool is_truth_on;

  bool use_fieldmap;
  bool use_geofile;
  int width;
  int height;
  std::string mapname;
  std::string geoname;
  int nevent;
  int verbosity;

  TEveWindowSlot* _slot_dc;
  TEveWindowSlot* _slot_dc_00;
  TEveWindowSlot* _slot_dc_01;
  TEveWindowSlot* _slot_dc_10;
  TEveWindowSlot* _slot_dc_11;


  TEveWindowSlot* _slot_hodo;
  TEveWindowSlot* _slot_hodo_xz;
  TEveWindowSlot* _slot_hodo_yz;


};

#endif // __PHEVENTDISPLAY_H__
