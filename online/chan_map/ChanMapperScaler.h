#ifndef __CHAN_MAPPER_SCALER_H__
#define __CHAN_MAPPER_SCALER_H__
#include "ChanMapper.h"

class ChanMapperScaler : public ChanMapBase {
  struct MapItem {
    short roc;
    short board;
    short chan;
    std::string name;
  };
  typedef std::vector<MapItem> List_t;
  List_t m_list; ///< Used to keep all information in the added order.

  typedef std::map<RocBoardChan_t, std::string> Map_t;
  Map_t m_map; ///< Used in Find() for better speed.

 public:
  ChanMapperScaler();
  virtual ~ChanMapperScaler() {;}

  void Add (const short roc, const short board, const short chan, const std::string name);
  bool Find(const short roc, const short board, const short chan,  std::string& name);
  void Print(std::ostream& os);

 protected:
  int  ReadFileCont(LineList& lines);
  int WriteFileCont(std::ostream& os);

  void  ReadDbTable(DbSvc& db);
  void WriteDbTable(DbSvc& db);
};

#endif // __CHAN_MAPPER_SCALER_H__
