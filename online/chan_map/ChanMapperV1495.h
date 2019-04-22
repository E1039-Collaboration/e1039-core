#ifndef __CHAN_MAPPER_V1495_H__
#define __CHAN_MAPPER_V1495_H__
#include "ChanMapper.h"

class ChanMapperV1495 : public ChanMapper {
  struct MapItem {
    short roc;
    short board;
    short chan;
    std::string det_name;
    short det;
    short ele;
    short lvl;
  };
  typedef std::vector<MapItem> List_t;
  List_t m_list; ///< Used to keep all information in the added order.

  typedef std::tuple<short, short, short> DetEleLvl_t;
  typedef std::map<RocBoardChan_t, DetEleLvl_t> Map_t;
  Map_t m_map; ///< Used in Find() for better speed.

  std::map<std::string, short> m_map_name2id;

 public:
  ChanMapperV1495();
  virtual ~ChanMapperV1495() {;}

  void Add (const short roc, const short board, const short chan, const std::string det, const short ele, const short lvl);
  void Add (const short roc, const short board, const short chan, const std::string det_name, const short det_id, const short ele, const short lvl);

  //bool Find(const short roc, const short board, const short chan,  std::string& det, short& ele, short& lvl);
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
