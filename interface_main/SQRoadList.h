#ifndef __SQ_ROAD_LIST_H__
#define __SQ_ROAD_LIST_H__
#include <iostream>
#include <phool/PHObject.h>

/// An SQ interface class to hold a list of trigger roads.
class SQRoadList : public PHObject {
public:
  typedef std::vector<int> RoadList_t;

  SQRoadList() {}
  virtual ~SQRoadList() {}

  virtual void identify(std::ostream& os=std::cout) const = 0;
  virtual void Reset() = 0;
  virtual int  isValid() const = 0;
  virtual SQRoadList* Clone() const = 0;

  virtual void        get_list(RoadList_t& pos_top, RoadList_t& pos_bot, RoadList_t& neg_top, RoadList_t& neg_bot) const = 0;
  virtual RoadList_t get_positive_top   () const = 0;
  virtual RoadList_t get_positive_bottom() const = 0;
  virtual RoadList_t get_negative_top   () const = 0;
  virtual RoadList_t get_negative_bottom() const = 0;

  virtual void set_list(RoadList_t& pos_top, RoadList_t& pos_bot, RoadList_t& neg_top, RoadList_t& neg_bot) = 0;
  virtual void set_positive_top   (const RoadList_t& pos_top) = 0;
  virtual void set_positive_bottom(const RoadList_t& pos_bot) = 0;
  virtual void set_negative_top   (const RoadList_t& neg_top) = 0;
  virtual void set_negative_bottom(const RoadList_t& neg_bot) = 0;

  virtual void print(std::ostream& os=std::cout) const = 0;
  
  ClassDef(SQRoadList, 1);
};

#endif // __SQ_ROAD_LIST_H__
