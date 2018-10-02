#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include "MapperV1495.h"
using namespace std;

MapperV1495::MapperV1495()
{
  m_map_name2id["H1B"  ] =  25+6;
  m_map_name2id["H1T"  ] =  26+6;
  m_map_name2id["H1L"  ] =  27+6;
  m_map_name2id["H1R"  ] =  28+6;
  m_map_name2id["H2L"  ] =  29+6;
  m_map_name2id["H2R"  ] =  30+6;
  m_map_name2id["H2B"  ] =  31+6;
  m_map_name2id["H2T"  ] =  32+6;
  m_map_name2id["H3B"  ] =  33+6;
  m_map_name2id["H3T"  ] =  34+6;
  m_map_name2id["H4Y1L"] =  35+6;
  m_map_name2id["H4Y1R"] =  36+6;
  m_map_name2id["H4Y2L"] =  37+6;
  m_map_name2id["H4Y2R"] =  38+6;
  m_map_name2id["H4B"  ] =  39+6;
  m_map_name2id["H4T"  ] =  40+6;

  m_map_name2id["H4Bu"  ] = 39+6;
  m_map_name2id["H4Bd"  ] = 39+6;
  m_map_name2id["H4Tu"  ] = 40+6;
  m_map_name2id["H4Td"  ] = 40+6;
  m_map_name2id["H4Y1Ll"] = 35+6;
  m_map_name2id["H4Y1Lr"] = 35+6;
  m_map_name2id["H4Y1Rl"] = 36+6;
  m_map_name2id["H4Y1Rr"] = 36+6;
  m_map_name2id["H4Y2Ll"] = 37+6;
  m_map_name2id["H4Y2Lr"] = 37+6;
  m_map_name2id["H4Y2Rl"] = 38+6;
  m_map_name2id["H4Y2Rr"] = 38+6;
}

void MapperV1495::ReadFile(const string fn_tsv)
{
  cout << "  MapperV1495::ReadFile(): " << fn_tsv << "... ";
  ifstream ifs(fn_tsv.c_str());
  if (! ifs) {
    cerr << "\n!!ERROR!!  Cannot open the map file '" << fn_tsv << "'." << endl;
    exit(1);
  } 

  string buffer;
  istringstream iss;
  getline(ifs, buffer); // discard the 1st line
  unsigned int nn = 0;
  while ( getline(ifs, buffer) ) {
    iss.clear(); // clear any error flags
    iss.str(buffer);
    string det;
    short ele, lvl, roc, board, chan;
    if (! (iss >> det >> ele >> lvl >> roc >> board >> chan)) continue;
    m_map[RocBoardChan_t(roc, board, chan)] = DetEleLvl_t(det, ele, lvl);
    nn++;
  }
  ifs.close();
  cout << nn << " read in." << endl;
}

bool MapperV1495::Find(const short roc, const short board, const short chan,  std::string& det, short& ele, short& lvl)
{
  RocBoardChan_t key(roc, board, chan);
  if (m_map.find(key) != m_map.end()) {
    DetEleLvl_t* value = &m_map[key];
    det = std::get<0>(*value);
    ele = std::get<1>(*value);
    lvl = std::get<2>(*value);
    return true;
  }
  return false;
}  

bool MapperV1495::Find(const short roc, const short board, const short chan,  short& det, short& ele, short& lvl)
{
  string det_str;
  if (! Find(roc, board, chan, det_str, ele, lvl)) return false;
  if (m_map_name2id.find(det_str) != m_map_name2id.end()) {
    det = m_map_name2id[det_str];
    return true;
  }
  return false;
}
