#ifndef __CHAN_MAPPER_V1495_H__
#define __CHAN_MAPPER_V1495_H__
#include <tuple>
#include "ChanMapper.h"

class ChanMapperV1495 : public ChanMapper {
  typedef std::tuple<short, short, short> RocBoardChan_t;
  typedef std::tuple<std::string, short, short> DetEleLvl_t;
  typedef std::map<RocBoardChan_t, DetEleLvl_t> Map_t;
  Map_t m_map;
  std::map<std::string, short> m_map_name2id;

 public:
  ChanMapperV1495();
  virtual ~ChanMapperV1495() {;}

  void Add (const short roc, const short board, const short chan, const std::string det, const short ele, const short lvl);
  bool Find(const short roc, const short board, const short chan,  std::string& det, short& ele, short& lvl);
  bool Find(const short roc, const short board, const short chan,        short& det, short& ele, short& lvl);
  void Print(std::ostream& os);

 protected:
  int  ReadFileCont(LineList& lines);
  int WriteFileCont(std::ostream& os);

  void  ReadDbTable(DbSvc& db);
  void WriteDbTable(DbSvc& db);

  void InitNameMap();
};

#endif // __CHAN_MAPPER_V1495_H__
