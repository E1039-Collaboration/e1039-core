#ifndef _UTIL_HODO__H_
#define _UTIL_HODO__H_
#include <string>
#include <vector>
#include <TGraph.h>
#include <TVector3.h>
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
    TGraph*  GetGraphX();
    TGraph*  GetGraphY();
    int      GetNDF   ();
    double   GetChi2  ();
    TVector3 GetPos0  ();
    TVector3 GetSlope ();
    TVector3 GetPos   (const double z);
  };
}; // namespace UtilHodo

#endif /* _UTIL_HODO__H_ */
