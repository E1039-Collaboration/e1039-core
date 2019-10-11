#ifndef _UTIL_HODO__H_
#define _UTIL_HODO__H_
#include <string>
#include <vector>
#include <TGraph.h>
#include <TVector3.h>
class TH1;
class TH2;
class SQHit;

namespace UtilHodo {
  bool IsHodoX(const std::string det_name);
  bool IsHodoY(const std::string det_name);
  bool IsHodoX(const int det_id);
  bool IsHodoY(const int det_id);
  int GetPlanePos  (const int det,                double& x, double& y, double& z);
  int GetElementPos(const int det, const int ele, double& x, double& y, double& z);

  struct Track1D {
    typedef enum { X, Y } XY_t;
    typedef std::vector<SQHit*> HitList_t;
    XY_t      type_xy;
    HitList_t list_hit;
    TGraph    graph;
    int      ndf;
    double   chi2;
    double   pos; //< Track position at z = 0.
    double   slope; //< Track slope, i.e. dx/dz or dy/dz.
    Track1D(const XY_t xy);
    ~Track1D() {;}
    int DoTracking();
  };

  struct Track2D {
    Track1D trk_x;
    Track1D trk_y;

    Track2D();
    ~Track2D() {;}
    void AddHit(SQHit* hit);
    int DoTracking();
    int      GetNDF  ();
    double   GetChi2 ();
    TVector3 GetPos0 ();
    TVector3 GetSlope();
    TVector3 GetPos  (const double z);
  };

  class HodoTrack {
    typedef std::vector<SQHit*> HitList_t;
    HitList_t m_list_hit_x;
    HitList_t m_list_hit_y;
    TGraph    m_gr_x;
    TGraph    m_gr_y;
    int      m_ndf;
    double   m_chi2;
    TVector3 m_pos0; //< Track position at z = 0.
    TVector3 m_slope; //< Track slope.  It is normalized so that z = +1.
    
  public:
    HodoTrack() {;}
    ~HodoTrack() {;}
    void AddHit(SQHit* hit);
    int DoTracking();

    int    GetNDF () { return m_ndf ; }
    double GetChi2() { return m_chi2; }
    TVector3* GetPos  () { return &m_pos0 ; }
    TVector3* GetSlope() { return &m_slope; }
    TVector3  GetPos(const double z);

  protected:
    int DoTracking1D(const bool is_x, HitList_t* list_hit, TGraph* gr);
  };

};

#endif /* _UTIL_HODO__H_ */
