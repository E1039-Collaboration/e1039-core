#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <UtilAna/UtilOnline.h>
#include "evio.h"
#include "CodaInputManager.h"
using namespace std;

CodaInputManager::CodaInputManager() : 
  m_verb(0), m_online(true), m_go_end(false), m_handle(-1), m_run(0)
{
  ;
}

int CodaInputManager::OpenFile(const std::string fname, const long file_size_min, const int sec_wait, const int n_wait)
{
  if (! file_exists(fname)) {
    cerr << "!!ERROR!!  Coda file does not exist: " << fname << "." << endl;
    return 1;
  }
  m_fname = fname;
  
  // evOpen will return an error if the file is less than
  //	a certain size, so wait until the file is big enough.
  bool size_ok = false;
  for (int i_wait = 0; i_wait < n_wait + 1; i_wait++) {
    FILE* fp = fopen (fname.c_str(), "r");
    if (fp == NULL) {
      cout << "Failed at fopen() with errno=" << errno << "." << endl;
    } else {
      if (fseek(fp, 0L, SEEK_END) != 0) {
        cout << "Failed at fseek() with errno=" << errno << "." << endl;
      }
      m_file_size = ftell(fp);
      fclose(fp);
      if (m_file_size >= file_size_min) {
        size_ok = true;
        break;
      } 
      if (m_verb) {
        cout << "File size: " << m_file_size << " < " << file_size_min 
             << ".  Wait for " << sec_wait << " s (" << i_wait << ")." << endl;
      }
    }
    sleep (sec_wait);
  }
  if (! size_ok) {
    cout << "File size not enough (" << m_file_size << " < " << file_size_min << ").  Wait timeout.  Exiting..." << endl;
    return 2;
  }
  
  if (m_verb) {
    cout << "Loading " << fname << "..." << endl;
  }
  CloseFile();
  int ret = evOpen((char*)fname.c_str(), (char*)"r", &m_handle);
  if (ret != 0) {
    cout << "Failed at opening the Coda file.  ret = " << ret << ".  Exiting..." << endl;
    return 3;
  }
  m_event_count = 0;
  return 0;
}

int CodaInputManager::CloseFile()
{
  if (m_handle < 0) return 0; // Do nothing since no file is opened.
  int ret = evClose(m_handle);
  m_handle = -1;
  return ret;
}

bool CodaInputManager::JumpCodaEvent(unsigned int& event_count, int*& event_words, const unsigned int n_evt)
{
  for (unsigned int i_evt = 0; i_evt < n_evt; i_evt++) {
    if (! NextCodaEvent(event_count, event_words)) {
      cout << "CodaInputManager::SkipCodaEvent():  Failed at i=" << i_evt << " (n=" << n_evt << ")." << endl;
      return false;
    }
  }
  return true;
}

bool CodaInputManager::NextCodaEvent(unsigned int& event_count, int*& event_words)
{
  if (m_go_end) return false;
  int ret = evRead(m_handle, m_event_words, buflen);
  if (ret == 0) {
    event_count = m_event_count++;
    event_words = m_event_words;
    return true;
  }

  if (m_online && m_run > 0) {
    cout << "No new event seems available for now.  Try to recover." << endl;
    if (file_exists(UtilOnline::GetEndFilePath(m_run))) {
      cout << "Exiting since the END file exists." << endl;
      ForceEnd();
      return false;
    }

    if (file_exists(UtilOnline::GetCodaFilePath(m_run+1))) {
      cout << "Exiting since the next run file exists." << endl;
      ForceEnd();
      return false;
    }
    // Re-open the file, requring a larger file size
    unsigned int event_count_jump = m_event_count + 1;
    ret = OpenFile(m_fname, m_file_size + 32768, 10, 20); // m_event_count is reset
    if (ret == 0) {
      cout << "Jumping over " << event_count_jump << " events." << endl;
      return JumpCodaEvent(event_count, event_words, event_count_jump);
    } else {
      cout << "OpenFile() returned " << ret << "." << endl;
    }
  }
  cout << "CodaInputManager::NextCodaEvent():  Bad end." << endl;
  ForceEnd();
  return false;
}

bool CodaInputManager::file_exists(const std::string fname)
{
  FILE *fp = fopen(fname.c_str(), "r");
  if (fp) {
    fclose (fp);
    return true;
  }
  return false;
}

/** This function takes an integer, grabs a certain number of hexadecimal digits from a certain position in the hex representation of the number.
 *
 * For example, if number = 0x10e59c (or, 0d107356)
 *		then get_hex_bit(number, 3) would return e (or 0d14),
 *		representing 0x10e59c <-- those parts of the number
 */
int get_hex_bit (unsigned int hexNum, int numBitFromRight)
{
    int shift;
    unsigned int hexBit;
    // Shift the number to get rid of the bits on the right that we want
    shift = numBitFromRight;
    hexBit = hexNum;
    hexBit = hexBit >> (4 * shift);
    // Do the bitwise AND operation
    hexBit = hexBit & 0xF;
    return hexBit;
}

/** This function takes an integer, grabs a certain number of hexadecimal digits from a certain position in the hex representation of the number.
 *
 * For example, if number = 0x10e59c (or, 0d107356)
 *		then get_bin_bits(number, 3, 3) would return e59 (or 0d3673),
 *		representing 0x10e59c <-- those parts of the number
 * 				 ^^^
 */
int get_hex_bits (unsigned int hexNum, int numBitFromRight, int numBits)
{
    unsigned int hexBits = 0x0;
    int shift;
    unsigned int bitwiseand = 0xF;
    unsigned int eff = 0xF;
    int i;
    // Bitwise method.  Shift the bits, and use bitwise AND to get the bits we want
    // Shift the number to get rid of the bits on the right that we want
    shift = numBitFromRight - numBits + 1;
    hexBits = hexNum;
    hexBits = hexBits >> (4 * shift);

    // Assemble the number that we will use with the above number 
    //   in the bitwise AND operation
    //   so, if we want get_hex_bits(hexNum, 3, 2), it will make 0xFF
    for (i = 1; i < numBits; i++)
    {
        bitwiseand += (eff << (4 * i) );
    }

    // Do the bitwise AND operation
    hexBits = hexBits & bitwiseand;
    return hexBits;
}

/** This function takes an integer, grabs a certain binary digit from a certain position in the binary number.
 *
 * For example, if number = 11010001101011100 (or, 0d107356)
 *		then get_bin_bit(number, 3) would return 1 (or 0d01),
 *		representing 11010001101011100 <-- this part of the number
 *					  ^
 */
int get_bin_bit (unsigned int binNum, int numBitFromRight)
{
    while (numBitFromRight--)
    {
        binNum /= 2;
    }

    return (binNum % 2);
}

/** This function takes an integer, grabs a certain number of binary digits
 *	from a certain position in the binary number.
 *
 * For example, if number = 11010001101011100,
 *		then get_bin_bits(number, 3, 3) would return 110 (or 0d06),
 *		representing 11010001101011100 <-- those parts of the number
 *			                  ^^^
 */
int get_bin_bits (unsigned int binNum, int numBitFromRight, int numBits)
{
    int binBit = 0;
    int binBits = 0;
    int n = 0;
    double d = 1.0;

    for (n = 0; n < (numBits - 1); n++)
    {
        d *= 2.0;
    }

    for (n = 0; n < (numBits) && n <= numBitFromRight; n++)
    {
        binBit = get_bin_bit (binNum, numBitFromRight - n);
        binBits += binBit * d;
        d /= 2;
    }

    return binBits;
}

void Abort(const char* message)
{
  cerr << "!!ERROR!!  " << message << endl;
  exit(1);
}

void PrintWords(int* words, int idx_begin, int idx_end, int idx_atte)
{
  cout << "  PrintWords[" << idx_begin << "-" << idx_end << "]:\n";
  int idx_b5 = (idx_begin / 5) * 5;
  for (int idx = idx_b5; idx < idx_end; idx++) {
    if (idx % 5 == 0) cout << "\n  " << setw(6) << idx << ": ";
    cout << " " << hex << setw(8) << words[idx] << dec;
    if (idx == idx_atte) cout << "!";
  }
  cout << endl;
}

/** Print out lists of ROCs and boards found in one Coda event.
 *
 * This function might be useful when you check the word format
 * independent of all main functions (like ProcessPhysFlush() & 
 * format()).  But you should not assume that the output of this 
 * function is always consistent with those of the main functions, 
 * since this function decodes words by itself.
 */
void PrintCodaEventSummary(int* words)
{
  int n_wd = words[0];
  int code = words[1];
  cout << "  Event code = " << hex << code
       //<< " 0x" << get_hex_bits(code, 3, 4)
       //<< " 0x" << get_hex_bits(code, 7, 4)
       << dec << " (" << n_wd << ")" << endl;
  int idx = 7; // the start of ROC data
  while (idx < n_wd) {
    int n_wd_roc = words[idx  ];
    int roc_id   = get_hex_bits(words[idx+1], 5, 2);
    cout << "  ROC " << setw(2) << roc_id << " (" << n_wd_roc << ") |";
    int idx_roc_end = idx + n_wd_roc + 1;
    idx += 5; // the 1st word of data of boards
    while (idx < idx_roc_end) {
      int e906flag = words[idx];
      if (e906flag == (int)0xe906c0da) {
	cout << " c0da";
	idx++;
	break;
      }
      int flag_hi = get_hex_bits(e906flag, 7, 4);
      int flag_lo = get_hex_bits(e906flag, 3, 4);
      bool is_flag = (flag_hi == (int)0xe906);

      int board_id = get_hex_bits(words[idx + 1], 7, 2);
      int n_wd_bd  = get_hex_bits(words[idx + 1], 3, 4);
      bool has_dummy = (words[idx+2] == (int)0xe906e906);
      idx += n_wd_bd + (has_dummy  ?  3  :  2); // index of _next_ board
      bool overflow_n_wd = (idx > idx_roc_end);
      cout << hex << " " << (is_flag ? "" : "?") << flag_lo << ":" << board_id 
	   << dec << "(" << n_wd_bd << ")";
      if (overflow_n_wd) cout << "OF";
    }
    if      (idx < idx_roc_end) cout << " | UF";
    else if (idx > idx_roc_end) cout << " | OF";
    idx = idx_roc_end;
    cout << endl;
  }
}
