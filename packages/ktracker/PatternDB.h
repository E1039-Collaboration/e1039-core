/*
 * PatternDB.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_PatternDB_H_
#define _H_PatternDB_H_

#include <phool/PHObject.h>

#include <iostream>
#include <limits>
#include <string>
#include <set>
//#include <tuple>

class TrackletKey : public TObject{
public:
	TrackletKey() : k0(0), k1(0) {}
	TrackletKey(const unsigned int i0, const unsigned int i1) : k0(i0), k1(i1){}
	unsigned int k0;
	unsigned int k1;

	bool operator == (const TrackletKey & k) const {if (k0==k.k0 and k1==k.k1) return true; return false;}
	bool operator != (const TrackletKey & k) const {if (k0==k.k0 and k1==k.k1) return false; return true;}
	bool operator < (const TrackletKey & k) const {
		if (k0<k.k0) return true;
		else if(k0==k.k0 and k1<k.k1) return true;
		return false;
	}

	ClassDef(TrackletKey, 1);
};

class PartTrackKey : public TObject{
public:
	PartTrackKey(): k0 (TrackletKey()), k1(TrackletKey()) {}
	PartTrackKey(const TrackletKey & i0, const TrackletKey & i1) : k0(i0), k1(i1) {}
	TrackletKey k0;
	TrackletKey k1;

	bool operator == (const PartTrackKey & k) const {if (k0==k.k0 and k1==k.k1) return true; return false;}
	bool operator != (const PartTrackKey & k) const {if (k0==k.k0 and k1==k.k1) return false; return true;}
	bool operator < (const PartTrackKey & k) const {
		if (k0<k.k0) return true;
		else if(k0==k.k0 and k1<k.k1) return true;
		return false;
	}

	ClassDef(PartTrackKey, 1);
};

class PatternDB : public PHObject {

public:

  //typedef std::tuple<unsigned int, unsigned int> TrackletKey;
  //typedef std::tuple<TrackletKey, TrackletKey> PartTrackKey;


  const static TrackletKey ERR_KEY;

  enum STATION { DC1, DC2, DC3p, DC3m};

	PatternDB();
  virtual ~PatternDB() {}

  // PHObject virtual overloads
  virtual void              identify(std::ostream& os = std::cout) const;
  virtual void              Reset() {*this = PatternDB();};
  virtual int               isValid() const;
  virtual PatternDB*        Clone() const {return (new PatternDB(*this));}

  static void print(const TrackletKey &k);
  static void print(const PartTrackKey &k);

  void print();

  std::set<TrackletKey>     St2;
  std::set<TrackletKey>     St3;
  std::set<PartTrackKey>    St23;

  ClassDef(PatternDB, 1);
};




#endif /* _H_PatternDB_H_ */
