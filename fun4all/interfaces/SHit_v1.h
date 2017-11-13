/*
 * SHit_v1.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SHit_v1_H_
#define _H_SHit_v1_H_


#include "SHit.h"

#include <phool/PHObject.h>
#include <iostream>

class SHit_v1 : public SHit {

public:

  SHit_v1();
  virtual ~SHit_v1() {}

  // PHObject virtual overloads

  void         identify(std::ostream& os = std::cout) const;
  void         Reset() {*this = SHit_v1();}
  int          isValid() const;
  SHit*        Clone() const {return (new SHit_v1(*this));}

  // digitized hit info

  virtual int          get_hit_id() const                        {return _hit_id;}
  virtual void         set_hit_id(const int a)                   {_hit_id = a;}

  virtual short        get_detector_id() const                   {return _detector_id;}
  virtual void         set_detector_id(const short a)            {_detector_id = a;}

  virtual short        get_element_id() const                    {return _element_id;}
  virtual void         set_element_id(const short id)            {_element_id = id;}

  virtual float        get_tdc_time() const                      {return _tdc_time;}
  virtual void         set_tdc_time(const float a)               {_tdc_time=a;}

  virtual float        get_drift_distance() const                {return _drift_distance;}
  virtual void         set_drift_distance(const float a)         {_drift_distance=a;}

  virtual float        get_pos() const                           {return _pos;}
  virtual void         set_pos(const float a)                    {_pos=a;}

  virtual bool         is_in_time() const                        {return (_flag&(SHit::InTime)) != 0;}
  virtual void         set_in_time(const bool a)                 {a? (_flag |= SHit::InTime) : (_flag &= ~SHit::InTime);}

  virtual bool         is_hodo_mask() const                      {return (_flag&(SHit::HodoMask)) != 0;}
  virtual void         set_hodo_mask(const bool a)               {a? (_flag |= SHit::HodoMask) : (_flag &= ~SHit::HodoMask);}

  virtual bool         is_trigger_mask() const                   {return (_flag&(SHit::TriggerMask)) != 0;}
  virtual void         set_trigger_mask(const bool a)            {a? (_flag |= SHit::TriggerMask) : (_flag &= ~SHit::TriggerMask);}

private:

  int _hit_id;                   //< unique identifier within container
  short _detector_id;            //<
  short _element_id;             //< element id

  float _tdc_time;               //<
  float _drift_distance;         //<
  float _pos;                    //<

  unsigned short _flag;           //<

  ClassDef(SHit_v1, 1);
};


#endif /* _H_SHit_v1_H_ */
