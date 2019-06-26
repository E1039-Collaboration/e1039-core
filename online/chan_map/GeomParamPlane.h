#ifndef __GEOM_PARAM_PLANE_H__
#define __GEOM_PARAM_PLANE_H__
#include "RunParamBase.h"

class GeomParamPlane : public GeomParamBase {
  struct Plane {
    std::string det_name; // detectorName;
    int    n_ele; // numElements;
    double cell_spacing;
    double cell_width;
    double angle_from_vert;
    double xoffset; // xPrimeOffset;
    double width; // planeWidth;
    double height; // planeHeight;
    double x0;
    double y0;
    double z0;
    double theta_x;
    double theta_y;
    double theta_z;
  };
  typedef std::vector<Plane> List_t;
  List_t m_list; ///< Used to keep all information in the added order.

  typedef std::map<std::string, Plane> Map_t;
  Map_t m_map; ///< Used in Find() for better speed.

 public:
  GeomParamPlane();
  virtual ~GeomParamPlane() {;}

  void Add (const Plane& plane);
  bool Find(const std::string det_name, Plane*& plane);
  void Print(std::ostream& os);

 protected:
  int  ReadFileCont(LineList& lines);
  int WriteFileCont(std::ostream& os);

  void  ReadDbTable(DbSvc& db);
  void WriteDbTable(DbSvc& db);
};

#endif // __GEOM_PARAM_PLANE_H__
