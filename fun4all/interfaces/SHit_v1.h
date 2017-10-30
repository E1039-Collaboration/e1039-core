/*
 * SHit_v1.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef FUN4ALL_INTERFACES_SHIT_V1_H_
#define FUN4ALL_INTERFACES_SHIT_V1_H_


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
  SHit*     Clone() const {return (new SHit_v1(*this));}

  // digitized hit info

  virtual unsigned int get_id() const                        {return _hit_id;}
  virtual void         set_id(unsigned int id)               {_hit_id = id;}

  virtual std::string  get_detector_name() const                     {return _detector_name;}
  virtual void         set_detector_name(const std::string& name)         {_detector_name = name;}

  virtual unsigned int get_element_id() const                     {return _element_id;}
  virtual void         set_element_id(unsigned int id)         {_element_id = id;}

  virtual float        get_drift_distance() const                       {return _drift_distance;}
  virtual void         set_drift_distance(const float distance)             {_drift_distance=distance;}

private:

  unsigned int _hit_id;                //< unique identifier within container
  std::string _detector_name;
  int _element_id;             //< element id
  float _drift_distance;        //<

  ClassDef(SHit_v1, 1);
};


#endif /* FUN4ALL_INTERFACES_SHIT_V1_H_ */
