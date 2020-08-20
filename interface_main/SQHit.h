/*
 * SQHit.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQHit_H_
#define _H_SQHit_H_

#include <phool/PHObject.h>
#include "g4main/PHG4HitDefs.h"

#include <iostream>
#include <limits>
#include <string>
#include <map>

class SQHit : public PHObject {

public:
  typedef std::map<short, float> CellMap;    //!< key -> plateID, float -> edep

  SQHit() {}
  virtual ~SQHit() {}

  // PHObject virtual overloads

  virtual void         identify(std::ostream& os = std::cout) const {
    os << "---SQHit base class------------" << std::endl;
  }
  virtual void         Reset() {};
  virtual int          isValid() const {return 0;}
  virtual SQHit*       Clone() const {return NULL;}

  // digitized hit info

  virtual int          get_hit_id() const                               {return std::numeric_limits<int>::max();}
  virtual void         set_hit_id(const int a)                          {}

  virtual short        get_detector_id() const                          {return std::numeric_limits<short>::max();}
  virtual void         set_detector_id(const short a)                   {}

  virtual short        get_element_id() const                           {return std::numeric_limits<short>::max();}
  virtual void         set_element_id(const short a)                    {}

  virtual short        get_tower_id() const                             {return std::numeric_limits<short>::max();}
  virtual void         set_tower_id(const short a)                      {}

  virtual short        get_level() const                                {return std::numeric_limits<short>::max();}
  virtual void         set_level(const short a)                         {}

  virtual float        get_tdc_time() const                             {return std::numeric_limits<float>::max();}
  virtual void         set_tdc_time(const float a)                      {}

  virtual float        get_drift_distance() const                       {return std::numeric_limits<float>::max();}
  virtual void         set_drift_distance(const float a)                {}

  virtual float        get_pos() const                                  {return std::numeric_limits<float>::max();}
  virtual void         set_pos(const float a)                           {}

  virtual float        get_edep() const                                 {return std::numeric_limits<float>::max();}
  virtual void         set_edep(const float a)                          {}

  virtual int          get_track_id() const                             {return std::numeric_limits<int>::max();}
  virtual void         set_track_id(const int a)                        {}

  virtual PHG4HitDefs::keytype          get_g4hit_id() const                             {return std::numeric_limits<PHG4HitDefs::keytype>::max();}
  virtual void                          set_g4hit_id(const PHG4HitDefs::keytype a)       {}

  virtual float        get_truth_x() const                              {return std::numeric_limits<float>::max();}
  virtual void         set_truth_x(const float a)                       {}

  virtual float        get_truth_y() const                              {return std::numeric_limits<float>::max();}
  virtual void         set_truth_y(const float a)                       {}

  virtual float        get_truth_z() const                              {return std::numeric_limits<float>::max();}
  virtual void         set_truth_z(const float a)                       {}

  virtual float        get_truth_px() const                             {return std::numeric_limits<float>::max();}
  virtual void         set_truth_px(const float a)                      {}

  virtual float        get_truth_py() const                             {return std::numeric_limits<float>::max();}
  virtual void         set_truth_py(const float a)                      {}

  virtual float        get_truth_pz() const                             {return std::numeric_limits<float>::max();}
  virtual void         set_truth_pz(const float a)                      {}

  virtual bool         is_in_time() const {return false;}
  virtual void         set_in_time(const bool a) {}

  virtual bool         is_hodo_mask() const {return false;}
  virtual void         set_hodo_mask(const bool a) {}

  virtual bool         is_trigger_mask() const {return false;}
  virtual void         set_trigger_mask(const bool a) {}

  virtual unsigned int  get_n_cells() const                { return std::numeric_limits<int>::max(); }
  virtual CellMap       get_cells()   const                { CellMap none; return none; } 
  virtual float         get_cell(unsigned int i) const     { return std::numeric_limits<float>::max(); }
  virtual void          add_cell(unsigned int i, float a)  {}

  enum HitQuality
  {
      InTime = 1<<0,
      HodoMask = 1<<1,
      TriggerMask = 1<<2
  };

//protected:
//  SQHit() {}

private:

  ClassDef(SQHit, 1);
};




#endif /* _H_SQHit_H_ */
