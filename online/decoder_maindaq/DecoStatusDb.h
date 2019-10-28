#ifndef _DECO_STATUS_DB__H_
#define _DECO_STATUS_DB__H_
class DbSvc;

class DecoStatusDb {
  typedef enum {
    UNKNOWN  = 0,
    STARTED  = 1,
    FINISHED = 2
  } Status_t;

  std::string m_name_table;
  DbSvc* m_db;

  //typedef std::map<std::string, int> StatusMap_t;
  //StatusMap_t m_stat_map;

 public:
  DecoStatusDb();
  virtual ~DecoStatusDb() {}

  void InitTable(const bool refresh=false);
  void RunStarted (const int run, int utime=0);
  void RunFinished(const int run, const int result, int utime=0);
};

#endif /* _DECO_STATUS_DB__H_ */
