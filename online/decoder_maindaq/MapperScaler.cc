#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include "MapperScaler.h"
using namespace std;

void MapperScaler::ReadFile(const string fn_tsv)
{
  cout << "  MapperScaler::ReadFile(): " << fn_tsv << "... ";
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
    string name;
    short  roc, board, chan;
    if (! (iss >> name >> roc >> board >> chan)) continue;
    m_map[RocBoardChan_t(roc, board, chan)] = name;
    nn++;
  }
  ifs.close();
  cout << nn << " read in." << endl;
}

bool MapperScaler::Find(const short roc, const short board, const short chan,  std::string& name)
{
  RocBoardChan_t key(roc, board, chan);
  if (m_map.find(key) != m_map.end()) {
    name = m_map[key];
    return true;
  }
  return false;
}  
