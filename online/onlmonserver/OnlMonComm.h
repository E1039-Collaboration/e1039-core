#ifndef _ONL_MON_COMM__H__
#define _ONL_MON_COMM__H__
#include <map>
#include <string>
//#include <pthread.h>
class TSocket;
class OnlMonClient;

class OnlMonComm {
 public:
  typedef enum { SP_ALL, SP_LAST, SP_RANGE } SpillMode_t;

 private:
  static OnlMonComm* m_inst;

  SpillMode_t m_sp_mode;
  int m_sp_num; 
  int m_sp_lo;
  int m_sp_hi;

  typedef std::vector<int> SpillList_t;
  SpillList_t m_list_sp;

 public:
  static OnlMonComm *instance();
  virtual ~OnlMonComm();

  void SetSpillMode(const SpillMode_t mode) { m_sp_mode = mode; }
  SpillMode_t GetSpillMode() { return m_sp_mode; } 

  void SetSpillNum(const int num) { m_sp_num = num; }
  int  GetSpillNum()       { return m_sp_num; }

  void SetSpillRangeLow (const int sp);
  void SetSpillRangeHigh(const int sp);
  void SetSpillRange(const int sp_lo, const int sp_hi);
  void GetSpillRange(int& sp_lo, int& sp_hi);

  void AddSpill(const int id);
  void GetFullSpillRange(int& id_min, int& id_max);

  int ReceiveSpillRange(int& sp_min, int& sp_max);
  TSocket* ConnectServer();

 protected:
  OnlMonComm();
};

#endif // _ONL_MON_COMM__H__
