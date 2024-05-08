#ifndef _RS_READER__H_
#define _RS_READER__H_
#include <string>
#include <iostream>
#include <vector>
 
using namespace std;

class rs{
public:
 int road_id;
 int H1X;
 int H2X;
 int H3X;
 int H4X;
 int signal;
 int background;
 
 rs(){};
};

class rs_Reader {
 public:  
  std::string file_name;
  int N_ROADS;
  int pol;
  std::vector<rs> roads;
 
//functions
  rs_Reader(const std::string file_name); 

};

#endif /* RS_READER__H_ */
                                                   
