/*
 * SHit.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SHit_H_
#define _H_SHit_H_

#include <phool/PHObject.h>

#include <iostream>
#include <limits.h>
#include <string>

class SHit : public PHObject {

public:

  virtual ~SHit() {}

  // PHObject virtual overloads

  virtual void         identify(std::ostream& os = std::cout) const {
    os << "---SHit base class------------" << std::endl;
  }
  virtual void         Reset() {};
  virtual int          isValid() const {return 0;}
  virtual SHit*     Clone() const {return NULL;}

  // digitized hit info

  virtual unsigned int get_id() const                        {return UINT_MAX;}
  virtual void         set_id(unsigned int id)               {}

  virtual std::string  get_detector_name() const                     {std::string a(""); return a;}
  virtual void         set_detector_name(const std::string& name)         {}

  virtual unsigned int get_element_id() const                     {return UINT_MAX;}
  virtual void         set_element_id(unsigned int id)         {}

  virtual float        get_drift_distance() const                       {return UINT_MAX;}
  virtual void         set_drift_distance(const float distance)             {}

protected:
  SHit() {}

private:

  ClassDef(SHit, 1);
};




#endif /* _H_SHit_H_ */
