#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include "MapperTaiwan.h"
using namespace std;

MapperTaiwan::MapperTaiwan()
{
  m_map_name2id["D0U"  ] =   1;
  m_map_name2id["D0Up" ] =   2;
  m_map_name2id["D0X"  ] =   3;
  m_map_name2id["D0Xp" ] =   4;
  m_map_name2id["D0V"  ] =   5;
  m_map_name2id["D0Vp" ] =   6;
  m_map_name2id["D1U"  ] =   1+6;
  m_map_name2id["D1Up" ] =   2+6;
  m_map_name2id["D1X"  ] =   3+6;
  m_map_name2id["D1Xp" ] =   4+6;
  m_map_name2id["D1V"  ] =   5+6;
  m_map_name2id["D1Vp" ] =   6+6;
  m_map_name2id["D2V"  ] =   7+6;
  m_map_name2id["D2Vp" ] =   8+6;
  m_map_name2id["D2Xp" ] =   9+6;
  m_map_name2id["D2X"  ] =  10+6;
  m_map_name2id["D2U"  ] =  11+6;
  m_map_name2id["D2Up" ] =  12+6;
  m_map_name2id["D3pVp"] =  13+6;
  m_map_name2id["D3pV" ] =  14+6;
  m_map_name2id["D3pXp"] =  15+6;
  m_map_name2id["D3pX" ] =  16+6;
  m_map_name2id["D3pUp"] =  17+6;
  m_map_name2id["D3pU" ] =  18+6;
  m_map_name2id["D3mVp"] =  19+6;
  m_map_name2id["D3mV" ] =  20+6;
  m_map_name2id["D3mXp"] =  21+6;
  m_map_name2id["D3mX" ] =  22+6;
  m_map_name2id["D3mUp"] =  23+6;
  m_map_name2id["D3mU" ] =  24+6;
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
  m_map_name2id["P1Hf" ] =  41+6;
  m_map_name2id["P1Hb" ] =  42+6;
  m_map_name2id["P1Vf" ] =  43+6;
  m_map_name2id["P1Vb" ] =  44+6;
  m_map_name2id["P2Vf" ] =  45+6;
  m_map_name2id["P2Vb" ] =  46+6;
  m_map_name2id["P2Hf" ] =  47+6;
  m_map_name2id["P2Hb" ] =  48+6;

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

  for (int ipl = 1; ipl <= 9; ipl++) {
    char tmpName[8][10];
    sprintf(tmpName[0], "P1H%df", ipl);
    sprintf(tmpName[1], "P1H%db", ipl);
    sprintf(tmpName[2], "P1V%df", ipl);
    sprintf(tmpName[3], "P1V%db", ipl);
    sprintf(tmpName[4], "P2V%df", ipl);
    sprintf(tmpName[5], "P2V%db", ipl);
    sprintf(tmpName[6], "P2H%df", ipl);
    sprintf(tmpName[7], "P2H%db", ipl);
    
    m_map_name2id[tmpName[0]] = 41+6;
    m_map_name2id[tmpName[1]] = 42+6;
    m_map_name2id[tmpName[2]] = 43+6;
    m_map_name2id[tmpName[3]] = 44+6;
    m_map_name2id[tmpName[4]] = 45+6;
    m_map_name2id[tmpName[5]] = 46+6;
    m_map_name2id[tmpName[6]] = 47+6;
    m_map_name2id[tmpName[7]] = 48+6;
  }
}

void MapperTaiwan::ReadFile(const string fn_tsv)
{
  cout << "  MapperTaiwan::ReadFile(): " << fn_tsv << "... ";
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
    short  ele, roc, board, chan;
    if (! (iss >> det >> ele >> roc >> board >> chan)) continue;
    m_map[RocBoardChan_t(roc, board, chan)] = DetEle_t(det, ele);
    nn++;
  }
  ifs.close();
  cout << nn << " read in." << endl;
}

bool MapperTaiwan::Find(const short roc, const short board, const short chan,  std::string& det, short& ele)
{
  RocBoardChan_t key(roc, board, chan);
  if (m_map.find(key) != m_map.end()) {
    DetEle_t* det_ele = &m_map[key];
    det = det_ele->first;
    ele = det_ele->second;
    return true;
  }
  return false;
}  

bool MapperTaiwan::Find(const short roc, const short board, const short chan,  short& det, short& ele)
{
  string det_str;
  if (! Find(roc, board, chan, det_str, ele)) return false;
  if (m_map_name2id.find(det_str) != m_map_name2id.end()) {
    det = m_map_name2id[det_str];
    return true;
  }
  return false;
}

//bool MapperTaiwan::FindInv(const std::string det, const int ele,  int& roc, int& board, int& chan)
//{
//  for (int i_ent = 0; i_ent < m_n_ent; i_ent++) {
//    if(m_map_det[i_ent] == det &&
//       m_map_ele[i_ent] == ele    ) {
//      roc   = m_map_roc  [i_ent];
//      board = m_map_board[i_ent];
//      chan  = m_map_chan [i_ent];
//      return true;
//    }
//  }
//  return false;
//}


//void MapperTaiwan::Print(std::ostream& ofs)
//{
//   if (! m_init) Init();
//   ofs << "================================================================\n"
//       << "              " << NameCham() << " :  TDC ID -> Element ID\n"
//       << "================================================================\n"
//       << "   " << m_n_ent << " valid mapping entries,  "
//       << m_n_ent_all << " all entries\n";
//   for (int i_ent = 0; i_ent < m_n_ent; i_ent++) {
//      ofs << "   "
//          << setw(2) << m_map_roc  [i_ent] << "  "
//          << setw(4) << m_map_board[i_ent] << "  "
//          << setw(2) << m_map_chan [i_ent] << "  ->  "
//          << setw(2) << m_map_plane[i_ent] << "  "
//          << setw(3) << m_map_ele  [i_ent] << endl;
//   }
//   ofs << endl
//       << "================================================================\n";
//}
//
//void MapperTaiwan::PrintInv(std::ostream& ofs)
//{
//   if (! m_init) Init();
//   ofs << "================================================================\n"
//       << "              " << NameCham() << " :  Element ID -> TDC ID\n"
//       << "================================================================\n";
//   for (int i_pl = 0; i_pl < NumPlanes(); i_pl++) {
//      ofs << "   Plane " << i_pl << endl;
//      int n_ent = 0;
//      for (int i_ent = 0; i_ent < m_n_ent; i_ent++) {
//         if (m_map_plane[i_ent] != i_pl) continue;
//         ofs << "      "
//             << setw(3) << m_map_ele  [i_ent] << "  ->  "
//             << setw(2) << m_map_roc  [i_ent] << "  "
//             << setw(4) << m_map_board[i_ent] << "  "
//             << setw(2) << m_map_chan [i_ent] << endl;
//         n_ent++;
//      }
//      ofs << "      " << n_ent << " mapping entries\n"
//          << "================================================================\n";
//   }
//}
