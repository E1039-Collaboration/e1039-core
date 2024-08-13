#ifndef _UTIL_ONLINE__H_
#define _UTIL_ONLINE__H_
#include <string>
#include <vector>

class UtilOnline {
  static std::string m_dir_end;
  static std::string m_dir_coda;
  static std::string m_dir_dst;
  static std::string m_dir_eddst;
  static std::string m_dir_onlmon;
  static std::string m_sch_maindaq;

 public:
  static void UseOutputLocationForDevel();

  static void SetEndFileDir   (const std::string dir);
  static void SetCodaFileDir  (const std::string dir);
  static void SetDstFileDir   (const std::string dir);
  static void SetEDDstFileDir (const std::string dir);
  static void SetOnlMonDir    (const std::string dir);
  static void SetSchemaMainDaq(const std::string sch);

  static std::string GetEndFileDir   () { return m_dir_end    ; }
  static std::string GetCodaFileDir  () { return m_dir_coda   ; }
  static std::string GetDstFileDir   () { return m_dir_dst    ; }
  static std::string GetEDDstFileDir () { return m_dir_eddst  ; }
  static std::string GetOnlMonDir    () { return m_dir_onlmon ; }
  static std::string GetSchemaMainDaq() { return m_sch_maindaq; }

  static int CodaFile2RunNum(const std::string name);
  static std::string RunNum2CodaFile(const int run);
  static std::string RunNum2EndFile(const int run);
  static std::string RunNum2DstFile(const int run);
  static std::string RunNum2EDDstFile(const int run);

  static std::string GetSpillDstDir (const int run);
  static std::string GetSpillDstFile(const int run, const int spill);
  static std::string GetSpillDstPath(const int run, const int spill);
  static std::vector<std::string> GetListOfSpillDSTs(const int run, const std::string dir_dst="");

  static std::string GetCodaFilePath(const int run);
  static std::string GetEndFilePath(const int run);
  static std::string GetDstFilePath(const int run);
  static std::string GetEDDstFilePath(const int run);

  static std::string Run6(const int run, const int digit=6);
  static std::string Spill9(const int spill, const int digit=9);
};

#endif /* _UTIL_ONLINE__H_ */
