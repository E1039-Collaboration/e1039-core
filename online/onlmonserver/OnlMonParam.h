#ifndef _ONL_MON_PARAM__H_
#define _ONL_MON_PARAM__H_
#include <string>
class OnlMonClient;

/// Class to get OnlMon run-dependent parameters from the configuration directory.
/**
 * The OnlMon client often needs run-dependent parameters when judging
 * the goodness of data quality in `OnlMonClient::DrawMonitor()`.
 * Such parameters should be first defined in TSV files under the configuration directory, 
 * which is by default `/data2/e1039/onlmon/param`.
 * They are then read via `OnlMonParam::GetIntParam()` etc.
 *
 * This class aborts the program when a value of the requested parameter doesn't exist.
 * You can chage this behavior by `OnlMonParam::DoAssert()`.
 *
 * The run number is automatically selected via `recoConsts`.
 * You can use `OnlMonParam::RunID()` when you have to fix it.
 * 
 * Typical usage: 
 * @code
 * OnlMonParam param(this);
 * int n_ttdc = param.GetIntParam("N_TAIWAN_TDC");
 * @endcode
 */
class OnlMonParam {
 protected:
  std::string m_name;
  std::string m_dir_base;
  int  m_run_id;
  int  m_verb;
  bool m_do_assert;

 public:
  OnlMonParam(const OnlMonClient* omc);
  OnlMonParam(const std::string name);
  virtual ~OnlMonParam();

  void        BaseDir(const std::string dir_base) { m_dir_base = dir_base; }
  std::string BaseDir() const              { return m_dir_base; }

  void RunID(const int run_id) { m_run_id = run_id; }
  int  RunID() const    { return m_run_id; }

  void Verbosity(const bool verb) { m_verb = verb; }
  int  Verbosity() const   { return m_verb; }

  void DoAssert(const bool val) { m_do_assert = val; }
  bool DoAssert() const  { return m_do_assert; }

  std::string GetCharParam  (const std::string par_name);
  double      GetDoubleParam(const std::string par_name);
  int         GetIntParam   (const std::string par_name);
  bool        GetBoolParam  (const std::string par_name);

 protected:
  std::string FindParamInDir(const std::string par_name);
  bool FindParamInFile(const std::string file_name, const int run, const std::string par_name, std::string& par_value);
};

#endif // _ONL_MON_PARAM__H_
