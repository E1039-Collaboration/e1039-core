#ifndef _ONL_MON_COMM__H__
#define _ONL_MON_COMM__H__
#include <vector>
class TSocket;

class OnlMonComm {
 public:
  typedef enum { SP_ALL, SP_LAST, SP_RANGE } SpillMode_t;

 private:
  static OnlMonComm* m_inst;

  SpillMode_t m_sp_mode;
  int m_sp_num; 
  int m_sp_lo; //< Low  of selected range
  int m_sp_hi; //< High of selected range
  int m_sp_min; //< Min of full (not selected) range
  int m_sp_max; //< Max of full scale
  bool m_sp_sel; //< True if spills are selectable
  int m_n_sp_sel_max; //< Max number of spills for which spills are kept selectable.

  typedef std::vector<int> SpillList_t;
  SpillList_t m_list_sp;

 public:
  static OnlMonComm *instance();
  virtual ~OnlMonComm();

  void SetSpillMode(const SpillMode_t mode) { m_sp_mode = mode; }
  SpillMode_t GetSpillMode() { return m_sp_mode; } 

  void SetSpillSelectability(const bool val) { m_sp_sel = val; }
  bool GetSpillSelectability()        { return m_sp_sel; }

  void SetSpillNum(const int num) { m_sp_num = num; }
  int  GetSpillNum()       { return m_sp_num; }

  void SetSpillRangeLow (const int sp); //< Set the low  edge of selected spills
  void SetSpillRangeHigh(const int sp); //< Set the high edge of selected spills
  void SetSpillRange(const int sp_lo, const int sp_hi); //< Set the range of selected spills
  void GetSpillRange(int& sp_lo, int& sp_hi); //< Get the range of selected spills

  void SetMaxNumSelSpills(const int val) { m_n_sp_sel_max = val; }
  int  GetMaxNumSelSpills()       { return m_n_sp_sel_max; }

  int  GetNumSpills() { return (int)m_list_sp.size(); } //< Used in the server process
  void ClearSpill() { m_list_sp.clear(); } //< Used in the server process
  void AddSpill(const int id); //< Used in the server process
  void FindFullSpillRange(int& id_min, int& id_max); //< Used in the server process

  int ReceiveFullSpillRange(); //< Used in the viewer process
  void GetFullSpillRange(int& id_min, int& id_max); //< Used in the viewer process

  TSocket* ConnectServer();

 protected:
  OnlMonComm();
};

#endif // _ONL_MON_COMM__H__
