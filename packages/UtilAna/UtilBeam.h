#ifndef _UTIL_BEAM__H_
#define _UTIL_BEAM__H_
class TH1;

namespace UtilBeam {
  void ListOfRfValues(int& n_value, int*& list_values);
  void ListOfRfValues(int& n_value, double*& list_values);
  void NormRFHist(TH1* h1);
}; // namespace UtilBeam

#endif /* _UTIL_BEAM__H_ */
