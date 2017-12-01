/*
 * SQHit_v1.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */

#ifndef _H_SQHit_v1_H_
#define _H_SQHit_v1_H_


#include <phool/PHObject.h>
#include <iostream>

#include "SQHit.h"

class SQHit_v1 : public SQHit {

public:

  SQHit_v1();
  virtual ~SQHit_v1() {}

  // PHObject virtual overloads

  void         identify(std::ostream& os = std::cout) const;
  void         Reset() {*this = SQHit_v1();}
  int          isValid() const;
  SQHit*        Clone() const {return (new SQHit_v1(*this));}

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

  //virtual float        get_pos() const                           {return _pos;}
  //virtual void         set_pos(const float a)                    {_pos=a;}

  virtual bool         is_in_time() const                        {return (_flag&(SQHit::InTime)) != 0;}
  virtual void         set_in_time(const bool a)                 {a? (_flag |= SQHit::InTime) : (_flag &= ~SQHit::InTime);}

  virtual bool         is_hodo_mask() const                      {return (_flag&(SQHit::HodoMask)) != 0;}
  virtual void         set_hodo_mask(const bool a)               {a? (_flag |= SQHit::HodoMask) : (_flag &= ~SQHit::HodoMask);}

  virtual bool         is_trigger_mask() const                   {return (_flag&(SQHit::TriggerMask)) != 0;}
  virtual void         set_trigger_mask(const bool a)            {a? (_flag |= SQHit::TriggerMask) : (_flag &= ~SQHit::TriggerMask);}

private:

  int _hit_id;                   ///< hitID
  short _detector_id;            ///< mapping from detector name to ID
  short _element_id;             ///< elementID

  float _tdc_time;               ///< tdcTime
  float _drift_distance;         ///< driftDistance
  //float _pos;                    ///< pos?

  unsigned short _flag;          ///< bits collection

  ClassDef(SQHit_v1, 1);
};


#endif /* _H_SQHit_v1_H_ */
