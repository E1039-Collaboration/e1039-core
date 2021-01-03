#ifndef _SQ_DIMUON_VECTOR__H_
#define _SQ_DIMUON_VECTOR__H_
#include <iostream>
#include <vector>
#include <phool/PHObject.h>
#include "SQDimuon.h"

/// An SQ interface class to hold a list of SQDimuon objects.
/**
 * Below is the standard way to find this object in "topNode" and loop over each dimuon;
 * @code
 * SQDimuonVector* dim_vec = findNode::getClass<SQDimuonVector>(topNode, "SQDimuonVector");
 * for (SQDimuonVector::ConstIter it = dim_vec->begin(); it != dim_vec->end(); it++) {
 *   SQDimuon* dim = *it;
 *   int dim_id = dim->get_dimuon_id();
 * }
 * @endcode
 */
class SQDimuonVector: public PHObject {
 public:
  typedef std::vector<SQDimuon*>                    Vector;
  typedef std::vector<SQDimuon*>::const_iterator ConstIter;
  typedef std::vector<SQDimuon*>::iterator            Iter;

  virtual ~SQDimuonVector() {}

  virtual void identify(std::ostream& os = std::cout) const = 0;
  virtual void Reset() = 0;
  virtual int  isValid() const = 0;
  virtual SQDimuonVector* Clone() const = 0;

  virtual ConstIter begin() const = 0;
  virtual ConstIter end  () const = 0;
  virtual      Iter begin() = 0;
  virtual      Iter end  () = 0;
  virtual bool      empty() const = 0;
  virtual size_t    size () const = 0;
  virtual void      clear() = 0;

  virtual const SQDimuon* at(const size_t id) const = 0;
  virtual       SQDimuon* at(const size_t id) = 0;
  virtual       void   push_back(const SQDimuon *dim) = 0;
  virtual       size_t erase(const size_t id) = 0;

 protected:
  SQDimuonVector() {}

  ClassDef(SQDimuonVector, 1);
};

#endif // _SQ_DIMUON_VECTOR__H_
