#ifndef __CHAN_MAP_TAIWAN_H__
#define __CHAN_MAP_TAIWAN_H__
#include <unordered_map>
#include "RunParamBase.h"

class ChanMapTaiwan : public ChanMapBase {
  struct MapItem {
    short roc;
    short board;
    short chan;
    std::string det_name;
    short det;
    short ele;
  };
  typedef std::vector<MapItem> List_t;
  List_t m_list; ///< Used to keep all information in the added order.

  typedef std::pair<short, short> DetEle_t;
  typedef std::unordered_map<RocBoardChan_t, DetEle_t, RocBoardChanHash> Map_t;
  Map_t m_map; ///< Used in Find() for better speed.

 public:
  ChanMapTaiwan();
  virtual ~ChanMapTaiwan() {;}

  void Add(const short roc, const short board, const short chan, const std::string det, const short ele);
  void Add(const short roc, const short board, const short chan, const std::string det_name, const short det_id, const short ele);

  //bool Find(const short roc, const short board, const short chan,  std::string& det, short& ele);
  bool Find(const short roc, const short board, const short chan,        short& det, short& ele);
  void Print(std::ostream& os);

 protected:
  int  ReadFileCont(LineList& lines);
  int WriteFileCont(std::ostream& os);

  void  ReadDbTable(DbSvc& db);
  void WriteDbTable(DbSvc& db);
};

#endif // __CHAN_MAP_TAIWAN_H__
