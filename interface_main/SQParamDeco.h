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

/// An SQ interface class to hold a set of decoding parameters.
/**
 * Available only for real data.
 */
class SQParamDeco : public PHObject {
public:
  typedef std::map<std::string, std::string> ParamMap;
  typedef ParamMap::const_iterator ParamConstIter;
  typedef ParamMap::iterator            ParamIter;

  virtual ~SQParamDeco() {}

  virtual void identify(std::ostream& os = std::cout) const = 0;
  virtual int        isValid() const = 0;
  virtual SQParamDeco* Clone() const = 0;
  virtual void         Reset() = 0;

  virtual bool        has_variable(const std::string name) const = 0; ///< Return 'true' if a variable having 'name' exists.
  virtual std::string get_variable(const std::string name) const = 0; ///< Return the value of a variable having 'name'.  Return "" if no variable exists.
  virtual void        set_variable(const std::string name, const std::string value) = 0;

  virtual ParamConstIter begin() const = 0; ///< Return the begin iterator.
  virtual ParamConstIter   end() const = 0; ///< Return the end iterator.
  virtual unsigned int    size() const = 0; ///< Return the number of variables held.
  
protected:
  SQParamDeco() {}

private:
  ClassDef(SQParamDeco, 1);
};

#endif /* _H_SQ_PARAM_DECO_H_ */
