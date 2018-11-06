/*
 * SQSlowCont.h
 */
#ifndef _H_SQSlowCont_H_
#define _H_SQSlowCont_H_
#include <phool/PHObject.h>
#include <iostream>
#include <limits>
#include <string>

class SQSlowCont : public PHObject {
public:
  virtual ~SQSlowCont() {}

  // PHObject virtual overloads

  virtual void         identify(std::ostream& os = std::cout) const {
    os << "---SQSlowCont base class------------" << std::endl;
  }
  virtual void      Reset() {};
  virtual int       isValid() const {return 0;}
  virtual SQSlowCont* Clone() const {return NULL;}
  virtual PHObject*   clone() const {return NULL;}

  virtual std::string get_time_stamp() const {return "";}
  virtual void set_time_stamp(const std::string a) {}

  virtual std::string get_name() const {return "";}
  virtual void set_name(const std::string a) {}

  virtual std::string get_value() const {return "";}
  virtual void set_value(const std::string a) {}

  virtual std::string get_type() const {return "";}
  virtual void set_type(const std::string a) {}

protected:
  SQSlowCont() {}
  
private:
  ClassDef(SQSlowCont, 1);
};

#endif /* _H_SQSlowCont_H_ */
