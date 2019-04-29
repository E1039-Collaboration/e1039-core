#ifndef __PARAM_RUN_RANGE_H__
#define __PARAM_RUN_RANGE_H__
#include <iostream>
#include <vector>
#include <string>

class ParamRunRange {
  struct RangeItem {
    int run_b;
    int run_e;
    std::string map_id;
  };
  typedef std::vector<RangeItem> RangeList;
  RangeList m_list;

 public:
  ParamRunRange() {;}
  ~ParamRunRange() {;}
  void Add(const int run_b, const int run_e, const std::string map_id);
  bool Find(const std::string map_id);
  std::string Find(const int run, const bool exit_on_error=true);

  void ReadFromFile(const std::string fn_tsv);
  void ReadFromDB  (const std::string schema);
  void WriteToDB   (const std::string schema);
};

#endif // __PARAM_RUN_RANGE_H__
