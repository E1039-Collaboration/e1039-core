// Do yourself and others a favour, please sort variable/function name
// according to the roman alphabet

#ifndef RECOCONSTS_H__
#define RECOCONSTS_H__

#include <string>

#include "PHFlag.h"

class recoConsts: public PHFlag
{
public:
  static recoConsts* instance();
  void set_defaults();

  void init(int runNo = 0, bool verbose = false);
  void init(const std::string& filename, bool verbose = false);

  //overide the virtual function to expand the environmental variables
  virtual void set_CharFlag(const std::string& name, const std::string& flag);

  void Print() const;

protected: 
  recoConsts();
  std::string ExpandEnvironmentals(const std::string& input);

  static recoConsts* __instance;

};

#endif /* __RECOCONSTS_H__ */
