#ifndef __MANAGE_CODA_INPUT_H__
#define __MANAGE_CODA_INPUT_H__
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <string>

class ManageCodaInput {
  static const int buflen = 500000;
  int  m_verb;
  bool m_online; //< Always 'true' for now
  bool m_go_end;
  int  m_handle; //< Handler for evio
  int  m_run;
  std::string m_fname;
  long int m_file_size;
  int m_event_count;
  int event_words[buflen];

 public:
  ManageCodaInput();
  virtual ~ManageCodaInput() {;}

  void SetVerbosity(const int verb) { m_verb = verb; }
  void SetRunNumber(const int run ) { m_run  = run ; }
  void SetOnline() { m_online = true; }
  void ForceEnd () { m_go_end = true; }
  bool IsEnded  () { return m_go_end; }

  int OpenFile(const std::string fname, const int file_size_min=0, const int sec_wait=10, const int n_wait=0, const int n_evt_pre_read=0);
  int CloseFile();
  bool NextEvent(unsigned int& coda_id, int*& event_words);

 private:
  bool file_exists(const std::string fname);
};

//
// Event Type Codes
//
enum { PHYSICS_EVENT  =     0x10cc };
enum { CODA_EVENT     =     0x01cc };
enum { FEE_PREFIX     =     0x0100 };
enum { PRESTART_EVENT = 0x001101cc };
enum { GO_EVENT       = 0x001201cc };
enum { END_EVENT      = 0x001401cc };
enum { FEE_EVENT      = 0x00840100 };

enum { STANDARD_PHYSICS =  14 }; //
enum { FLUSH_EVENTS     =  10 }; // previous IGNORE_ME type
enum { SLOW_CONTROL     = 130 };
enum { RUN_DESCRIPTOR   = 140 };
enum { PRESTART_INFO    = 150 };
enum { BEGIN_SPILL      =  11 };
enum { END_SPILL        =  12 };
enum { SPILL_COUNTER    = 129 };

typedef enum {
  TYPE_BOS = 1,
  TYPE_EOS = 2
} SpillType_t;

//
// Helper function
//   These are not in ManageCodaInput for easier call.
int get_hex_bits (unsigned int hexNum, int numBitFromRight, int numBits);
int get_hex_bit (unsigned int hexNum, int numBitFromRight);
int get_bin_bits (unsigned int binNum, int numBitFromRight, int numBits);
int get_bin_bit (unsigned int binNum, int numBitFromRight);

void Abort(const char* message);
void PrintWords(int* words, int idx_begin, int idx_end, int idx_atte=-1);
void PrintCodaEventSummary(int* words);

#endif // __MANAGE_CODA_INPUT_H__