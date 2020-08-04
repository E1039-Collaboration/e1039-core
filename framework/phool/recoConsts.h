#ifndef RECOCONSTS_H__
#define RECOCONSTS_H__

#include <string>
#include "PHFlag.h"

/**
 * This class instantiates a singleton object that can be accessed via recoConsts::instance()
 * User is expected to do the following at the begining of the user macro:
 *   - recoConsts* rc = recoConsts::instance();    // get an instance of the recoConsts
 *   - rc->init(runNo);    // initiatiate the constants by run numer, or
 *   - rc->init("cosmic"); // initiatiate the constants by a pre-defined parameter set, or
 *   - rc->initfile("const.txt"); //initialize the constants by reading a file, or
 *   - rc->set_DoubleFlag("something", somevalue);  // set specific constants individually
 */ 

class recoConsts: public PHFlag
{
public:
  static recoConsts* instance();

  //! set the default value for all the constants needed - user is supposed to add a default value here to introduce new constants
  void set_defaults();

  //! initialize the constants by the runNo - not implemented yet
  void init(int runNo = 0, bool verbose = false);

  //! initialize the constants by pre-defined parameter set name - not implemented yet
  void init(const std::string& setname, bool verbose = false);

  //! initialize by reading a file
  void initfile(const std::string& filename, bool verbose = false);

  //! overide the virtual function to expand the environmental variables
  virtual void set_CharFlag(const std::string& name, const std::string& flag);

  //! print all the parameters
  void Print() const;

protected: 
  recoConsts();
  std::string ExpandEnvironmentals(const std::string& input);

  static recoConsts* __instance;

};

#endif /* __RECOCONSTS_H__ */
