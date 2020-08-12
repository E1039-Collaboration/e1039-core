#ifndef PHG4HITCONTAINER_H__
#define PHG4HITCONTAINER_H__

#include "PHG4HitDefs.h"

#include <phool/PHObject.h>
#include <map>
#include <set>
#include <string>
class PHG4Hit;

class PHG4HitContainer: public PHObject
{

  public:
  typedef std::map<PHG4HitDefs::keytype, PHG4Hit *> Map;
  typedef Map::iterator Iterator;
  typedef Map::const_iterator ConstIterator;
  typedef std::pair<Iterator, Iterator> Range;
  typedef std::pair<ConstIterator, ConstIterator> ConstRange;
  typedef std::set<unsigned int>::const_iterator LayerIter;

  PHG4HitContainer(); //< used only by ROOT for DST readback
  PHG4HitContainer(const std::string &nodename);

  virtual ~PHG4HitContainer() {}

  void Reset();

  void identify(std::ostream& os = std::cout) const;

  //! container ID should follow definition of PHG4HitDefs::get_volume_id(DST nodename)
  void SetID(int i) {id = i;}
  int GetID() const {return id;}
  
  ConstIterator AddHit(PHG4Hit *newhit);

  ConstIterator AddHit(const unsigned int detid, PHG4Hit *newhit);
  
  Iterator findOrAddHit(PHG4HitDefs::keytype key);

  PHG4Hit* findHit(PHG4HitDefs::keytype key );

  PHG4HitDefs::keytype genkey(const unsigned int detid);

  //! return all hits matching a given detid
  ConstRange getHits(const unsigned int detid) const;

  //! return all hist
  ConstRange getHits( void ) const;

  unsigned int size( void ) const
  { return hitmap.size(); }
  unsigned int num_layers(void) const
  { return layers.size(); }
  std::pair<LayerIter, LayerIter> getLayers() const
     { return make_pair(layers.begin(), layers.end());} 
  void AddLayer(const unsigned int ilayer);
  void RemoveZeroEDep();
  PHG4HitDefs::keytype getmaxkey(const unsigned int detid);

  void registerHitOnLayer(const PHG4Hit* hit);

 protected:

  int id; //< unique identifier from hash of node name. Defined following PHG4HitDefs::get_volume_id
  Map hitmap;
  std::set<unsigned int> layers; // layers is not reset since layers must not change event by event
  std::map<unsigned int, PHG4HitDefs::keytype> layerMaxID;

  ClassDef(PHG4HitContainer,2)
};

#endif
