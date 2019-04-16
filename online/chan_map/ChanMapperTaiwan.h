#ifndef __CHAN_MAPPER_TAIWAN_H__
#define __CHAN_MAPPER_TAIWAN_H__
#include <tuple>
#include "ChanMapper.h"

class ChanMapperTaiwan : public ChanMapper {
  typedef std::tuple<short, short, short> RocBoardChan_t;
  typedef std::pair<std::string, short> DetEle_t;
  typedef std::map<RocBoardChan_t, DetEle_t> Map_t;
  Map_t m_map;
  std::map<std::string, short> m_map_name2id;

 public:
  ChanMapperTaiwan();
  virtual ~ChanMapperTaiwan() {;}

  using ChanMapper::ReadFromFile;
  using ChanMapper::ReadFromDB;

  void Add (const short roc, const short board, const short chan, const std::string det, const short ele);
  bool Find(const short roc, const short board, const short chan,  std::string& det, short& ele);
  bool Find(const short roc, const short board, const short chan,        short& det, short& ele);
  void Print(std::ostream& os);

 protected:
  int  ReadFileCont(LineList& lines);
  int WriteFileCont(std::ostream& os);

  void  ReadDbTable(DbSvc& db);
  void WriteDbTable(DbSvc& db);

  void InitNameMap();
};

#endif // __CHAN_MAPPER_TAIWAN_H__
