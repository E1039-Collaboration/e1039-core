#ifndef __DECO_DATA_H__
#define __DECO_DATA_H__
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <TObject.h>

/* ================================================================
 DecoData
  |
  +- RunData
  |   +- FeeData
  |   +- Other run variables
  |
  |
  +- map<spill ID, SpillData> = SpillDataMap
  |                 +- BOS & EOS spill variables
  |                 +- vector<SlowControlData>
  |                 +- vector<ScalerData>
  |
  |
  +- map<event ID, EventData> = EventDataMap
  |                 +- EventInfo
  |                 |   +- Trigger & QIE variables
  |                 +- vector<HitData> for Taiwan-TDC hits
  |                 +- vector<HitData> for v1495-TDC hits
  |     Note:  The contents of HitData are useful for now 
  |            since SRawEvent cannot hold roc/board/tdc IDs.
  O

 Timeline of Coda events
  | 
  +- BOS
  | 
  +- EOS
  |                <- Flush events
  +- Spill Counter <- Flush events
  |                <- Flush events
  +- Slow Control  <- Flush events
  |                <- Flush events
  + (Next BOS)

================================================================ */

////////////////////////////////////////////////////////////////
//
// Run Data
//

struct FeeData : public TObject {
  unsigned int roc;
  unsigned int board;
  unsigned int hard; // TDCHardID
  unsigned int falling_enabled;
  unsigned int segmentation;
  unsigned int multihit_elim_enabled;
  unsigned int updating_enabled;
  unsigned int elim_window;
  unsigned int lowLimit;
  unsigned int highLimit;
  unsigned int selectWindow;

  FeeData();
  virtual ~FeeData() {;}
};
typedef std::vector<FeeData> FeeDataList;

struct RunData : public TObject {
  int run_id;
  int utime_b;
  int utime_e; // not implemented

  int fpga_enabled [5];
  int  nim_enabled [5];
  int fpga_prescale[5];
  int  nim_prescale[3];

  int trig_bit[10];
  int prescale[ 8];
  FeeDataList fee;
  std::string run_desc;
  
  int n_fee_event;
  int n_fee_prescale;
  int n_run_desc;
  int n_spill;
  int n_evt_all; //< N of all real (i.e. triggered) events
  int n_evt_dec; //< N of decoded real events
  int n_phys_evt; //< N of Coda physics events
  int n_phys_evt_bad;
  int n_flush_evt; //< N of Coda flush events
  int n_flush_evt_bad;
  int n_hit; //< N of Taiwan-TDC hits
  int n_t_hit; //< N of v1495-TDC hits
  int n_hit_bad; //< N of bad hits
  int n_t_hit_bad; //< N of bad t-hits.  Not implemented
  int n_v1495; //< N of v1495 events
  int n_v1495_d1ad;
  int n_v1495_d2ad;
  int n_v1495_d3ad;

  RunData();
  virtual ~RunData() {;}
};

////////////////////////////////////////////////////////////////
//
// Spill Data
//

struct SlowControlData {
  std::string ts;
  std::string name;
  std::string value;
  std::string type;

  SlowControlData();
  virtual ~SlowControlData() {;}
};
typedef std::vector<SlowControlData> SlowControlDataList;

struct ScalerData {
  short type ;
  int   coda ;
  short roc  ;
  short board;
  short chan ;
  int   value;
  std::string name;

  ScalerData();
  virtual ~ScalerData() {;}
};
typedef std::vector<ScalerData> ScalerDataList;

struct SpillData {
  unsigned int spill_id; //< spill ID in Spill Counter
  unsigned int spill_id_slow; //< spill ID in slow control
  unsigned int run_id;
  unsigned int targ_pos;
  unsigned int bos_coda_id;
  unsigned int bos_vme_time;
  unsigned int eos_coda_id;
  unsigned int eos_vme_time;

  unsigned int n_bos_spill;
  unsigned int n_eos_spill;
  unsigned int n_slow;
  unsigned int n_scaler;

  double time_input;  ///< In msec.
  double time_decode; ///< In msec.
  double time_map;    ///< In msec.

  SlowControlDataList list_slow_cont;
  ScalerDataList list_scaler;

  SpillData();
  virtual ~SpillData() {;}
};
typedef std::map<unsigned int, SpillData> SpillDataMap;

////////////////////////////////////////////////////////////////
//
// Event-by-event data
//
struct HitData {
  int   event;
  int   id;
  short roc;
  int   board;
  short chan;
  short det;
  short ele;
  short lvl;
  double         time;
  HitData();
  virtual ~HitData() {;}
};
typedef std::vector<HitData> HitDataList;

struct EventInfo {
  int eventID;
  int codaEventID;
  int runID;
  int spillID;
  int dataQuality;
  int vmeTime;
  int RawMATRIX[5];
  int AfterInhMATRIX[5];
  int NIM[5];
  int MATRIX[5];
  int trigger_bits;
  unsigned int sums[4];
  unsigned int triggerCount;
  unsigned int turnOnset;
  unsigned int rfOnset;
  unsigned int rf[33];
  short flag_v1495;

  EventInfo();
  ~EventInfo() {;}
};

struct EventData {
  unsigned int n_qie;
  unsigned int n_v1495;
  unsigned int n_tdc;
  unsigned int n_trig_b;
  unsigned int n_trig_c;

  EventInfo event;
  HitDataList list_hit;
  HitDataList list_hit_trig;

  EventData();
  ~EventData() {;}
};
typedef std::map<unsigned int, EventData> EventDataMap;

#endif // __DECO_DATA_H__
