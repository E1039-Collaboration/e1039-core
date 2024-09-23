//rs_Reader.cc
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <cstring>
#include <vector>
#include "rs_Reader.h"
using namespace std;

rs_Reader::rs_Reader(const std::string file_name_in) 
  : file_name(file_name_in)
{ 
  //cout << "Opening File with RS: "<< file_name << endl;

   //READ IN DATA FROM RS
   ifstream infile_rs; 
   infile_rs.open(file_name);
   if (!infile_rs.is_open()){
            cout << "Error opening RS file" <<endl;
            exit(1);
   }
   infile_rs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');    
   int pol_temp;
   int road_count = 0;
   rs rs_temp;

   while(infile_rs >> rs_temp.road_id >> pol_temp >> rs_temp.H1X >> rs_temp.H2X >> rs_temp.H3X >> rs_temp.H4X >> rs_temp.signal >> rs_temp.background){
  	roads.push_back(rs_temp); 
	road_count ++;
   }
   pol = pol_temp;
   N_ROADS = road_count;	
/*
   rs_vec.push_back(rs_vals());
   int count = 0;  
  
   while(infile_rs >> rs_vec[count].road_id >> pol >> rs_vec[count].H1X >> rs_vec[count].H2X >> rs_vec[count].H3X >> rs_vec[count].H4X >> rs_vec[count].signal >> rs_vec[count].background){
     count++;
     rs_vec.push_back(rs_vals()); 
   }

   vector<rs_vals>::iterator it;
  
   it = rs_vec.end()-1;
   rs_vec.erase(it);

   N_ROADS = rs_vec.size();
*/
   infile_rs.close();   
}
