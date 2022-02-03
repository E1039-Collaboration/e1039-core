#ifndef __CODA_INPUT_MANAGER_H__
#define __CODA_INPUT_MANAGER_H__
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <string>

class CodaInputManager {
  static const int buflen = 500000;
  int  m_verb;
  bool m_online; //< True if the decoder runs in the online mode.
  bool m_go_end;
  int  m_handle; //< Handler for evio
  int  m_run;
  std::string m_fname;
  long int m_file_size;
  int m_event_count;
  int m_event_words[buflen];

 public:
  CodaInputManager();
  virtual ~CodaInputManager() {;}

  void SetVerbosity(const int verb) { m_verb = verb; }
  void SetRunNumber(const int run ) { m_run  = run ; }
  void SetOnline   (const bool val) { m_online = val; }
  void ForceEnd () { m_go_end = true; }
  bool IsEnded  () { return m_go_end; }

  int OpenFile(const std::string fname, const long file_size_min=0, const int sec_wait=10, const int n_wait=0);
  int CloseFile();
  bool JumpCodaEvent(unsigned int& event_count, int*& event_words, const unsigned int n_evt);
  bool NextCodaEvent(unsigned int& event_count, int*& event_words);

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

//
// Helper function
//   These are not in CodaInputManager for easier call.
int get_hex_bits (unsigned int hexNum, int numBitFromRight, int numBits);
int get_hex_bit (unsigned int hexNum, int numBitFromRight);
int get_bin_bits (unsigned int binNum, int numBitFromRight, int numBits);
int get_bin_bit (unsigned int binNum, int numBitFromRight);

void Abort(const char* message);
void PrintWords(int* words, int idx_begin, int idx_end, int idx_atte=-1);
void PrintCodaEventSummary(int* words);

#endif // __CODA_INPUT_MANAGER_H__
