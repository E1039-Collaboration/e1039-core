#ifndef _SQRECOCONFIG_H
#define _SQRECOCONFIG_H

#include <phool/PHFlag.h>

class SQRecoConfig: public PHFlag
{
public:
  static SQRecoConfig* instance();
  void setDefaults();
  void init(int runNo = 0);
  void Print() const;

  //overide the virtual function to expand the environmental variables
  virtual void set_CharFlag(const std::string &name, const std::string &flag);

protected:
  SQRecoConfig();
  std::string ExpandEnvironmentals(const std::string& input);

  static SQRecoConfig* __instance;

};

#endif 
