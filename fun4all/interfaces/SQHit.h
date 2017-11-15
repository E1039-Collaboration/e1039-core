/*
 * SQHit.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQHit_H_
#define _H_SQHit_H_

#include <phool/PHObject.h>

#include <iostream>
#include <limits.h>
#include <string>

class SQHit : public PHObject {

public:



  virtual ~SQHit() {}

  // PHObject virtual overloads

  virtual void         identify(std::ostream& os = std::cout) const {
    os << "---SQHit base class------------" << std::endl;
  }
  virtual void         Reset() {};
  virtual int          isValid() const {return 0;}
  virtual SQHit*        Clone() const {return NULL;}

  // digitized hit info

  virtual int          get_hit_id() const                               {return UINT_MAX;}
  virtual void         set_hit_id(const int a)                          {}

  virtual short        get_detector_id() const                          {return UINT_MAX;}
  virtual void         set_detector_id(const short a)                   {}

  virtual short        get_element_id() const                           {return UINT_MAX;}
  virtual void         set_element_id(const short a)                    {}

  virtual float        get_tdc_time() const                             {return UINT_MAX;}
  virtual void         set_tdc_time(const float a)                      {}

  virtual float        get_drift_distance() const                       {return UINT_MAX;}
  virtual void         set_drift_distance(const float a)                {}

  virtual float        get_pos() const                                  {return UINT_MAX;}
  virtual void         set_pos(const float a)                           {}

  virtual bool         is_in_time() const = 0;
  virtual void         set_in_time(const bool a) = 0;

  virtual bool         is_hodo_mask() const = 0;
  virtual void         set_hodo_mask(const bool a) = 0;

  virtual bool         is_trigger_mask() const = 0;
  virtual void         set_trigger_mask(const bool a) = 0;

  enum HitQuality
  {
      InTime = 1<<0,
      HodoMask = 1<<1,
      TriggerMask = 1<<2
  };

protected:
  SQHit() {}

private:

  ClassDef(SQHit, 1);
};




#endif /* _H_SQHit_H_ */
