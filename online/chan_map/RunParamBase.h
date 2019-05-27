#ifndef __RUN_PARAM_BASE_H__
#define __RUN_PARAM_BASE_H__
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include "ParamRunRange.h"
class DbSvc;

class RunParamBase {
  std::string m_dir_base;
  std::string m_type;
  std::string m_label;
  std::string m_header;
  std::string m_map_id;
  std::string m_db_conf;
  ParamRunRange m_range;

 public:
  RunParamBase(const std::string type, const std::string label, const std::string header);
  virtual ~RunParamBase() {;}

  void SetBaseDir(const std::string dir_base) { m_dir_base = dir_base; }
  std::string GetBaseDir() { return m_dir_base; }

  std::string GetMapID() { return m_map_id; }
  void        SetMapID(const std::string map_id) { m_map_id = map_id; }
  std::string GetDBConf() { return m_db_conf; }
  void        SetDBConf(const std::string db_conf) { m_db_conf = db_conf; }
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

class ChanMapBase : public RunParamBase {
 protected:
  typedef std::tuple<short, short, short> RocBoardChan_t;
 public:
  ChanMapBase(const std::string label, const std::string header) : RunParamBase("chan_map", label, header) {;}
  virtual ~ChanMapBase() {;}
};

class CalibParamBase : public RunParamBase {
 public:
  CalibParamBase(const std::string label, const std::string header) : RunParamBase("calib", label, header) {;}
  virtual ~CalibParamBase() {;}
};

#endif // __RUN_PARAM_BASE_H__
