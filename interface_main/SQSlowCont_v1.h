/*
 * SQSlowCont_v1.h
 */
#ifndef _H_SQSlowCont_v1_H_
#define _H_SQSlowCont_v1_H_
#include <phool/PHObject.h>
#include <iostream>
#include "SQSlowCont.h"

class SQSlowCont_v1 : public SQSlowCont {
public:
  SQSlowCont_v1();
  virtual ~SQSlowCont_v1() {}

  // PHObject virtual overloads
  void      identify(std::ostream& os = std::cout) const;
  void      Reset() {*this = SQSlowCont_v1();}
  int       isValid() const;
  SQSlowCont* Clone() const {return (new SQSlowCont_v1(*this));}
  PHObject*   clone() const {return (new SQSlowCont_v1(*this));}

  virtual std::string get_time_stamp() const {return _time_stamp;}
  virtual void set_time_stamp(const std::string a) { _time_stamp = a; }

  virtual std::string get_name() const {return _name;}
  virtual void set_name(const std::string a) { _name = a; }

  virtual std::string get_value() const {return _value;}
  virtual void set_value(const std::string a) { _value = a; }

  virtual std::string get_type() const {return _type;}
  virtual void set_type(const std::string a) { _type = a; }

private:
  std::string _time_stamp;
  std::string _name;
  std::string _value;
  std::string _type;

  ClassDef(SQSlowCont_v1, 1);
};

#endif /* _H_SQSlowCont_v1_H_ */
