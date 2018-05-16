// $Id: PHG4DSTReader.h,v 1.7 2015/02/27 23:42:23 jinhuang Exp $

/*!
 * \file PHG4DSTReader.h
 * \brief 
 * \author Jin Huang <jhuang@bnl.gov>
 * \version $Revision: 1.7 $
 * \date $Date: 2015/02/27 23:42:23 $
 */
#ifndef PHG4DSTREADER_H_
#define PHG4DSTREADER_H_

#include <fun4all/SubsysReco.h>


#ifndef __CINT__

#include <memory>

#endif

#include <iostream>
#include <string>
#include <set>
#include <vector>

class PHCompositeNode;

class PHG4Particle;

class TClonesArray;
class TTree;


/*!
 * \brief PHG4DSTReader save information from DST to an evaluator, which could include hit. particle, vertex, towers and jet (to be activated)
 */

class PHG4DSTReader : public SubsysReco
{
public:
  PHG4DSTReader(const std::string &filename);
  virtual
  ~PHG4DSTReader();

  //! full initialization
  int
  Init(PHCompositeNode *);

  //! event processing method
  int
  process_event(PHCompositeNode *);

  //! end of run method
  int
  End(PHCompositeNode *);

  void
  AddNode(const std::string &name)
  {
    _node_postfix.push_back(name);
  }

  void
  AddHit(const std::string &name)
  {
    _node_postfix.push_back(name);
  }

  //! load all particle in truth info module?
  //! size could be very large, e.g. showers
  void
  set_load_all_particle(bool b)
  {
    _load_all_particle = b;
  }

  //! load all particle that produced a saved hit
  void
  set_load_active_particle(bool b)
  {
    _load_active_particle = b;
  }

  //! Switch for saving any particles at all
  void
  set_save_particle(bool b)
  {
    _save_particle = b;
  }

  //! Switch for vertex
  void
  set_save_vertex(bool b)
  {
    _save_vertex = b;
  }

protected:

  std::vector<std::string> _node_postfix;
//  std::vector<std::string> _node_name;
  int nblocks;

#ifndef __CINT__

  typedef std::shared_ptr<TClonesArray> arr_ptr;

  struct record
  {
    unsigned int _cnt;
    std::string _name;
    arr_ptr _arr;
    TClonesArray * _arr_ptr;

    enum enu_type
    {
      typ_hit, typ_part, typ_vertex
    };
    enu_type _type;
  };
  typedef std::vector<record> records_t;
  records_t _records;
  /*
  typedef PHG4Particlev2 part_type;
  typedef PHG4HitEval hit_type;
  typedef PHG4VtxPointv1 vertex_type;
  typedef RawTowerv1 RawTower_type;
  typedef JetV1 PHPyJet_type;
  */
#endif

  int _event;

  std::string _out_file_name;

//  TFile * _file;
  TTree * _T;

  //! master switch to save particles
  bool _save_particle;

  //! load all particle in truth info module?
  bool _load_all_particle;

  //! load all particle that produced a saved hit
  bool _load_active_particle;

  typedef std::set<int> PartSet_t;
  PartSet_t _particle_set;
  PartSet_t _vertex_set;

  //! save vertex for particles?
  bool _save_vertex;

#ifndef __CINT__

  //! add a particle and associated vertex if _save_vertex
  void
  add_particle(record & rec, PHG4Particle * part);

#endif

  void
  build_tree();
};

#endif /* PHG4DSTREADER_H_ */
