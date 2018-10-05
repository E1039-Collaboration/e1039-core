#ifndef __MAPPER_TAIWAN_H__
#define __MAPPER_TAIWAN_H__
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <string>

class MapperTaiwan {
  std::map<std::string, short> m_map_name2id;

  typedef std::tuple<short, short, short> RocBoardChan_t;
  typedef std::pair<std::string, short> DetEle_t;
  std::map<RocBoardChan_t, DetEle_t> m_map;

 public:
  MapperTaiwan();
  virtual ~MapperTaiwan() {;}

  void ReadFile(const std::string fn_tsv);
  bool Find(const short roc, const short board, const short chan,  std::string& det, short& ele);
  bool Find(const short roc, const short board, const short chan,        short& det, short& ele);

  //bool FindInv(const std::string det, const int ele,  int& roc, int& board, int& chan);

  //bool FindInv(const int roc, const int board, const int chan,  int& plane, int& ele);

  //void Print   (std::ostream& ofs);
  //void PrintInv(std::ostream& ofs);
};

#endif // __MAPPER_TAIWAN_H__
