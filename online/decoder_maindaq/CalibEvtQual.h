#ifndef _CALIB_EVT_QUAL_H_
#define _CALIB_EVT_QUAL_H_
#include <fun4all/SubsysReco.h>
class SQEvent;
class SQHardEvent;

class CalibEvtQual: public SubsysReco {
  /** Error flags to be inserted to the "Event.dataQuality" field.
   * We should add more types.
   * Ref: https://cdcvs.fnal.gov/redmine/projects/seaquest-production/wiki/Data_Quality_in_our_Productions
   */
  typedef enum { 
    ERR_N_TDC     = 0x001, // No or extra TDC-board info
    ERR_V1495     = 0x002, // v1495 Readout Problem, i.e. 0xD1AD, 0xD2AD 0xD3AD issues.
    ERR_N_V1495_0 = 0x004, // No    v1495 info
    ERR_N_V1495_2 = 0x008, // Extra v1495 info
    ERR_N_TRIGB_0 = 0x010, // No    trigger-bit info
    ERR_N_TRIGB_2 = 0x020, // Extra trigger-bit info
    ERR_N_TRIGC_0 = 0x040, // No    trigger-count info
    ERR_N_TRIGC_2 = 0x080, // Extra trigger-count info
    ERR_N_QIE_0   = 0x100, // No    QIE info
    ERR_N_QIE_2   = 0x200  // Extra QIE info
    //EVT_ERR_ID    = 0x20  // Event-ID mismatch between ROCs.
    // 0x00000800...0x20000000 ... Taiwan-TDC readout error on bit_number=roc_ID where roc_ID=12...30.
  } EventErrorFlag_t;

 public:
  CalibEvtQual(const std::string &name = "CalibEvtQual");
  virtual ~CalibEvtQual() {}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);
 private:
  void PrintEvent(SQEvent* evt, SQHardEvent* hevt);
};

#endif /* _CALIB_EVT_QUAL_H_ */
