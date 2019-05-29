#ifndef _UTIL_ONLINE__H_
#define _UTIL_ONLINE__H_
#include <string>

//namespace UtilOnline {
class UtilOnline {
public:
  static std::string GetCodaFileDir();
  static std::string GetDstFileDir();

  static int CodaFile2RunNum(const std::string name);
  static std::string RunNum2CodaFile(const int run);

  static std::string RunNum2DstFile(const int run);
};

#endif /* _UTIL_ONLINE__H_ */
