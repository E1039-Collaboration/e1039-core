#include "PatternDB.h"

// matching resolution 1: exact; 2: nearest neighbor; ...
#define _RESOLUTION_ 2
#define _RESOLUTION3_ 3

ClassImp(TrackletKey)
ClassImp(PatternDB)

const TrackletKey PatternDB::ERR_KEY = TrackletKey(0,0,0,0);

bool TrackletKey::operator == (const TrackletKey & k) const {
	if (St == k.St
			and abs (X - k.X) < _RESOLUTION_
			and abs (U - k.U) < _RESOLUTION_
			and abs (V - k.V) < _RESOLUTION_
			) return true;
	return false;
}

bool TrackletKey::operator != (const TrackletKey & k) const {
	if (St == k.St
			and abs (X - k.X) < _RESOLUTION_
			and abs (U - k.U) < _RESOLUTION_
			and abs (V - k.V) < _RESOLUTION_
			) return false;
	return true;
}

bool TrackletKey::operator < (const TrackletKey & k) const {
	if (St<k.St) return true;
	else if(St==k.St) {
		if(St==PatternDB::DC1 or St==PatternDB::DC2) {
			if(X<k.X-(_RESOLUTION_-1)) return true;
			else if(St==k.St and abs(X-k.X)<_RESOLUTION_ and U<k.U-(_RESOLUTION_-1)) return true;
			else if(St==k.St and abs(X-k.X)<_RESOLUTION_ and abs(U-k.U)<_RESOLUTION_ and V<k.V-(_RESOLUTION_-1)) return true;
		}	else if(St==PatternDB::DC3p or St==PatternDB::DC3m) {
			if(X<k.X-(_RESOLUTION3_-1)) return true;
			else if(St==k.St and abs(X-k.X)<_RESOLUTION3_ and U<k.U-(_RESOLUTION3_-1)) return true;
			else if(St==k.St and abs(X-k.X)<_RESOLUTION3_ and abs(U-k.U)<_RESOLUTION3_ and V<k.V-(_RESOLUTION3_-1)) return true;
		}
	}
	return false;
}

PatternDB::PatternDB()
{}

void PatternDB::identify(std::ostream& os) const {
	os
	<< "PatternDB::identify: "
	<< " St1 size: " << St1.size()
	<< " St2 size: " << St2.size()
	<< " St3 size: " << St3.size()
	<< " St23 size: " << St23.size()
	<< " St123 size: " << St123.size()
	<< std::endl;
}

int PatternDB::isValid() const {
	if (
			St1.size()>0 or
			St2.size()>0 or
			St3.size()>0 or
			St23.size()>0
			) return true;

	return false;
}

//void PatternDB::print(const TrackletKey &key)
//{
////	std::cout
////	<<"TrackletKey: "
////	<< " {"
////	<< ((key.k0>>24) & 255) << ", "
////	<< ((key.k0>>16) & 255) << ", "
////	<< ((key.k0>>8) & 255) << ", "
////	<< ((key.k0) & 255)
////	<< "} "
////	<< " {"
////	<< ((key.k1>>24) & 255) << ", "
////	<< ((key.k1>>16) & 255) << ", "
////	<< ((key.k1>>8) & 255) << ", "
////	<< ((key.k1) & 255)
////	<< "} "
////	<< " {" << key.k0 << ", " << key.k1 << "} "
////	<< std::endl;
//
//
//		std::cout
//		<<"TrackletKey: "
//		<< " {"
//		<< ((key.k0>>24) & 255) << ", "
//		<< ((key.k0>>16) & 255) << ", "
//		<< ((key.k0>>8) & 255) << ", "
//		<< ((key.k0) & 255)
//		<< "} "
//		<< " {"
//		<< ((key.k1>>24) & 255) << ", "
//		<< ((key.k1>>16) & 255) << ", "
//		<< ((key.k1>>8) & 255) << ", "
//		<< ((key.k1) & 255)
//		<< "} "
//		<< " {" << key.k0 << ", " << key.k1 << "} "
//		<< std::endl;
//}

//void PatternDB::print(const PartTrackKey &key)
//{
//	std::cout <<"PartTrackKey: " << std::endl;
//	PatternDB::print(key.k0);
//	PatternDB::print(key.k1);
//}

void PatternDB::print() {
	identify();
	int i=0;
	for(auto key : St123) {
		std::cout << key;
		++i;
		if(i>100) break;
	}
}
