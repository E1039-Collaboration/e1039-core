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

	friend ostream & operator << (ostream &out, const TrackletKey &key) {
		std::cout
		<<"TrackletKey: "
		<< " {"
		<< ((key.k0>>24) & 255) << ", "
		<< ((key.k0>>16) & 255) << ", "
		<< ((key.k0>>8) & 255) << ", "
		<< ((key.k0) & 255)
		<< "} "
		<< " {"
		<< ((key.k1>>24) & 255) << ", "
		<< ((key.k1>>16) & 255) << ", "
		<< ((key.k1>>8) & 255) << ", "
		<< ((key.k1) & 255)
		<< "} "
		<< " {" << key.k0 << ", " << key.k1 << "} "
		<< std::endl;

		return out;
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

	friend ostream & operator << (ostream &out, const PartTrackKey &key) {
		std::cout
		<<"PartTrackKey: " << std::endl
		<< key.k0
		<< key.k1;

		return out;
	}

	ClassDef(PartTrackKey, 1);
};

class GlobTrackKey : public TObject{
public:
	GlobTrackKey(): k0 (TrackletKey()), k1(TrackletKey()), k2(TrackletKey()) {}
	GlobTrackKey(const TrackletKey & i0, const TrackletKey & i1, const TrackletKey & i2) : k0(i0), k1(i1), k2(i2) {}
	TrackletKey k0;
	TrackletKey k1;
	TrackletKey k2;

	bool operator == (const GlobTrackKey & k) const {if (k0==k.k0 and k1==k.k1 and k2==k.k2) return true; return false;}
	bool operator != (const GlobTrackKey & k) const {if (k0==k.k0 and k1==k.k1 and k2==k.k2) return false; return true;}
	bool operator < (const GlobTrackKey & k) const {
		if (k0<k.k0) return true;
		else if(k0==k.k0 and k1<k.k1) return true;
		else if(k0==k.k0 and k1==k.k1 and k2<k.k2) return true;
		return false;
	}

	friend ostream & operator << (ostream &out, const GlobTrackKey &key) {
		std::cout
		<<"GlobTrackKey: " << std::endl
		<< key.k0
		<< key.k1
		<< key.k2;

		return out;
	}

	ClassDef(GlobTrackKey, 1);
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

  //static void print(const TrackletKey &k);
  //static void print(const PartTrackKey &k);

  void print();

  std::set<TrackletKey>     St1;
  std::set<TrackletKey>     St2;
  std::set<TrackletKey>     St3;
  std::set<PartTrackKey>    St23;
  std::set<GlobTrackKey>    St123;

  ClassDef(PatternDB, 1);
};




#endif /* _H_PatternDB_H_ */
