/*
        \file PHEveDisplay.cxx
        \author Sookhyun Lee
        \brief main display module, load geometry, configure b-field, draw default
        \version $Revision: 1.2 $
        \date    $Date: 07/26/2016
*/

#ifndef __PHEVEDISPLAY_H__
#define __PHEVEDISPLAY_H__

#include<string>
#include "TEveTrackPropagator.h"

class TEveManager;
class TEveElementList;
class PHField;
class PHCompositeNode;
class PHEventDisplay;

class PHEveDisplay
{
public:

  class MappedField : public TEveMagField
  {
  public:
    MappedField(const PHField *field);
    ~MappedField(){};
    using TEveMagField::GetFieldD;

    virtual TEveVectorD GetFieldD(Double_t x, Double_t y, Double_t z) const;
  private:
    const PHField* _fieldmap;
  };

  PHEveDisplay(int w,
	       int h,
	       bool use_fieldmap,
	       bool use_goefile,
	       const std::string& mapname,
	       const std::string& filename,
	       int verb);
  ~PHEveDisplay();

  void load_geometry(PHCompositeNode* topNode, TEveManager* geve);
  void add_elements(TEveManager* geve);
  void config_bfields(const PHField *field);
  void go_fullscreen(TEveManager* geve);
 
  /// Set a pointer to the underlying TEveManager 
  void set_eve_manager(TEveManager* geve){_eve_manager = geve;}
  /// Return a pointer to the underlying TEveManager
  TEveManager* get_eve_manager() const { return _eve_manager;}
  TEveTrackPropagator* get_cnt_prop() const { return cnt_prop; }
  TEveElementList* get_top_list() const { return _top_list;}
  TEveElementList* get_dc_list() const { return _dc_list;}
  TEveElementList* get_hodo_list() const { return _hodo_list;}
  TEveElementList* get_prop_list() const {return _prop_list;}
  TEveElementList* get_dp_list() const {return _dp_list;}
  TEveElementList* get_true_list() const { return _true_list;}  
  
  void set_jet_pt_threshold(float pt){_jet_pt_threshold = pt;}
  float get_jet_pt_threshold() const {return _jet_pt_threshold;}
  void set_jet_e_scale(float e_scale){_jet_e_scale = e_scale;}
  float get_jet_e_scale() const {return _jet_e_scale;}
  void set_calo_e_threshold(float e){_calo_e_threshold = e;}
  float get_calo_e_threshold() const {return _calo_e_threshold;}
  int get_verbosity() const {return verbosity;}
  void set_verbosity(const int a) {verbosity = a;}

protected:

  TEveManager *_eve_manager; 
  TEveElementList* _top_list;
  TEveElementList* _dc_list;
  TEveElementList* _hodo_list;
  TEveElementList* _prop_list;
  TEveElementList* _dp_list;
  TEveElementList* _true_list;

  TEveTrackPropagator* cnt_prop;
  MappedField* mapped_field;


  int _width, _height;
  bool _use_fieldmap;
  bool _use_geofile;
  float _jet_pt_threshold;
  float _jet_e_scale;
  float _calo_e_threshold;

  std::string map_filename;
  std::string geo_filename;

  int verbosity;
};

#endif // __PHEVEDISPLAY_H__
