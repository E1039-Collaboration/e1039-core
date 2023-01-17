#ifndef _RS_READER__H_
#define _RS_READER__H_
#include "OnlMonClient.h"
#include <string>

class rs_Reader: public OnlMonClient {
//variables
 public:
 char* str;
 
 private:
 
 int road_id[256];
 int pol[256];
 int H1X[256];
 int H2X[256];
 int H3X[256];
 int H4X[256];
 int signal[256];
 int background[256];

 
//functions
 public:
  rs_Reader(const char* file_name); //const int lvl);
  virtual ~rs_Reader() {}
  OnlMonClient* Clone() { return new rs_Reader(*this); }

 private:
 int get_road_id(int index);
 int get_pol(int index);
 int get_H2X(int index);
 int get_H4X(int index);
};

#endif /* RS_READER__H_ */
                                                   
