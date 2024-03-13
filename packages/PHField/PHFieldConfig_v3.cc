// $Id: $

/*!
 * \file PHFieldConfig_v3.cc
 * \brief 
 * \author Jin Huang <jhuang@bnl.gov>
 * \version $Revision:   $
 * \date $Date: $
 */

#include "PHFieldConfig_v3.h"

#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TMemFile.h>

#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

PHFieldConfig_v3::PHFieldConfig_v3(
    const std::string& filename1,
		const std::string& filename2,
		const double scale1,
		const double scale2,
		const double targetmag_y) :
		field_config_(kFieldSeaQuest),
		filename1_(filename1),
		filename2_(filename2),
		scale1_(scale1),
		scale2_(scale2),
		_taregetmag_y(targetmag_y)
{
  if(filename1_.find("INVALID") == string::npos)  //suppress output when default ctor is called
  {
    cout << "PHFieldConfig_v3::PHFieldConfig_v3:" << endl;
    cout << " from file1 [" << filename1 << "]" << endl;
    cout << "  and file2 [" << filename2 << "]" << endl;
    cout << "scale1: " << setprecision(5) << scale1_ << ", scale2: " << setprecision(5) << scale2_ << ", targetmag_y: " << _taregetmag_y << endl;
  }
}

PHFieldConfig_v3::~PHFieldConfig_v3()
{
}

/// Virtual copy constructor.
PHObject*
PHFieldConfig_v3::clone() const
{
  return new PHFieldConfig_v3(*this);
}

/** identify Function from PHObject
 @param os Output Stream
 */
void PHFieldConfig_v3::identify(std::ostream& os) const
{
  os << "PHFieldConfig_v3::identify -";
  if (isValid())
  {
    os << " Field type of [" << "Sea Quest: " << get_field_config_description().c_str() << "]" << endl;
    os << " from file1 [" << get_filename1() << "]" << endl;
    os << "  and file2 [" << get_filename2() << "]" << endl;
    os << "scale1: " << scale1_ << ", scale2: " << scale2_ << ", targetmag_y: " << _taregetmag_y << endl;
    os << "scale1: " << get_magfield_rescale1() << ", scale2: " << get_magfield_rescale2() << ", targetmag_y: " << get_taregetmag_y() << endl;
  }
  else
    os << "Empty";
  os << endl;
}
/// Clear Event
void PHFieldConfig_v3::Reset()
{
}

/// isValid returns non zero if object contains vailid data
int PHFieldConfig_v3::isValid() const
{
  return (filename1_.length()>0 && filename2_.length()>0 );
}
