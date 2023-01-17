//rs_Reader.cc
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <cstring>
#include "rs_Reader.h"
using namespace std;

rs_Reader::rs_Reader(const char* file_name) 
{ 
  //READ IN FILE NAME
    if (file_name  == nullptr) {
        str = new char[1];
        str[0] = '\0';
    }
 
    else {
 
        str = new char[strlen(file_name) + 1];
        //Copy character of val[]
        // using strcpy
        strcpy(str, file_name);
        str[strlen(file_name)] = '\0';
        cout << "The string passed is: "<< str << endl;
    }

   //READ IN DATA FROM RS
   ifstream infile_rs; 
   infile_rs.open(file_name);
   if (!infile_rs.is_open()){
            cout << "Error opening RS file" <<endl;
            exit(1);
   }
   infile_rs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');    
   
   for(int i = 0; i < 256; i++){
     infile_rs >> road_id[i] >> pol[i] >> H1X[i] >> H2X[i] >> H3X[i] >> H4X[i] >> signal[i] >> background[i];

   } 

   infile_rs.close();   
}
