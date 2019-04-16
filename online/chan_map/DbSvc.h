#ifndef __DB_SVC_H__
#define __DB_SVC_H__
#include <string>
#include <sstream>
#include <TMySQLServer.h>
#include <TSQLStatement.h>

class DbSvc {
 public:
  typedef enum { DB1, DB2, DB3, DB01, UIUC, UNDEF } SvrId_t;

  DbSvc(const SvrId_t svr_id=DB1);
  ~DbSvc();

  void UseSchema(const char*       name, const bool do_create=false, const bool do_drop=false);
  void UseSchema(const std::string name, const bool do_create=false, const bool do_drop=false) { UseSchema(name.c_str(), do_create, do_drop); }
  //void UseSchema(const char* schema, const bool do_create=true);
  //void UseSchema(const std::string schema, const bool do_create=true) { UseSchema(schema.c_str(), do_create);}

  void DropTable  (const char* name);
  void DropTable  (const std::string name) { DropTable(name.c_str()); }
  bool AssureTable(const char* name, const bool exit_on_error=true);
  bool AssureTable(const std::string name, const bool exit_on_error=true) { AssureTable(name.c_str(), exit_on_error); }
  
  TSQLServer* Con() { return m_con; }
  TSQLStatement* Process(const char*             query);
  TSQLStatement* Process(const std::string       query) { return Process(query.c_str()); }
  
 private:
  SvrId_t     m_svr_id;
  std::string m_svr;
  TSQLServer* m_con;

  void SelectServer();
  void ConnectServer();
};
#endif // __DB_SVC_H__
