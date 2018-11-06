/*
 * SQScaler_v1.h
 */
#ifndef _H_SQScaler_v1_H_
#define _H_SQScaler_v1_H_
#include <phool/PHObject.h>
#include <iostream>
#include "SQScaler.h"

class SQScaler_v1 : public SQScaler {
public:
  SQScaler_v1();
  virtual ~SQScaler_v1() {}

  // PHObject virtual overloads
  void      identify(std::ostream& os = std::cout) const;
  void      Reset() {*this = SQScaler_v1();}
  int       isValid() const;
  SQScaler* Clone() const {return (new SQScaler_v1(*this));} ///< Use Clone() or clone()??
  PHObject* clone() const {return (new SQScaler_v1(*this));} ///< Use Clone() or clone()??

  virtual ScalerType_t get_type() const { return _type; }
  virtual void set_type(const ScalerType_t a) { _type = a; }

  virtual std::string get_name() const { return _name; }
  virtual void set_name(const std::string a) { _name = a; }

  virtual int  get_count() const { return _count; }
  virtual void set_count(const int a) { _count = a; }

private:
  ScalerType_t _type;
  std::string _name;
  int _count;

  ClassDef(SQScaler_v1, 1);
};

#endif /* _H_SQScaler_v1_H_ */
