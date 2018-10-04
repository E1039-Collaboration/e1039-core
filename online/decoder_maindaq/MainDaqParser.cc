#include <iomanip>
#include <sstream>
#include "CodaInputManager.h"
#include "MainDaqParser.h"
using namespace std;

// In the current design, all event-word handlers (i.e. functions) should print out all error messages needed, so that their caller should print nothing but just check the returned value.

MainDaqParser::MainDaqParser()
{
  coda = new CodaInputManager();
  list_sd = new SpillDataMap();
  list_ed = new EventDataMap();
  sd_now  = 0; // Will be just a pointer to one object in "list_sd"
  list_ed_now = new EventDataMap();
}

MainDaqParser::~MainDaqParser()
{
  if (coda       ) delete coda;
  if (list_sd    ) delete list_sd;
  if (list_ed    ) delete list_ed;
  if (list_ed_now) delete list_ed_now;
}

int MainDaqParser::OpenCodaFile(const std::string fname, const int file_size_min, const int sec_wait, const int n_wait)
{
  dec_par.timeStart = time(NULL);
  dec_par.fn_in = fname;
  return coda->OpenFile(fname, file_size_min, sec_wait, n_wait);
}

bool MainDaqParser::NextPhysicsEvent(EventData*& ed, SpillData*& sd, RunData*& rd)
{
  static EventDataMap::iterator it = list_ed_now->begin();
  if (it == list_ed_now->end()) { // No event in the event buffer
    list_ed_now->clear();
    while (list_ed_now->size() == 0 && ! coda->IsEnded()) {
      ParseOneSpill();
    }
    it = list_ed_now->begin();
  }
  if (it != list_ed_now->end()) {
    rd = &run_data;
    sd = sd_now;
    ed = &it->second;
    it++;
    return true;
  } else {
    rd = 0;
    sd = 0;
    ed = 0;
    return false;
  }
}

int MainDaqParser::ParseOneSpill()
{
  dec_par.at_bos = false;
  int* event_words = 0;
  while (coda->NextCodaEvent(dec_par.codaID, event_words)) {
    //cout << "NextCodaEvent(): " << dec_par.codaID << " 0x" << hex <<  event_words[1] << dec << endl;
    int ret = 0;
    int evt_type_id = event_words[1];
    
    // The last 4 hex digits will be 0x01cc for Prestart, Go Event,
    //		and End Event, and 0x10cc for Physics Events
    switch (evt_type_id & 0xFFFF) {
    case PHYSICS_EVENT:
      ret = ProcessCodaPhysics(event_words);
      break;
    case CODA_EVENT:
      switch (evt_type_id) {
      case PRESTART_EVENT:
        ret = ProcessCodaPrestart(event_words);
        break;
      case GO_EVENT: // do nothing
        break;
      case END_EVENT:
        coda->ForceEnd(); //if (dec_par.verbose) printf ("End Event Processed\n");
        break;
      default:
        cerr << "!!ERROR!! Uncovered Coda event type: " << evt_type_id << ".  Exit.\n";
        return false;
      }
      break;
    case FEE_PREFIX:
      ret = ProcessCodaFee(event_words);
      break;
    case 0: // Special case which requires waiting and retrying
      if (dec_par.verbose) cout << "Case '0' @ coda " << dec_par.codaID << "\n";
      ret = coda->OpenFile(dec_par.fn_in, file_size_min, sec_wait, n_wait, dec_par.codaID-1);
      break;
    default: // If no match to any given case, print it and exit.
      cerr << "!!ERROR!!  Uncovered Coda event type: " << evt_type_id << ".  Exit\n";
      return false;
    }
    if (ret != 0) {
      cout << "WARNING:  ParseOneSpill():  ret = " << ret << endl;
      break;
    }
    if (dec_par.at_bos) break;
  }
  return 0;
}

/** A function that is called when one input file is finished reading.
 * We might have to parse any remaining Coda events here as we did in the original decoder via CloseOutput(), although such events are most likely useless since their spill info is imcomplete.
 */
int MainDaqParser::End()
{
  coda->CloseFile();
  dec_par.timeEnd = time(NULL);
  if (dec_par.verbose) dec_par.PrintStat();
  return 0;
}

/** Process one Coda Prestart event.
 * It is called in main() on CODA_EVENT PRESTART_EVENT.
 * Since the run number is given to the decoder only by this Prestart event,
 * some initialization works have to be done here.
 */
int MainDaqParser::ProcessCodaPrestart(int* words)
{
    // int evLength     = words[0];
    // int prestartCode = words[1];
    int runTime = words[2];
    dec_par.runID = run_data.run_id = words[3];
    // int runType = words[4];

    cout << "  ProcessCodaPrestart " << dec_par.runID << " " << runTime << " " << dec_par.runID << " " << runTime << " " << dec_par.runID << " " << dec_par.sampling << "\n";

    dec_par.InitMapper();
    coda->SetRunNumber(dec_par.runID);

    return 0;
}

int MainDaqParser::ProcessCodaFee(int* words)
{
  if (words[1] != FEE_EVENT) {
    cerr << "!!ERROR!!  Not FEE_EVENT in case of FEE_PREFIX: " << words[1] << ".  Exit.\n";
    return 1;
  }
  if (words[0] > 8) { // Process only if there is content
    if (words[3] == (int)0xe906f011) {
      return ProcessCodaFeeBoard(words);
    } else if (words[3] == (int)0xe906f012) {
      if (dec_par.verbose) cout << "feePrescale: codaEventID = " << dec_par.codaID << endl;
      return ProcessCodaFeePrescale(words);
    }
  }
  return 0;
}

int MainDaqParser::ProcessCodaFeeBoard(int* words)
{
  if (dec_par.verbose > 1) cout << "CodaFeeBoard: coda = " << dec_par.codaID << endl;

    FeeData data;
    int size = words[0];
    data.roc = words[2];
    int i_wd = 3;
    while (i_wd < size)
    {
        // words[i_wd] = 0xe906f011 // ignore this flag
        i_wd++;
        // This word contains only the boardID
        data.board = get_hex_bits (words[i_wd], 7, 4);
        data.hard  = get_hex_bits (words[i_wd], 1, 2);
        i_wd++;
        // This is the TDC registry word
        data.falling_enabled = get_hex_bit (words[i_wd], 2);
        data.segmentation    = get_hex_bit (words[i_wd], 1);
        if      ( data.segmentation == 0 ) data.segmentation =  512;
        else if ( data.segmentation == 2 ) data.segmentation = 1024;
        else if ( data.segmentation == 6 ) data.segmentation = 2048;

        i_wd++;
        // Another TDC registry word
        data.multihit_elim_enabled = get_bin_bit (words[i_wd], 23);
        data.updating_enabled      = get_bin_bit (words[i_wd], 22);

	data.elim_window = 0;
        if (data.multihit_elim_enabled) {
            data.elim_window = get_bin_bits (words[i_wd], 21, 6);
            data.elim_window = 4 * (4 + data.elim_window);
        }

        i_wd++;
        data.lowLimit     =  get_hex_bits(words[i_wd], 2, 3);
        data.highLimit    =  get_hex_bits(words[i_wd], 6, 3);
	data.selectWindow = (get_hex_bit (words[i_wd], 7) == 8  ?  1  :  0);
        i_wd++;

	run_data.fee.push_back(data);
	if (dec_par.verbose > 1) {
	  cout << "  feeE " << data.roc << " " << data.board << " " << data.hard 
	       << " " << data.falling_enabled << " " << data.segmentation
	       << " " << data.multihit_elim_enabled << " " << data.updating_enabled
	       << " " << data.elim_window << " " << data.selectWindow << " "
	       << data.lowLimit << " " << data.highLimit << "\n";
	}
    }

  run_data.n_fee_event++;
  return 0;
}

int MainDaqParser::ProcessCodaFeePrescale(int* words)
{
  if (dec_par.verbose > 1) cout << "CodaFeePrescale: coda = " << dec_par.codaID << endl;
  int feeTriggerBits = words[4];
  /// 10 bits = FPGA1, ..., FPGA5, NIM1, ..., NIM5
  for (int ii = 0; ii < 10; ii++) {
    run_data.trig_bit[ii] = get_bin_bit (feeTriggerBits, ii);
  }
  /// 8 words = FPGA1, ..., FPGA5, NIM1, ..., NIM3
  for (int ii = 0; ii < 8; ii++) {
    run_data.prescale[ii] = words[ii + 5];
  }
  run_data.n_fee_prescale++;
  if (dec_par.verbose > 1) {
    cout << "  feeP ";
    for (int ii = 0; ii < 10; ii++) cout << " " << run_data.trig_bit[ii];
    cout << "\n  feeP ";
    for (int ii = 0; ii <  8; ii++) cout << " " << run_data.prescale[ii];
    cout << "\n";
  }
  return 0;
}

/** Process one Coda STANDARD_PHYSICS event.
 *
 *  Format of words:
 *    { 0:evLength, 1:eventCode, 2:headerLength, 3:headerCode, 4:codaEventID-4, 5:eventType, 6:"0", {roc data #1}, {roc data #2},,, {roc data #n} }
 *
 *   - {roc data #*} = { rocEvLength, rocID, X, X, vmeTime, {board data #1}...{board data #n}, [e906c0da] }
 *   - rocEvLength = N of words on this ROC, excluding the word "rocEvLength" itself.
 *   - rocID = "AA" of "0x00AA0000"
 *   - The format of board data is taken care of in ProcessBoardData().
 */
int MainDaqParser::ProcessCodaPhysics(int* words)
{
  int eventCode = words[1];
  int ret = 0;
  switch (get_hex_bits(eventCode, 7, 4) ) {
  case FLUSH_EVENTS:
    ////KN: test code to check if a specific word exists.
    //PrintWords(words, 0, evLength);
    //PrintCodaEventSummary(words);
    ret = ProcessPhysFlush(words);
    break;
  case SLOW_CONTROL:
    ret = ProcessPhysSlow(words);
    break;
  case PRESTART_INFO:
    ret = ProcessPhysPrestart(words);
    break;
  case STANDARD_PHYSICS: // do nothing
    break;
  case SPILL_COUNTER:
    ret = ProcessPhysSpillCounter(words);
    break;
  case RUN_DESCRIPTOR:
    ret = ProcessPhysRunDesc(words);
    break;
  case BEGIN_SPILL:
    ret = ProcessPhysBOSEOS(words, TYPE_BOS);
    break;
  case END_SPILL:
    ret = ProcessPhysBOSEOS(words, TYPE_EOS);
    break;
  default: // Awaiting further event types
    cout << "Unknown event code: " << eventCode << ".  Ignore.\n";
    break;
  }
  
  return ret;
}

int MainDaqParser::ProcessPhysRunDesc(int* words)
{
  string desc = "";
  int evLength = words[0];
  // Assemble the descriptor from the hex
  for (int i_wd = 4; i_wd < (evLength + 1); i_wd++) {
    unsigned int number = words[i_wd];
    for (int ii = 0; ii < 4; ii++) {
      char word = (char)(number & 0xFF);
      if (word == 0x22) word = 0x27; // replace " with '.
      desc += word;
      number = number >> 8;
    }
  }
  run_data.run_desc = desc;
  run_data.n_run_desc++;
  cout << "  run desc: " << desc.length() << " chars.\n";
  return 0;
}

// Called in ProcessCodaPhysics() on PRESTART_INFO
//   ProcessPhysEvent() is called in main() on PHYSICS_EVENT
// Never called as long as checking run 28700??
int MainDaqParser::ProcessPhysPrestart(int* words)
{
    int evLength = words[0];
    vector<string> list_line;
    string line = "";
    for (int i_wd = 4; i_wd < (evLength + 1); i_wd++) {
      unsigned int number = words[i_wd];
      for (int ii = 0; ii < 4; ii++) {
	if ((number & 0xFF) == '\n') {
	  list_line.push_back(line);
	  line = "";
	} else {
	  line += (char)(number & 0xFF);
	}
	number = number >> 8;
      }
    }
    if (line.length() > 0) list_line.push_back(line);
    else cerr << "Unexpectedly line.length() == 0.\n";
    for (unsigned int ii = 0; ii < list_line.size(); ii++) {
      cout << "  pre " << list_line[ii] << endl;
    }
    return 0;
}

/** Process the Coda physics event from Spill Counter.
 * Kenichi hasn't confirmed with any document but understand that
 * the spill ID in Slow Control means ID of the _previous_ spill.
 * Since this info will be used in the next spill (BOS etc),
 * this function increments the value ("atoi(value) + 1").
 * Because most of the slow-control values except TARGPOS_CONTROL 
 * etc. are of the previous spill, ID for those values are set to
 * "spillID_slow - 1".
 *
 * This function takes the Slow Control Physics Event, which is purely
 * a blob of ASCII presented in the form of parsed and reversed
 * unsigned int's.  It slaps together the whole event into one
 * big string of hex values, parses it into 2-digit/1-char chunks,
 * flips every group of four characters.
 */
int MainDaqParser::ProcessPhysSlow(int* words)
{
  if (dec_par.verbose) cout << "Slow Cont @ coda " << dec_par.codaID << ": ";
  int evLength = words[0];
  dec_par.targPos_slow = 0;
  
  vector<string> list_line;
  string line = "";
  for (int i_wd = 4; i_wd < (evLength + 1); i_wd++) {
    unsigned int number = words[i_wd];
    for (int ii = 0; ii < 4; ii++) {
      if ((number & 0xFF) == '\n') {
	list_line.push_back(line);
	line = "";
      } else {
	line += (char)(number & 0xFF);
      }
      number = number >> 8;
    }
  }
  if (line.length() > 0) list_line.push_back(line);
  else cout << "WARNING: Unexpectedly line.length() == 0.\n";
  
  std::vector<SlowControlData> list_data; //< temporary list
  for (unsigned int ii = 0; ii < list_line.size(); ii++) {
    istringstream iss(list_line[ii]);
    string ts, name, value, type;
    iss >> ts >> name >> value >> type;
    if      (name == "TARGPOS_CONTROL" ) dec_par.targPos_slow = atoi(value.c_str());
    else if (name == "local_spillcount") dec_par.spillID_slow = atoi(value.c_str()) + 1;
    //if (dec_par.verbose) cout << "  slow [" << ts << "] [" << name << "] [" << value << "] [" << type <<"]\n";
    SlowControlData data;
    data.ts       = ts;
    data.name     = name;
    data.value    = value;
    data.type     = type;
    list_data.push_back(data);
    // Kenichi found on 2018-08-30 that the past (and current) decoder always miss the entry with name=U:TODB25
    // since it value includes ' '.  He believe it has to be fixed at the slowcontrol side (i.e. removing ' ').
  }
  for (unsigned int ii = 0; ii < list_data.size(); ii++) {
    unsigned int spill_id = dec_par.spillID_slow - 1; // default value
    SlowControlData* data = &list_data[ii];
    if ( data->type == "target" &&
        (data->name == "TARGPOS_CONTROL"   ||
	 data->name.substr(0, 4) == "FREQ" ||
	 data->name.substr(0, 4) == "PROX" ||
	 data->name.substr(0, 5) == "TARG_"  ) ) {
      spill_id = dec_par.spillID_slow; // No "- 1" according to the original decoder.
    }
    SpillData* spill_data = &(*list_sd)[spill_id];
    spill_data->spill_id = spill_id;
    spill_data->list_slow_cont.push_back(*data); // put into the global list
    spill_data->n_slow++;
  }
  if (dec_par.verbose) cout << "  spill " << dec_par.spillID_slow << ", target " << (short)dec_par.targPos_slow << "\n";
  // In the past decoder, almost all variables obtained here are inserted into
  // the Beam, HV, Environment and Target tables according to "type".
  return 0;
}

/** Process the Coda physics event from Spill Counter.
 * Kenichi hasn't confirmed with any document but understand that
 * the spill ID from Spill Counter means ID of the previous spill.
 * Since this info will be used in the next spill (BOS etc),
 * this function increments the value ("atoi(spillNum) + 1").
 */
int MainDaqParser::ProcessPhysSpillCounter(int* words)
{
  int n_wd = words[0];
  int i_wd_sc = 0;
  char spillNum[12];
  for (int i_wd = 4; i_wd < n_wd + 1; i_wd++) {
    int number = words[i_wd];
    for (int i_byte = 0; i_byte < 4; i_byte++) {
      spillNum[i_wd_sc++] = (char)(number & 0xFF);
      number = number >> 8;
    }
  }
  spillNum[i_wd_sc] = '\0';
  
  /// Keep this new spill ID in the temporal variable (dec_par.spillID_cntr)
  /// since flush events can come after spill counter.
  /// Replace spillID with spillID_cntr at BOS.
  dec_par.spillID_cntr = atoi(spillNum) + 1;
  if (dec_par.verbose) {
    cout << "Spill Counter @ coda = " << dec_par.codaID << ":  spill = " << dec_par.spillID_cntr << "\n";
  }
  return 0;
}

int MainDaqParser::ProcessPhysBOSEOS(int* words, const int type)
{
  string type_str = (type == TYPE_BOS ? "BOS" : "EOS");
  if (type == TYPE_BOS) {
    dec_par.at_bos = true;
    if (PackOneSpillData() != 0) { // if (SubmitEventData() != 0) {
      cout << "Error submitting data.  Exiting...\n";
      return 1;
    }

    /// Regard the Slow Control info as primary (rather than Spill Counter)
    dec_par.spillID     = dec_par.spillID_slow;
    dec_par.targPos     = dec_par.targPos_slow;
    SpillData* data     = &(*list_sd)[dec_par.spillID];
    data->spill_id      = dec_par.spillID;
    data->spill_id_cntr = dec_par.spillID_cntr;
    data->run_id        = dec_par.runID;
    data->targ_pos      = dec_par.targPos;

    /// initialize
    dec_par.spillID_cntr = dec_par.spillID_slow = 0; 
    dec_par.turn_id_max  = 0;
  }

  if (dec_par.verbose) {
    cout << type_str << " @ coda " << dec_par.codaID << ": spill " << dec_par.spillID << ".\n";
  }
  dec_par.spillType = type;

  int idx = 7;
  int evLength = words[0];
  while (idx < evLength && (words[idx] & 0xfffff000) != 0xe906f000) {
    int rocEvLength = words[idx];
    int idx_roc_end = idx + rocEvLength; // inclusive endpoint
    if ( (rocEvLength + idx) > evLength) {
      cout << "Word limit error: " << rocEvLength << " + " << idx << " > " << evLength <<"\n";
      return 1;
    }
    idx++;
    int rocID = get_hex_bits (words[idx], 7, 4);
    dec_par.rocID = rocID;
    idx += 3;
    int codaEvVmeTime = words[idx];
    idx++;
    if (rocID == 2) {
      SpillData* data = &(*list_sd)[dec_par.spillID];
      if (type == TYPE_BOS) {
	data->bos_coda_id  = dec_par.codaID;
	data->bos_vme_time = codaEvVmeTime;
	data->n_bos_spill++;
      } else {
	data->eos_coda_id  = dec_par.codaID;
	data->eos_vme_time = codaEvVmeTime;
	data->n_eos_spill++;
      }
      if (dec_par.verbose > 1) cout << "  " << type_str << " spill: " << dec_par.spillID << " " << dec_par.runID << " " << dec_par.codaID << " " << (short)dec_par.targPos << " " << codaEvVmeTime << "\n";
    }
    /// Skip ROC 25 in END_SPILL since unknown words are placed for debug by Xinkun(?).
    if (type != TYPE_BOS && rocID == 25) idx = idx_roc_end + 1;

    /// Skip this ROC without looking at board data since it's too small (was "< 5")
    /// (as also done in "case FLUSH_EVENTS").
    if (rocEvLength <= 6) idx = idx_roc_end + 1;

    while (idx <= idx_roc_end) {
      int e906flag = words[idx];
      //cout << "  " << type_str << " " << idx << " 0x" << hex << e906flag << dec << endl;
      idx++;
      idx = ProcessBoardData (words, idx, idx_roc_end, e906flag);
      if (idx == -1) return 0;
    }
  }
  return 0;
}

/** Process one FLUSH_EVENTS event.
 *
 * @param[in] words  The word array of one Coda event.
 * @return  "0" if OK.  "-1" if NG.
 *
 */
int MainDaqParser::ProcessPhysFlush(int* words)
{
  dec_par.n_flush_evt_all++;
  const bool print_event = false; ///< If "true", print out ROCs & boards found.
  int evLength = words[0];
  //int codaEvVmeTime = 0;

  //PrintWords(words, 0, evLength+1);
  if (print_event) cout << "\nEvent code = 0x" << hex << words[1] << dec << " (" << evLength << ")";
  
  int idx = 7; // the 1st word of ROC data.
  while (idx < evLength) { // Loop over ROCs
    int rocEvLength = words[idx];
    //KN: In run 23751, ROC 2 often (always?) says "rocEvLength = 6" but 
    // the 1st board (0xe906f00f) says "boardEvLength = 512 or 1024".  
    // We should skip such a case.  This problem is not seen in run 23930.
    if (rocEvLength <= 6) { // Was "< 5" in the previous version.
      idx += rocEvLength + 1;
      continue; // Move to next ROC
    } else if (get_hex_bits(rocEvLength, 7, 4) != 0) {
      cerr << "ERROR: rocEvLength != 0x0000****." << endl;
      return -1;
    }
    
    // evLength is all the words summing all ROCs, and rocEvlLength is just for this ROC only.
    // then, of course, evLength should be greater than rocEvLength
    int idx_roc_end = idx + rocEvLength + 1; // this index itself is _not_ inside this ROC
    if (idx_roc_end > evLength + 1) {
      cerr << "ERROR: ROC Event Length exceeds event length\n"
	   << "  Event: " << dec_par.codaID << ", EventLength = " << evLength 
	   << ", position = " << idx << ", rocEvLength = " << words[idx] << endl;
      return -1;
    }
    
    idx++; // go to next position to get ROCID
    int rocID = get_hex_bits (words[idx], 5, 2);
    if (rocID > 30) {
      cerr << "ERROR: rocID > 30." << endl;
      return -1;
    }
    dec_par.rocID = rocID;
    if (print_event) cout << "\n  ROC " << setw(2) << rocID << " (" << setw(3) << rocEvLength << ") |";
    
    idx += 3; // move from "rocID" to "vmeTime"
    //int vmeTime = words[idx];
    //if (rocID == 2) codaEvVmeTime = vmeTime;
    
    idx++; // move to the 1st word of board data
    while (idx < idx_roc_end) { // Loop over boards
      int e906flag = words[idx];
      //cout << "  board " << idx << " " << e906flag << endl;
      if (get_hex_bits(e906flag, 7, 4) != (int)0xe906) {
	cerr << "Seems not e906flag (0x" << hex << e906flag << dec << ")" << endl;
	idx = idx_roc_end; // try to move to the next ROC
	break;
      }
      if (e906flag == (int)0xe906c0da) { // i.e. end of this ROC
	idx++;
	break;
      }
      idx++; // Move to the 1st word of board data
      int board_id = get_hex_bits(words[idx], 6, 1);
      int n_wd_bd  = get_hex_bits(words[idx], 3, 4);
      if (print_event) cout << hex << " " << (e906flag&0xFFFF) << "@" << board_id << dec << "(" << n_wd_bd << ")";
      idx = ProcessBoardData(words, idx, idx_roc_end, e906flag);
      if (idx == -1) {
        if (dec_par.verbose > 1) cout << "  ProcessBoardData() returned -1:  0x" << hex  << e906flag << dec << " " << board_id << " " << rocID << " " << dec_par.codaID << endl;
        return 0;
      }
    }
    if (idx != idx_roc_end) Abort("idx != idx_roc_end");
  }
  if (print_event) cout << endl;
  dec_par.n_flush_evt_ok++;
  return 0;
}

/** Process the word set of one board.
 * @param[in] idx  The 1st index of the target board (i.e. next to the e906flag).
 * @param[in] idx_roc_end  The last index of the current ROC (not board).  This index itself is not included in the word set of the current ROC, i.e. excluded endpoint.
 * @return  The index that points to the e906flag of the next board (not the last word of the current board).  Or "-1" in case of word overflow (i.e. N of words/board > N of words/ROC).
 *
 * The sub-functions called by this function (like "ProcessBoardJyTDC2()") must return
 * the index that points to the e906flag of the next board (not the last word of the current board).
 * 
 * Note #1:  Taiwan TDC special case
 * According to the crl file, the Taiwan TDC sets this flag (not 0xe906f018) when the number of flush events received exceeds the max defined in crl (9000).  This case is expected to happen about 300 times per spill, since the flush events are sent 9300 times.  The decoder stops reading all remaining words of such a Coda event.
 *
 *
 */
int MainDaqParser::ProcessBoardData (int* words, int idx, int idx_roc_end, int e906flag)
{
  if      ( e906flag == (int)0xE906F003 ) idx = ProcessBoardScaler  (words, idx);
  else if ( e906flag == (int)0xE906F005 ) idx = ProcessBoardV1495TDC(words, idx);
  else if ( e906flag == (int)0xE906F018 ) idx = ProcessBoardJyTDC2  (words, idx, idx_roc_end);
  else if ( e906flag == (int)0xE906F019 ) return -1;
  else if ( e906flag == (int)0xe906f01b ) idx = ProcessBoardFeeQIE      (words, idx);
  else if ( e906flag == (int)0xE906F014 ) idx = ProcessBoardTriggerCount(words, idx);
  else if ( e906flag == (int)0xE906F00F ) idx = ProcessBoardTriggerBit  (words, idx);
  else {
    cerr << "Unexpected board flag in CODA Event " << dec_par.codaID << " ROC " << (int)dec_par.rocID 
	 << ": e906flag = " << e906flag << " @ " << idx-1 << "\n";
    PrintWords(words, idx-10, idx+40);
    return -1; // Temporary solution.  Skip all boards and move to the next ROC.
    Abort("Unexpected board type.");
  }
  return idx;
}

int MainDaqParser::ProcessBoardScaler (int* words, int idx)
{
    int boardID = (0xFFFF & words[idx]);
    idx++;

    SpillData* spill_data = &(*list_sd)[dec_par.spillID];
    spill_data->n_scaler++;
    for (int ii = 0; ii < 32; ii++) {
      unsigned int value = words[idx];
      idx++;

      ScalerData data;
      data.type  = dec_par.spillType;
      data.coda  = dec_par.codaID;
      data.roc   = dec_par.rocID;
      data.board = boardID;
      data.chan  = ii;
      data.value = value;
      if (! dec_par.map_scaler.Find(data.roc, data.board, data.chan, data.name)) {
	if (dec_par.verbose > 1) cout << "  Unmapped Scaler: " << data.roc << " " << data.board << " " << data.chan << "\n";
	continue;
      }
      if (dec_par.verbose > 1) cout << "  scaler " << dec_par.spillID << " " << data.type << " " << data.name << " " << data.value << "\n";
      spill_data->list_scaler.push_back(data);
    }

    return idx;
}

int MainDaqParser::ProcessBoardTriggerBit (int* words, int j)
{
   int n_words = words[j]; // N of words including this word itself (with an exception).
   if (n_words == 0) return j+1; // Exception: n_words = 0 (not 1) in case of no event.
   j++; // Move to the 1st event word.
   n_words--; // Now it means N of trigger-bit & event-ID words.
   //PrintWords(words, j-5, j+n_words+5);
   if (n_words % 2 != 0) {
      PrintWords(words, j-10, j+20);
      Abort("N of trigger words % 2 != 0.");
    }
    int n_evt = n_words / 2; // because one event contains 1 trigger-bits & 1 event ID.
    for (int i_evt = 0; i_evt < n_evt; i_evt++) {
      int triggerBits = words[j + 2*i_evt    ];
      int evt_id      = words[j + 2*i_evt + 1];
      EventData* ed = &(*list_ed)[evt_id];
      ed->n_trig_b++;
      EventInfo* evt = &ed->event;
      SetEventInfo(evt, evt_id);
      evt->trigger_bits = triggerBits;

      int trigger[10];
      for (int i = 0; i < 10; i++) trigger[i] = get_bin_bit (triggerBits, i);
      for (int i = 0; i <  5; i++) {
	// This function doesn't support the bit order for old runs (# < 4923).
        evt->MATRIX[i] = trigger[i  ];
        evt->NIM   [i] = trigger[i+5];
      }
    }

    return j + n_words;
}

int MainDaqParser::ProcessBoardTriggerCount (int* words, int j)
{
  // KN: Do we need to check this as done in the previous version???  14 means STANDARD_PHYSICS
  //if (get_hex_bits (words[1], 7, 4) == 14)

   int n_words = words[j]; // N of words including this word itself (with an exception).
   if (n_words == 0) return j+1; // Exception: n_words = 0 (not 1) in case of no event.
   j++; // Move to the 1st event word.
   n_words--; // Now it means N of trigger-bit & event-ID words.

    if (n_words % 11 != 0) {
      PrintWords(words, j-10, j+20);
      Abort("N of trigger-count words % 11 != 0.");
    }
    int n_evt = n_words / 11; // because one event contains 10 counters & 1 event ID.
    for (int i_evt = 0; i_evt < n_evt; i_evt++) {
      int idx_evt = j + 11*i_evt; // The 1st index of this event
      int evt_id = words[idx_evt + 10];
      EventData* ed = &(*list_ed)[evt_id];
      ed->n_trig_c++;
      EventInfo* evt = &ed->event;
      SetEventInfo(evt, evt_id);
      for (int i_type = 0; i_type < 5; i_type++) {
        evt->RawMATRIX     [i_type] = words[idx_evt + i_type    ];
        evt->AfterInhMATRIX[i_type] = words[idx_evt + i_type + 5];
      }
    }

    return j + n_words;
}

/** Process one QIE event (flag "0xe906f01b").
 *
 * @param[in]  idx    The index of word "ID&N" (i.e. next to the e906flag word) of this board.
 * @return     The index that points to the e906flag of the next board.
 *
 * Word format: { 0xe906f01b, ID&N, [0xe906e906], N, {event #1},,, {event #7} }
 *  - ID&N = 0xAAAABBBB where AAAA = board ID and BBBB = N of words from "X" to end.
 *  - {event #*} ... described inside the function.
 *
 * The word format is described in QIE_REadout_Format_Description.docx of DocDB 537,
 * although the document is not fully up-to-date.
 */
int MainDaqParser::ProcessBoardFeeQIE (int* words, int idx)
{
    int submitQIE = 1;
    if (dec_par.runID < 22400) Abort("feeQIE does not support run < 22400.");

    while (words[idx] == (int)0xe906e906) idx++; // Still necessary??

    // int boardID = get_hex_bits (words[idx], 7, 4);
    int n_wd = get_hex_bits (words[idx], 3, 4);
    int idx_end = idx + n_wd + 1; // It points to the 1st word of the next board.
    idx++;
    if (n_wd == 0) return idx;

    while (words[idx] == (int)0xe906e906) { idx++; idx_end++; }

    idx++; // skip the number-of-words word for this TDC

    // One physics event contains 49 words:
    //   * 1 word for the number of filled words (44 or 39)
    //   * [5 words for spill header] ... appear only in 1st event in spill
    //   * 8 words for 4 presums,
    //   * 2 words for trigger counts
    //   * 2 words for turn ID
    //   * 1 words for rf ID
    //   * 25 words for rf intensities
    //   * 4 or 9 words for padding with "0"
    //   * 1 word for event ID
    // The number of physics events per Coda event is 7 in most cases, but can be less
    // in last Coda event per spill.  Such case is caught by "idx < idx_end".
    for (int i_evt = 0; i_evt < 7 && idx < idx_end; i_evt++) {
      int idx_evt = idx; // Increment idx_evt (not idx) and update idx at loop end.
      int eventID = words[idx_evt + 48];

      int n_wd_evt = get_hex_bits(words[idx_evt], 7, 4);
      if      (n_wd_evt == 44) idx_evt += 5; // skip the spill header
      else if (n_wd_evt != 39) {
	/// Known issue.  See memo.txt.
	//cerr << "!! QIE: Unexpected N of words: "  << dec_par.codaID << " " << i_evt << " " << n_wd_evt << " (" << n_wd << ")." << endl;
	return idx_end;
      }

      idx_evt++; // Move to the 1st word of data block

      unsigned int sums_vals[4]; // QIE records four intensity sums (called "presum")
      for (int i_sum = 0; i_sum < 4 ; i_sum++) {
	sums_vals[i_sum] = get_hex_bits(words[idx_evt],7,4)*65536 + get_hex_bits(words[idx_evt+1],7,4);
	idx_evt += 2;
      }
      //while (words[idx_evt] == (int)0xe906e906) { idx_evt++; idx_end++; }
      
      unsigned int triggerCount = words[idx_evt] | ( get_hex_bits (words[idx_evt+1], 7, 4) );
      idx_evt += 2;
      //while (words[idx_evt] == (int)0xe906e906) { idx_evt++; idx_end++; }
      
      unsigned int turnOnset = words[idx_evt] | ( get_hex_bits (words[idx_evt+1], 7, 4) );
      if (turnOnset > dec_par.turn_id_max) dec_par.turn_id_max = turnOnset;
      idx_evt += 2;
      //while (words[idx_evt] == (int)0xe906e906) { idx_evt++; idx_end++; }
      
      unsigned int rfOnset = get_hex_bits(words[idx_evt],7,4);
      idx_evt++;
      
      unsigned int rf_vals[25];
      for (int i_rf = 0; i_rf < 25; i_rf++) { // RF-12...RF+12
	if (words[idx_evt] == (int)0xe906e906) Abort("Unexpected 0xe906e906 in QIE.");
	rf_vals[i_rf] = ( get_hex_bits(words[idx_evt],7,4) );
	idx_evt++;
      }
      
      if (submitQIE) {
	EventData* ed = &(*list_ed)[eventID];
	ed->n_qie++;
	EventInfo* evt = &ed->event;
	//if (ed->n_qie == 2) {
	//  cout << "QIE#1 " << dec_par.codaID << " " << i_evt << " " << evt->eventID << " " << evt->spillID << " "
	//       << evt->triggerCount << " " << evt->turnOnset << " " << evt->rfOnset << endl;
	//}
	SetEventInfo(evt, eventID);

	evt->qieFlag = 1;
	for (int ii = 0; ii < 4; ii++) evt->sums[ii] = sums_vals[ii];
	evt->triggerCount = triggerCount;
	evt->turnOnset    = turnOnset;
	evt->rfOnset      = rfOnset;
	for (int ii = 0; ii < 25 ; ii++) evt->rf[ii+4] = rf_vals[ii]; // We are now missing first 4 bins and last 4 bins. YC, 2016/05/27

	//if (ed->n_qie == 2) {
	//  cout << "QIE#2 " << dec_par.codaID << " " << i_evt << " " << evt->eventID << " " << evt->spillID << " "
	//       << evt->triggerCount << " " << evt->turnOnset << " " << evt->rfOnset << endl;
	//}
      }
      
      idx += 49;
    }

    if (idx != idx_end) {
      cout << idx << " != " << idx_end;
      Abort("idx != idx_end in feeQIE().");
    }
    return idx_end;
}

/** Process one v1495-TDC event.
 *
 * Word format: { 0xe906f005, boardID(?), N of words, {event #1},,, {event #7} } 
 *  - {event #*} = { hit #1,,, hit #n, header, stop_hit, eventID_CPU, eventID_FPGA_high, eventID_FPGA_low }
 *  - This format was extracted by Kenichi from test file (scaler_6146.dat) on 2017-Jan-06.
 */
int MainDaqParser::ProcessBoardV1495TDC (int* words, int idx)
{
    int boardID   = words[idx++]; // was get_hex_bits (words[idx], 3, 4);
    int n_wd_fpga = words[idx++]; // N of words taken from FPGA buffer.
    if (n_wd_fpga == 0) return idx; // No event data.  Just finish.
    int idx_end = idx + n_wd_fpga; // Exclusive endpoint.  To be incremented in the while loop.
    //PrintWords(words, idx-5, idx_end+20);

    int i_evt = 0;
    vector<int> list_chan;
    vector<int> list_time;
    while (idx < idx_end) {
      if (get_hex_bits(words[idx], 7, 1) == 1) { // event header
	idx_end += 2; // one stop word and one event-ID word (which was added by CPU, not FPGA).
	int word_stop   =  words[idx+1];
	int evt_id      =  words[idx+2]; ///< Stored in CPU
	/// The two words (idx+3 & idx+4) mean event ID stored in FPGA, but
	/// is temporarily fixed to "0" as of 2017-Jan-11.  Thus not checked for now.
	//int evt_id_fpga = (words[idx+3]<<15) + words[idx+4];
	//if (evt_id != evt_id_fpga) {
	//  list_event[i_evt]->dataQuality |= EVT_ERR_V1495;
	//  cerr << "!! EventID mismatch @ v1495: " << evt_id << "@CPU vs " << evt_id_fpga << "@FPGA in " << dec_par.codaID << ":" << i_evt << endl;
	//}
	
	EventData* ed = &(*list_ed)[evt_id];
	ed->n_v1495++;
	EventInfo* evt = &ed->event;
	SetEventInfo(evt, evt_id);

	if (word_stop == (int)0xd2ad) {
	  dec_par.n_1495_d2ad++;
	  evt->dataQuality |= EVT_ERR_V1495;
	} else if (word_stop == (int)0xd3ad) {
	  dec_par.n_1495_d3ad++;
	  evt->dataQuality |= EVT_ERR_V1495;
	} else if (get_hex_bits(word_stop, 3, 1) != 1) {
	  evt->dataQuality |= EVT_ERR_V1495;
	  cerr << "  !! v1495: bad stop word: " << word_stop << endl;
	} else { // OK.  Insert hits.
	  int time_stop   = get_hex_bits(word_stop, 2, 3);
	  for (unsigned int ii = 0; ii < list_chan.size(); ii++) {
	    HitData hit;
	    hit.event = evt_id;
            hit.id    = ++dec_par.hitID;
	    hit.roc   = dec_par.rocID;
	    hit.board = boardID;
	    hit.chan  = list_chan[ii];
	    hit.time  = time_stop - list_time[ii];
            short level; // not stored for now
            if (! dec_par.map_v1495.Find(hit.roc, hit.board, hit.chan, hit.det, hit.ele, level)) {
              if (dec_par.verbose > 1) cout << "  Unmapped v1495: " << hit.roc << " " << hit.board << " " << hit.chan << "\n";
            }
	    ed->list_hit_trig.push_back(hit);
	  }
	  dec_par.n_1495_good++;
	}
	list_chan.clear();
	list_time.clear();
	dec_par.n_1495_all++;
	idx += 5;
	i_evt++;
      } else { // start signal
	/// 0xxxyy where xx is the channel and yy is the channel time
	list_chan.push_back( get_hex_bits (words[idx], 3, 2) );
	list_time.push_back( get_hex_bits (words[idx], 1, 2) );
	idx++;
      }
    }
    if (idx != idx_end) Abort("idx != idx_end in ProcessBoardV1495TDC.");
    return idx_end;
}


/** Process a set of words from board "e906flag=0xe906f018".
 *
 * @param[in]  idx_begin    The index of word "ID&N" (i.e. next to the e906flag word) of this board.
 * @param[in]  idx_roc_end  The last index of the current ROC (not board).
 * @return     The index that points to the e906flag of the next board, i.e. the excluding endpoint of this board.
 *
 * Word format: { e906flag, ID&N, [0xe906e906], X, {event #1}, {event #2},,, {event #7} }
 *  - ID&N = 0x0AAABBBB where AAA = board ID and BBBB = N of words from "X" to end.
 *    Actually board ID occupies one hex ".O.." according to Andrew and thus we can make stricter check.
 *  - {event #*} = { header, hit #1, hit #2,,, hit #n, eventID }
 *
 * Description by Michelle(?):
 * event block = block of data with specific number of events (words), for TDCs
 * _________________________________________________________________________________________________
 * xxx   xxx   xxx   flag   TDCheader   dummyFlag   totalNwords(evtLength)   evtHeader   xxx   xxx
 * xxx   xxx   xxx   xxx...   evtID   evtHeader   xxx   xxx   xxx   xxx   xxx   xxx...   evtID
 * evtHeader   xxx   xxx   xxx   xxx   xxx   xxx...   evtID   TDCheader   totalNwords(evtLength)    
 * evtHeader   xxx   xxx   xxx   xxx   xxx   xxx...   evtID   evtHeader   xxx   xxx   xxx   xxx   
 * xxx   xxx...   evtID
 * ________________________________________________________
 */
int MainDaqParser::ProcessBoardJyTDC2 (int* words, int idx_begin, int idx_roc_end)
{
  int hex7        = get_hex_bits (words[idx_begin], 7, 1);
  int hex54       = get_hex_bits (words[idx_begin], 5, 2);
  int boardID     = get_hex_bits (words[idx_begin], 6, 3);
  int nWordsBoard = get_hex_bits (words[idx_begin], 3, 4);
  if (nWordsBoard == 0) return idx_begin + 1;
  int roc = (int)dec_par.rocID;
  //if (roc < 12 || roc > 30) Abort("rocID out of 12...30.");

  if (hex7 != 0 || hex54 != 0) {
    // According to Kun on 2017-Jan-01, the board sometimes gets to output only 
    // "0x8*******" or "0x9*******".  This "if" condition is strict enough to 
    // catch this error.  Shifter has to reset VME crate to clear this error.
    cerr << "WARNING: Strange 'ID&N' word (0x" << hex << words[idx_begin] << dec << ") @ " << roc << endl;
    //for (int ii=0; ii<7; ii++) list_event[ii]->dataQuality |= 0x1<<(roc-1);
    return idx_begin + 1;
  }

  /// Index range of events: idx_events_begin <= idx < idx_events_end
  int idx_events_begin = idx_begin + 2;
  int idx_events_end   = idx_events_begin + nWordsBoard - 1; // "-1" to exclude the word "X".
  if (words[idx_begin + 1] == (int)0xe906e906) { // Dummy word *could* exist.  Skip it
    idx_events_begin++;
    idx_events_end  ++;
  }
  if (idx_events_end > idx_roc_end) {
    cerr << "WARNING: Word overflow.  Skip ROC (" << roc << ")" << endl;
    //for (int ii=0; ii<7; ii++) list_event[ii]->dataQuality |= EVT_ERR_FMT;
    return -1;
  }

  int i_evt = 0;
  int qual = 0;
  bool header_found = false;
  double time_header = 0;
  vector<int   > list_chan;
  vector<double> list_time;
  for (int idx = idx_events_begin; idx < idx_events_end; idx++) {
    if (get_hex_bits(words[idx], 7, 1) == 8) { // header = stop hit
      if (header_found) { // Not seen in run 23930, but seen in run 23751.
	qual |= 0x1<<(roc-1);
	cerr << "WARNING:  Header after header." << endl;
      }
      int word = words[idx];
      if (get_bin_bit(word, 16) == 0) Abort("Stop signal is not rising.  Not supported.");
      double fine  = 4.0 - get_hex_bit (word, 0) * 4.0 / 9.0;
      double rough = 4.0 * get_hex_bits(word, 3, 3);
      time_header = fine + rough;
      header_found = true;
    } else if (get_hex_bits(words[idx], 7, 1) == 0 &&
	       get_hex_bits(words[idx], 6, 7) != 0   ) { // event ID
      if (! header_found) { // Not seen in run 23930, but seen in run 23751.
	qual |= 0x1<<(roc-1);
	cerr << "WARNING:  eventID without stop word." << endl; // Possible to miss a stop signal???
	dec_par.n_hit_bad++;
	continue;
      }
      int evt_id = words[idx];
      EventData* ed = &(*list_ed)[evt_id];
      ed->n_tdc++;
      EventInfo* evt = &ed->event;
      SetEventInfo(evt, evt_id);
      evt->dataQuality |= qual;

      /// The data of one physics event are complete.  Let's insert hits to the storage.
      for (unsigned int ii = 0; ii < list_chan.size(); ii++) {
	HitData hit;
	hit.event = evt_id;
        hit.id    = ++dec_par.hitID;
	hit.roc   = roc;
	hit.board = boardID;
	hit.chan  = list_chan[ii];
	hit.time  = list_time[ii];
        if (! dec_par.map_taiwan.Find(hit.roc, hit.board, hit.chan, hit.det, hit.ele)) {
          if (dec_par.verbose > 1) cout << "  Unmapped Taiwan: " << hit.roc << " " << hit.board << " " << hit.chan << "\n";
        }
	ed->list_hit.push_back(hit);
	//cout << " HIT " << dec_par.spillID << " " << evt_id << " " << (int)dec_par.rocID << " " << boardID << " " << list_chan[ii] << " " << list_time[ii] << endl;
      }
      i_evt++;
      qual = 0;
      header_found = false;
      list_chan.clear();
      list_time.clear();
    } else { // start hit
      if (! header_found) { // Not seen in run 23930, but seen in run 23751.
	qual |= 0x1<<(roc-1);
	cerr << "WARNING:  Start without stop word." << endl; // Possible to miss a stop signal???
	dec_par.n_hit_bad++;
	continue;
      }
      int word = words[idx];
      if (get_bin_bit(word, 16) == 0) { // Not seen in run 23930, but seen in run 23751.
	qual |= 0x1<<(roc-1);
	cerr << "WARNING:  Start signal is not rising." << endl;
	dec_par.n_hit_bad++;
      }
      double fine  = 4.0 - get_hex_bit (word, 0) * 4.0 / 9.0;
      double rough = 4.0 * get_hex_bits(word, 3, 3);
      double time = time_header - (fine + rough);
      if (time < 0) time += 4096; // This constant 4096 is correct???
      list_time.push_back(time);
      
      int cable_chan = get_hex_bit (word, 6);
      int cable_id   = get_bin_bits(word, 29, 2);
      int chan       = cable_chan + 16*cable_id;
      list_chan.push_back(chan);
    }
  }
  if (header_found) { // Not seen in run 23930, but seen in run 23751.
    qual |= 0x1<<(roc-1);
    cerr << "WARNING:  Not finished cleanly." << endl;
  }
  
  return idx_events_end;
}

int MainDaqParser::PackOneSpillData()
{
  if (dec_par.verbose) cout << "PackOneSpillData(): n=" << list_ed->size() << endl;

  sd_now = &(*list_sd)[dec_par.spillID];
  
  dec_par.n_phys_evt_all += list_ed->size();

  for (EventDataMap::iterator it = list_ed->begin(); it != list_ed->end(); it++) {
    unsigned int evt_id = it->first;
    if (evt_id == 0) continue; // 1st event?  bad anyway
    if (dec_par.sampling > 0 && evt_id % dec_par.sampling != 0) continue;

    EventData* ed = &it->second;
    EventInfo* event  = &ed->event;
    if (dec_par.turn_id_max > 360000 && event->turnOnset == 0 && event->NIM[2]) continue;
    dec_par.n_phys_evt_dec++;

    unsigned int n_tdc; // expected number of tdc info
    if (dec_par.runID >= 28664) n_tdc = 101; // seen in run 28700
    else                        n_tdc = 108; // seen in run 28000
    // This run range was confirmed by checking n_tdc in the following runs;
    //  * 101 in runs 28680, 28670, 28665, 28664
    //  * 108 in runs 28500, 28600, 28650, 28660, 28661
    // Note that runs 28662 & 28663 contain no event.
    // Is the change of n_tdc related to elog #17385??
    // https://e906-gat6.fnal.gov:8080/SeaQuest/17385
    if (ed->n_tdc != n_tdc) event->dataQuality |= EVT_ERR_N_TDC;

    if      (ed->n_v1495  < 2) event->dataQuality |= EVT_ERR_N_V1495_0;
    else if (ed->n_v1495  > 2) event->dataQuality |= EVT_ERR_N_V1495_2;
    if      (ed->n_trig_b < 1) event->dataQuality |= EVT_ERR_N_TRIGB_0;
    else if (ed->n_trig_b > 1) event->dataQuality |= EVT_ERR_N_TRIGB_2;
    if      (ed->n_trig_c < 1) event->dataQuality |= EVT_ERR_N_TRIGC_0;
    else if (ed->n_trig_c > 1) event->dataQuality |= EVT_ERR_N_TRIGC_2;
    if      (ed->n_qie    < 1) event->dataQuality |= EVT_ERR_N_QIE_0;
    else if (ed->n_qie    > 1) event->dataQuality |= EVT_ERR_N_QIE_2;

    if (dec_par.verbose > 1) {
      if (ed->n_qie != 1 || ed->n_v1495 != 2 || ed->n_tdc != n_tdc || ed->n_trig_b != 1 || ed->n_trig_c != 1) {
	cout << "  N! " << evt_id << " | " << ed->n_qie << " " << ed->n_v1495 << " " << ed->n_tdc << " " << ed->n_trig_b << " " << ed->n_trig_c << endl;
      }
    }

    unsigned int n_taiwan = ed->list_hit     .size();
    unsigned int n_v1495  = ed->list_hit_trig.size();
    dec_par.n_hit  += n_taiwan;
    dec_par.n_thit += n_v1495;
    if (dec_par. n_hit_max < n_taiwan) dec_par. n_hit_max = n_taiwan;
    if (dec_par.n_thit_max < n_v1495 ) dec_par.n_thit_max = n_v1495 ;
  }

  EventDataMap* ptr = list_ed_now;
  list_ed_now = list_ed;
  list_ed = ptr;

  return 0;
}

void MainDaqParser::SetEventInfo(EventInfo* evt, const int eventID)
{
  evt->runID       = dec_par.runID;
  evt->eventID     = eventID;
  evt->spillID     = dec_par.spillID;
}
