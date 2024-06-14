#ifndef __CHAN_MAP_V1495_H__
#define __CHAN_MAP_V1495_H__
#include <unordered_map>
#include "RunParamBase.h"

class ChanMapV1495 : public ChanMapBase {
  struct MapItem {
    short roc;
    int   board;
    short chan;
    std::string det_name;
    short det;
    short ele;
    short lvl;
  };
  typedef std::vector<MapItem> List_t;
  List_t m_list; ///< Used to keep all information in the added order.

  typedef std::tuple<short, short, short> DetEleLvl_t;
  typedef std::unordered_map<RocBoardChan_t, DetEleLvl_t, RocBoardChanHash> Map_t;
  Map_t m_map; ///< Used in Find() for better speed.

 public:
  ChanMapV1495();
  virtual ~ChanMapV1495() {;}

  void Add (const short roc, const int board, const short chan, const std::string det, const short ele, const short lvl);
  void Add (const short roc, const int board, const short chan, const std::string det_name, const short det_id, const short ele, const short lvl);

  //bool Find(const short roc, const int board, const short chan,  std::string& det, short& ele, short& lvl);
  bool Find(const short roc, const int board, const short chan,        short& det, short& ele, short& lvl);
  void Print(std::ostream& os);

 protected:
  int  ReadFileCont(LineList& lines);
  int WriteFileCont(std::ostream& os);

  void  ReadDbTable(DbSvc& db);
  void WriteDbTable(DbSvc& db);
};

#endif // __CHAN_MAP_V1495_H__
