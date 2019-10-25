/// SQParamDeco.h
/**
 * A container class to hold a set of decoding parameters.
 */
#ifndef _H_SQ_PARAM_DECO_H_
#define _H_SQ_PARAM_DECO_H_
#include <string>
#include <map>
#include <iostream>
#include <phool/PHObject.h>

class SQParamDeco : public PHObject {
public:
  typedef std::map<std::string, std::string> ParamMap;
  typedef ParamMap::const_iterator ParamConstIter;
  typedef ParamMap::iterator                 Iter;

  virtual ~SQParamDeco() {}

  virtual void identify(std::ostream& os = std::cout) const = 0;
  virtual int        isValid() const = 0;
  virtual SQParamDeco* Clone() const = 0;
  virtual void         Reset() = 0;

  virtual bool        has_variable(const std::string name) const = 0; // {return false;}
  virtual std::string get_variable(const std::string name) const = 0; // {return "";}
  virtual void        set_variable(const std::string name, const std::string value) = 0; // {}

  virtual ParamConstIter begin() const = 0;
  virtual ParamConstIter   end() const = 0;
  
protected:
  SQParamDeco() {}

private:
  ClassDef(SQParamDeco, 1);
};

#endif /* _H_SQ_PARAM_DECO_H_ */
