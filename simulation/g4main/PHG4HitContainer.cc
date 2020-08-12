#include "PHG4HitContainer.h"
#include "PHG4Hit.h"
#include "PHG4Hitv1.h"

#include <phool/phool.h>

#include <TSystem.h>

#include <cstdlib>

using namespace std;

PHG4HitContainer::PHG4HitContainer()
  : id(-1), hitmap(), layers(), layerMaxID()
{
}

PHG4HitContainer::PHG4HitContainer(const std::string &nodename)
  : id(PHG4HitDefs::get_volume_id(nodename)), hitmap(), layers(), layerMaxID()
{
}

void
PHG4HitContainer::Reset()
{
  while(hitmap.begin() != hitmap.end())
  {
    delete hitmap.begin()->second;
    hitmap.erase(hitmap.begin());
  }

  for(auto iter = layerMaxID.begin(); iter != layerMaxID.end(); ++iter)
  {
    iter->second = 0;
  }
  return;
}

void
PHG4HitContainer::identify(ostream& os) const
{
   ConstIterator iter;
   os << "Number of hits: " << size() << endl;
   for (iter = hitmap.begin(); iter != hitmap.end(); ++iter)
     {
       os << "hit key 0x" << hex << iter->first << dec << endl;
       (iter->second)->identify();
     }
   set<unsigned int>::const_iterator siter;
   os << "Number of layers: " << num_layers() << endl;
   for (siter = layers.begin(); siter != layers.end(); ++siter)
     {
       os << "layer : " << *siter << endl;
     }
  return;
}

PHG4HitDefs::keytype
PHG4HitContainer::getmaxkey(const unsigned int detid)
{
  return layerMaxID[detid];
}

PHG4HitDefs::keytype
PHG4HitContainer::genkey(const unsigned int detid)
{
  PHG4HitDefs::keytype detidlong = detid;
  if((detidlong >> PHG4HitDefs::keybits) > 0)
  {
    cout << PHWHERE << " detector id too large: " << detid << endl;
    exit(EXIT_FAILURE);
  }
  PHG4HitDefs::keytype shiftval = detidlong << PHG4HitDefs::hit_idbits;
  //  cout << "max index: " << (detminmax->second)->first << endl;
  // after removing hits with no energy deposition, we have holes
  // in our hit ranges. This construct will get us the last hit in
  // a layer and return it's hit id. Adding 1 will put us at the end of this layer
  PHG4HitDefs::keytype hitid  = getmaxkey(detid) + 1;
  PHG4HitDefs::keytype newkey = hitid | shiftval;
  if(hitmap.find(newkey) != hitmap.end())
  {
    cout << PHWHERE << " duplicate key: 0x" 
         << hex << newkey << dec 
         << " for detector " << detid 
         << " hitmap.size: " << hitmap.size()
         << " hitid: " << hitid << " exiting now" << endl;
    exit(EXIT_FAILURE);
  }
  return newkey;
}

PHG4HitContainer::ConstIterator
PHG4HitContainer::AddHit(PHG4Hit *newhit)
{
  PHG4HitDefs::keytype key = newhit->get_hit_id();
  if (hitmap.find(key) != hitmap.end())
    {
      cout << "hit with id  0x" << hex << key << dec << " exists already" << endl;
      return hitmap.find(key);
    }

  registerHitOnLayer(newhit);
  hitmap[key] = newhit;
  return hitmap.find(key);
}

PHG4HitContainer::ConstIterator
PHG4HitContainer::AddHit(const unsigned int detid, PHG4Hit *newhit)
{
  PHG4HitDefs::keytype key = genkey(detid);
  newhit->set_hit_id(key);
  registerHitOnLayer(newhit);
  hitmap[key] = newhit;
  return hitmap.find(key);
}

PHG4HitContainer::ConstRange PHG4HitContainer::getHits(const unsigned int detid) const
{
  PHG4HitDefs::keytype detidlong = detid;
  if ((detidlong >> PHG4HitDefs::keybits) > 0)
    {
      cout << " detector id too large: " << detid << endl;
      exit(1);
    }
  PHG4HitDefs::keytype keylow = detidlong << PHG4HitDefs::hit_idbits;
  PHG4HitDefs::keytype keyup = ((detidlong + 1) << PHG4HitDefs::hit_idbits) -1 ;
  ConstRange retpair;
  retpair.first = hitmap.lower_bound(keylow);
  retpair.second = hitmap.upper_bound(keyup);
  return retpair;
}

PHG4HitContainer::ConstRange PHG4HitContainer::getHits( void ) const
{ return std::make_pair( hitmap.begin(), hitmap.end() ); }


PHG4HitContainer::Iterator PHG4HitContainer::findOrAddHit(PHG4HitDefs::keytype key)
{
  PHG4HitContainer::Iterator it = hitmap.find(key);
  if(it == hitmap.end())
  {
    hitmap[key] = new PHG4Hitv1();
    it = hitmap.find(key);
    PHG4Hit* mhit = it->second;
    mhit->set_hit_id(key);
    mhit->set_edep(0.);
    registerHitOnLayer(mhit);
  }
  return it;
}

PHG4Hit* PHG4HitContainer::findHit(PHG4HitDefs::keytype key)
{
  PHG4HitContainer::ConstIterator it = hitmap.find(key);
  if(it != hitmap.end())
    {
      return it->second;
    }
    
  return NULL;
}

void PHG4HitContainer::RemoveZeroEDep()
{
  //  unsigned int hitsbef = hitmap.size();
  Iterator itr = hitmap.begin();
  Iterator last = hitmap.end();
  for (; itr != last; )
    {
      PHG4Hit *hit = itr->second;
      if (hit->get_edep() == 0)
        {
          delete hit;
          hitmap.erase(itr++);
        }
      else
        {
          ++itr;
        }
    }
//   unsigned int hitsafter = hitmap.size();
//   cout << "hist before: " << hitsbef
//        << ", hits after: " << hitsafter << endl;
  return;
}

void PHG4HitContainer::AddLayer(const unsigned int ilayer)
{
  layers.insert(ilayer);
  layerMaxID[ilayer] = 0;
}

void PHG4HitContainer::registerHitOnLayer(const PHG4Hit* hit)
{
  PHG4HitDefs::keytype hitkey = hit->get_hit_id();
  unsigned int detid = (hitkey & PHG4HitDefs::detid_mask) >> PHG4HitDefs::hit_idbits;
  if(layerMaxID.find(detid) == layerMaxID.end())
  {
    AddLayer(detid);
  }

  PHG4HitDefs::keytype hitid = hitkey & PHG4HitDefs::hitid_mask;
  if(layerMaxID[detid] < hitid)
  {
    layerMaxID[detid] = hitid;
  }
}

