/*
 * SQMCHit_v1.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */

#ifndef _H_SQMCHit_v1_H_
#define _H_SQMCHit_v1_H_


#include <phool/PHObject.h>
#include <iostream>

#include "SQHit_v1.h"

class SQMCHit_v1 : public SQHit_v1 {

public:

  SQMCHit_v1();
  virtual ~SQMCHit_v1() {}

  // PHObject virtual overloads

  void         identify(std::ostream& os = std::cout) const;
  void         Reset() {*this = SQMCHit_v1();}
  int          isValid() const;
  SQHit*        Clone() const {return (new SQMCHit_v1(*this));}

  virtual int          get_track_id() const                             {return _track_id;}
  virtual void         set_track_id(const int a)                        {_track_id = a;}

  virtual PHG4HitDefs_keytype          get_g4hit_id() const                             {return _g4hit_id;}
  virtual void                          set_g4hit_id(const PHG4HitDefs_keytype a)                        {_g4hit_id = a;}

  virtual float        get_truth_x() const                              {return _truth_x;}
  virtual void         set_truth_x(const float a)                       {_truth_x = a;}

  virtual float        get_truth_y() const                              {return _truth_y;}
  virtual void         set_truth_y(const float a)                       {_truth_y = a;}

  virtual float        get_truth_z() const                              {return _truth_z;}
  virtual void         set_truth_z(const float a)                       {_truth_z = a;}

  virtual float        get_truth_px() const                             {return _truth_px;}
  virtual void         set_truth_px(const float a)                      {_truth_px = a;}

  virtual float        get_truth_py() const                             {return _truth_py;}
  virtual void         set_truth_py(const float a)                      {_truth_py = a;}

  virtual float        get_truth_pz() const                             {return _truth_pz;}
  virtual void         set_truth_pz(const float a)                      {_truth_pz = a;}

private:

  int _track_id;  ///< truth track id
  PHG4HitDefs_keytype _g4hit_id;  ///< truth hit id

  float _truth_x;
  float _truth_y;
  float _truth_z;

  float _truth_px;
  float _truth_py;
  float _truth_pz;

  ClassDef(SQMCHit_v1, 2);
};


#endif /* _H_SQMCHit_v1_H_ */
