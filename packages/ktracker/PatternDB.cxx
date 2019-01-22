#include "PatternDB.h"

ClassImp(PatternDB)

//const TrackletKey PatternDB::ERR_KEY = std::make_tuple(0,0);
const TrackletKey PatternDB::ERR_KEY = TrackletKey(0,0);

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
