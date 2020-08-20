#ifndef _H_SQCalHit_v1_H_
#define _H_SQCalHit_v1_H_


#include <phool/PHObject.h>
#include <iostream>
#include <map>

#include "SQHit.h"

class SQCalHit_v1 : public SQHit {

public:

  SQCalHit_v1();
  virtual ~SQCalHit_v1() {}

  // PHObject virtual overloads
  void         identify(std::ostream& os = std::cout) const;
  void         Reset() {*this = SQCalHit_v1();}
  int          isValid() const;
  SQHit*       Clone() const {return (new SQCalHit_v1(*this));}

  // digitized hit info
  virtual int          get_hit_id() const                        {return _hit_id;}
  virtual void         set_hit_id(const int a)                   {_hit_id = a;}

  virtual short        get_detector_id() const                   {return _detector_id;}
  virtual void         set_detector_id(const short a)            {_detector_id = a;}

  virtual short        get_element_id() const                    {return _element_id;}
  virtual void         set_element_id(const short id)            {_element_id = id;}

  virtual short        get_tower_id() const                      {return _element_id;}
  virtual void         set_tower_id(const short id)              {_element_id = id;}

  virtual float        get_edep() const                          {return _edep;}
  virtual void         set_edep(const float a)                   {_edep = a;}

  virtual unsigned int  get_n_cells() const                      {return _cells.size();}
  virtual CellMap       get_cells() const                        {return _cells;}
  virtual float         get_cell(short i) const;
  virtual void          add_cell(short i, float edep);

private:

  int   _hit_id;                 ///< hitID
  short _detector_id;            ///< mapping from detector name to ID
  short _element_id;             ///< elementID

  float _edep;

  CellMap _cells;

  ClassDef(SQCalHit_v1, 1);
};


#endif /* _H_SQCalHit_v1_H_ */