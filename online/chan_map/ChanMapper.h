#ifndef __CHAN_MAPPER_H__
#define __CHAN_MAPPER_H__
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <string>
class DbSvc;

class ChanMapper {
 protected:
  std::string m_dir_base;
  std::string m_label;
  std::string m_map_id;
  std::string m_header;

 public:
  ChanMapper();
  virtual ~ChanMapper() {;}

  std::string GetMapID() { return m_map_id; }
  void        SetMapID(const std::string map_id) { m_map_id = map_id; }
  std::string RangeFileName();
  std::string SchemaName();
  std::string TableName();

  void ReadFromFile(const int run);
  void ReadFromDB  (const int run);

  virtual void ReadFromFile(const std::string fn_tsv);
  virtual void WriteToFile (const std::string fn_tsv);

  void ReadFromDB();
  void WriteToDB ();

  virtual void Print(std::ostream& os);

 protected:
  virtual void  ReadDbTable(DbSvc& db);
  virtual void WriteDbTable(DbSvc& db);
};

#endif // __CHAN_MAPPER_H__
