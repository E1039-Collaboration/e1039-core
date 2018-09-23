// $Id: $

/*!
 * \file PHFieldConfig_v3.h
 * \brief 
 * \author Jin Huang <jhuang@bnl.gov>
 * \version $Revision:   $
 * \date $Date: $
 */

#ifndef PHFieldConfig_v3_H_
#define PHFieldConfig_v3_H_

#include "PHFieldConfig.h"


/*!
 * \brief PHFieldConfig_v3 implements field configuration information for input a field map file */
class PHFieldConfig_v3 : public PHFieldConfig
{
 public:
  PHFieldConfig_v3(
      const std::string& filename1,
			const std::string& filename2,
			const double targetmag_y = 5.0);

  //! default constructor for ROOT file IO
  PHFieldConfig_v3() {PHFieldConfig_v3("INVALID FILE1", "INVALID FILE2");}

  virtual ~PHFieldConfig_v3();

  /// Virtual copy constructor.
  virtual PHObject*
  clone() const;

  /** identify Function from PHObject
   @param os Output Stream
   */
  virtual void
  identify(std::ostream& os = std::cout) const;

  /// Clear Event
  virtual void
  Reset();

  /// isValid returns non zero if object contains vailid data
  virtual int
  isValid() const;

  FieldConfigTypes get_field_config() const
  {
    return field_config_;
  }
  void set_field_config(FieldConfigTypes fieldConfig)
  {
    field_config_ = fieldConfig;
  }

  const std::string& get_filename1() const
  {
    return filename1_;
  }

  void set_filename1(const std::string& filename)
  {
    filename1_ = filename;
  }

  const std::string& get_filename2() const
  {
    return filename2_;
  }

  void set_filename2(const std::string& filename)
  {
    filename2_ = filename;
  }

	double get_taregetmag_y() const {
		return _taregetmag_y;
	}

	void set_taregetmag_y(double taregetmagY) {
		_taregetmag_y = taregetmagY;
	}

 protected:
  FieldConfigTypes field_config_;
  std::string filename1_;
  std::string filename2_;
  double _taregetmag_y;

  ClassDef(PHFieldConfig_v3, 3)
};

#endif /* PHFieldConfig_v3_H_ */
