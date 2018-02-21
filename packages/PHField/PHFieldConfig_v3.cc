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
#include <sstream>

using namespace std;

PHFieldConfig_v3::PHFieldConfig_v3(
    const std::string& filename1,
		const std::string& filename2):
				field_config_(kFieldSeaQuest),
				filename1_(filename1),
				filename2_(filename2)
{
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
    os << " Field type of [" << "Sea Quest";
    os << "] from file1 [" << get_filename1();
    os << "] from file2 [" << get_filename2();
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
