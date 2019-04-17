#ifndef __CHAN_MAPPER_H__
#define __CHAN_MAPPER_H__
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include "ChanMapperRange.h"
class DbSvc;

class ChanMapper {
 protected:
  typedef std::tuple<short, short, short> RocBoardChan_t;

  std::string m_dir_base;
  std::string m_label;
  std::string m_map_id;
  std::string m_header;
  ChanMapperRange m_range;

 public:
  ChanMapper();
  virtual ~ChanMapper();

  std::string GetMapID() { return m_map_id; }
  void        SetMapID(const std::string map_id) { m_map_id = map_id; }
  void SetMapIDbyFile(const std::string map_id);
  void SetMapIDbyDB  (const std::string map_id);
  void SetMapIDbyFile(const int run);
  void SetMapIDbyDB  (const int run);

  void ReadFromFile();
  void  WriteToFile();
  void ReadFromLocalFile(const std::string fn_tsv);
  void  WriteToLocalFile(const std::string fn_tsv);

  void ReadFromDB();
  void WriteToDB ();
  void WriteRangeToDB();

  virtual void Print(std::ostream& os);

 protected:
  std::string RangeFileName();
  std::string MapFileName();
  std::string SchemaName();
  std::string MapTableName();

  typedef std::vector<std::string> LineList;
  virtual int  ReadFileCont(LineList& lines);
  virtual int WriteFileCont(std::ostream& os);

  virtual void  ReadDbTable(DbSvc& db);
  virtual void WriteDbTable(DbSvc& db);
};

#endif // __CHAN_MAPPER_H__
