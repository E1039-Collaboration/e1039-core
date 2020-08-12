#ifndef PHG4HITDEFS_H
#define PHG4HITDEFS_H

#include <string>

namespace PHG4HitDefs
{
  typedef unsigned int keytype;
  static const unsigned int keybits = 16;
  static const unsigned int hit_idbits = sizeof(keytype)*8-keybits;
  static const keytype detid_mask = 0xffff0000;
  static const keytype hitid_mask = 0x0000ffff;

  //! convert PHG4HitContainer node names in to ID number for the container.
  //! used in indexing volume ID in PHG4Shower
  int get_volume_id(const std::string & nodename);

}

#endif


