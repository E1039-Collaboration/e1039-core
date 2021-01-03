/*
 * SQScaler.h
 */
#ifndef _H_SQScaler_H_
#define _H_SQScaler_H_
#include <phool/PHObject.h>
#include <iostream>
#include <limits>
#include <string>

/// An SQ interface class to hold the data of one scaler channel.
/**
 * A list of this objects is held by SQSpill.
 */
class SQScaler : public PHObject {
public:
  typedef enum {
    UNDEF = -1,
    BOS   =  0,
    EOS   =  1
  } ScalerType_t;
  
  virtual ~SQScaler() {}

  // PHObject virtual overloads

  virtual void         identify(std::ostream& os = std::cout) const {
    os << "---SQScaler base class------------" << std::endl;
  }
  virtual void      Reset() {};
  virtual int       isValid() const {return 0;}
  virtual SQScaler* Clone() const {return NULL;}
  virtual PHObject* clone() const {return NULL;}

  virtual ScalerType_t get_type() const {return UNDEF;} ///< Return 'BOS' or 'EOS' according to the timing when this channel was read out.
  virtual void set_type(const ScalerType_t a) {}
  virtual std::string get_name() const {return "";} ///< Return the name of this scaler channel.
  virtual void set_name(const std::string a) {}
  virtual int  get_count() const {return 0;} ///< Return the count of this scaler channel.
  virtual void set_count(const int a) {}

protected:
  SQScaler() {}
  
private:
  ClassDef(SQScaler, 1);
};

#endif /* _H_SQScaler_H_ */
