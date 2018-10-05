#ifndef __MAPPER_V1495_H__
#define __MAPPER_V1495_H__
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <string>

class MapperV1495 {
  std::map<std::string, short> m_map_name2id;

  typedef std::tuple<short, short, short> RocBoardChan_t;
  typedef std::tuple<std::string, short, short> DetEleLvl_t;
  std::map<RocBoardChan_t, DetEleLvl_t> m_map;

 public:
  MapperV1495();
  virtual ~MapperV1495() {;}

  void ReadFile(const std::string fn_tsv);
  bool Find(const short roc, const short board, const short chan,  std::string& det, short& ele, short& lvl);
  bool Find(const short roc, const short board, const short chan,        short& det, short& ele, short& lvl);
};

#endif // __MAPPER_V1495_H__

