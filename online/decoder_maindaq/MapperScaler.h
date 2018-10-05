#ifndef __MAPPER_SCALER_H__
#define __MAPPER_SCALER_H__
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <string>

class MapperScaler {
  typedef std::tuple<short, short, short> RocBoardChan_t;
  std::map<RocBoardChan_t, std::string> m_map;

 public:
  MapperScaler() {;}
  virtual ~MapperScaler() {;}

  void ReadFile(const std::string fn_tsv);
  bool Find(const short roc, const short board, const short chan,  std::string& name);
};

#endif // __MAPPER_SCALER_H__
