#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include "evio.h"
#include "ManageCodaInput.h"
using namespace std;

ManageCodaInput::ManageCodaInput() : 
  m_verb(0), m_online(true), m_go_end(false), m_handle(0), m_run(0)
{
  ;
}

int ManageCodaInput::OpenFile(const std::string fname, const int file_size_min, const int sec_wait, const int n_wait, const int n_evt_pre_read)
{
  if (! file_exists(fname)) {
    cerr << "!!ERROR!!  Coda file does not exist: " << fname << ".\n"
	 << "Exiting...\n";
    return 1;
  }
  m_fname = fname;
  
  // evOpen will return an error if the file is less than
  //	a certain size, so wait until the file is big enough.
  bool size_ok = false;
  for (int i_wait = 0; i_wait < n_wait + 1; i_wait++) {
    FILE* fp = fopen (fname.c_str(), "r");
    fseek(fp, 0L, SEEK_END);
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
    sleep (sec_wait);
  }
  if (! size_ok) {
    cout << "File size not enough.  Wait timeout.  Exiting..." << endl;
    return 2;
  }
  
  if (m_verb) {
    cout << "Loading " << fname << "...\n";
  }
  int ret = evOpen((char*)fname.c_str(), (char*)"r", &m_handle);
  if (ret != 0) {
    cout << "Failed at opening the Coda file.  ret = " << ret << ".  Exiting..." << endl;
    return 3;
  }
  m_event_count = 0;
  for (int ii = 0; ii < n_evt_pre_read; ii++) { // todo: this loop number is exactly correct??
    unsigned int coda_id; // dummy
    int* words = 0; // dummy
    NextCodaEvent(coda_id, words); // todo: check if it fails
  }
  return 0;
}

int ManageCodaInput::CloseFile()
{
  return evClose(m_handle);
}

bool ManageCodaInput::NextCodaEvent(unsigned int& coda_id, int*& words)
{
  if (m_go_end) return false;
  int ret = evRead(m_handle, event_words, buflen);
  if (ret != 0 && m_online && m_run > 0) { // try to recover
    cout << "Try to recover..." << endl;
    ostringstream oss;
    oss << "/data2/e906daq/coda/data/END/" << m_run << ".end";
    string fn_end = oss.str();
    if (file_exists(fn_end)) {
      cout << "Exiting since the END file exists.\n";
      return false;
    }
    oss.str("");
    oss << setfill('0') << "/codadata/run_" << setw(6) << (m_run+1) << ".dat";
    string fn_next_run = oss.str();
    if (file_exists(fn_next_run)) {
      cout << "Exiting since the next run file exists.\n";
      return false;
    }
    // Re-open the file, requring a larger file size
    ret = OpenFile(m_fname, m_file_size + 32768, 5, 2, m_event_count);
  }
  if (ret != 0) return false;
  coda_id = m_event_count++;
  words   = event_words;
  return true;
}

bool ManageCodaInput::file_exists(const std::string fname)
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
  cout << "PrintWords[" << idx_begin << "-" << idx_end << "] " << hex;
  for (int idx = idx_begin; idx < idx_end; idx++) {
    cout << " " << words[idx];
    if (idx == idx_atte) cout << "!";
  }
  cout << dec << endl;
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
