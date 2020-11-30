#ifndef _SQ_TRACK_VECTOR__H_
#define _SQ_TRACK_VECTOR__H_
#include <iostream>
#include <vector>
#include <phool/PHObject.h>
#include "SQTrack.h"

/// An SQ interface class to hold a list of SQTrack objects.
/**
 * Below is the standard way to find this object in "topNode" and loop over each track;
 * @code
 * SQTrackVector* trk_vec = findNode::getClass<SQTrackVector>(topNode, "SQTrackVector");
 * for (SQTrackVector::ConstIter it = trk_vec->begin(); it != trk_vec->end(); it++) {
 *   SQTrack* trk = *it;
 *   int trk_id = trk->get_track_id();
 * }
 * @endcode
 */
class SQTrackVector: public PHObject {
 public:
  typedef std::vector<SQTrack*>                    Vector;
  typedef std::vector<SQTrack*>::const_iterator ConstIter;
  typedef std::vector<SQTrack*>::iterator            Iter;

  virtual ~SQTrackVector() {}

  virtual void identify(std::ostream& os = std::cout) const = 0;
  virtual void Reset() = 0;
  virtual int  isValid() const = 0;
  virtual SQTrackVector* Clone() const = 0;

  virtual ConstIter begin() const = 0;
  virtual ConstIter end  () const = 0;
  virtual      Iter begin() = 0;
  virtual      Iter end  () = 0;
  virtual bool      empty() const = 0;
  virtual size_t    size () const = 0;
  virtual void      clear() = 0;

  virtual const SQTrack* at(const size_t id) const = 0;
  virtual       SQTrack* at(const size_t id) = 0;
  virtual       void   push_back(const SQTrack *trk) = 0;
  virtual       size_t erase(const size_t id) = 0;

 protected:
  SQTrackVector() {}

  ClassDef(SQTrackVector, 1);
};

#endif // _SQ_TRACK_VECTOR__H_
