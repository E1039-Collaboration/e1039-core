#ifndef __SQ_ROAD_LIST_V1_H__
#define __SQ_ROAD_LIST_V1_H__
#include <phool/PHObject.h>
#include <iostream>
#include "SQRoadList.h"

/**
 * @code
 * PHNodeIterator iter(topNode);
 * PHCompositeNode* dst = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
 * if(!dst) {
 *   dst = new PHCompositeNode("DST");
 *   topNode->addNode(dst);
 * }
 * m_sq_road = findNode::getClass<SQRoadList>(topNode, "SQRoadList");
 * if(! m_sq_road) {
 *   m_sq_road = new SQRoadList_v1();
 *   dst->addNode(new PHIODataNode<PHObject>(m_sq_road, "SQRoadList", "PHObject"));
 * }
 * @endcode
 */
class SQRoadList_v1 : public SQRoadList {
 public:
  SQRoadList_v1();
  virtual ~SQRoadList_v1() {;}

  // PHObject
  void identify(std::ostream& os = std::cout) const;
  void Reset() {*this = SQRoadList_v1();}
  int  isValid() const;
  SQRoadList* Clone() const {return (new SQRoadList_v1(*this));}

  virtual void        get_list(RoadList_t& pos_top, RoadList_t& pos_bot, RoadList_t& neg_top, RoadList_t& neg_bot) const;
  virtual RoadList_t get_positive_top   () const { return m_pos_top;}
  virtual RoadList_t get_positive_bottom() const { return m_pos_bot;}
  virtual RoadList_t get_negative_top   () const { return m_neg_top;}
  virtual RoadList_t get_negative_bottom() const { return m_neg_bot;}

  virtual void set_list(RoadList_t& pos_top, RoadList_t& pos_bot, RoadList_t& neg_top, RoadList_t& neg_bot);
  virtual void set_positive_top   (const RoadList_t& pos_top) { m_pos_top = pos_top; }
  virtual void set_positive_bottom(const RoadList_t& pos_bot) { m_pos_bot = pos_bot; }
  virtual void set_negative_top   (const RoadList_t& neg_top) { m_neg_top = neg_top; }
  virtual void set_negative_bottom(const RoadList_t& neg_bot) { m_neg_bot = neg_bot; }

  virtual void print(std::ostream& os=std::cout) const;
  
 private:
  RoadList_t m_pos_top;
  RoadList_t m_pos_bot;
  RoadList_t m_neg_top;
  RoadList_t m_neg_bot;

  ClassDef(SQRoadList_v1, 1);
};


#endif // __SQ_ROAD_LIST_V1_H__
