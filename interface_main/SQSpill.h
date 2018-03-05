/*
 * SQSpill.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQSpill_H_
#define _H_SQSpill_H_

#include <phool/PHObject.h>

#include <iostream>
#include <limits>
#include <string>

class SQSpill : public PHObject {

public:



  virtual ~SQSpill() {}

  // PHObject virtual overloads

  virtual void         identify(std::ostream& os = std::cout) const {
    os << "---SQSpill base class------------" << std::endl;
  }
  virtual void         Reset() {};
  virtual int          isValid() const {return 0;}
  virtual SQSpill*        Clone() const {return NULL;}

  // digitized hit info

  virtual int          get_run_id() const                               {return std::numeric_limits<int>::max();}
  virtual void         set_run_id(const int a)                          {}

  virtual int          get_spill_id() const                               {return std::numeric_limits<int>::max();}
  virtual void         set_spill_id(const int a)                          {}

  virtual short        get_target_pos() const                               {return std::numeric_limits<short>::max();}
  virtual void         set_target_pos(const short a)                          {}

protected:
  SQSpill() {}

private:

  ClassDef(SQSpill, 1);
};




#endif /* _H_SQSpill_H_ */
