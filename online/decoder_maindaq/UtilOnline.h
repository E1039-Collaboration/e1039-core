#ifndef _UTIL_ONLINE__H_
#define _UTIL_ONLINE__H_
#include <string>

class UtilOnline {
  static std::string m_dir_end;
  static std::string m_dir_coda;
  static std::string m_dir_dst;

 public:
  static void GetEndFileDir (const std::string dir) { m_dir_end  = dir; }
  static void GetCodaFileDir(const std::string dir) { m_dir_coda = dir; }
  static void GetDstFileDir (const std::string dir) { m_dir_dst  = dir; }

  static std::string GetEndFileDir () { return m_dir_end ; }
  static std::string GetCodaFileDir() { return m_dir_coda; }
  static std::string GetDstFileDir () { return m_dir_dst ; }

  static int CodaFile2RunNum(const std::string name);
  static std::string RunNum2CodaFile(const int run);
  static std::string RunNum2EndFile(const int run);
  static std::string RunNum2DstFile(const int run);
};

#endif /* _UTIL_ONLINE__H_ */
