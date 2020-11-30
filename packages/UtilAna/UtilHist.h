#ifndef _UTIL_HIST__H_
#define _UTIL_HIST__H_
class TH1;
class TH2;
class TH3;

namespace UtilHist {
  /// Find the lowest and highest bins ("bin_lo" and "bin_hi") out of non-empty bins of "h1".
  void FindFilledRange(TH1* h1, int& bin_lo, int& bin_hi);

  /// Adjust the axis range via "h1->GetXaxis()->SetRange(bin_lo, bin_hi)" to zoom up non-empty bins.
  void AutoSetRange (TH1* h1, const int margin_lo=5, const int margin_hi=5);
  void AutoSetRangeX(TH2* h2, const int margin_lo=5, const int margin_hi=5);
  void AutoSetRangeY(TH2* h2, const int margin_lo=5, const int margin_hi=5);
};

#endif /* _UTIL_HIST__H_ */
