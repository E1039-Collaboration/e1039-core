/**
 * \class PatternDBUtil
 * \brief PatternDB utilities
 * \author Haiwang Yu, yuhw@nmsu.edu
 *
 * Created: 08-27-2018
 */

#ifndef _PatternDBUtil_H
#define _PatternDBUtil_H

#include "PatternDB.h"
#include "FastTracklet.h"
//#include "SRawEvent.h"

#include <map>

class PatternDBUtil
{
	public:

	static int BuildPatternDB (const std::string & fin, const std::string & fout, PatternDB & db);

	static PatternDB* LoadPatternDB (const std::string & fin);

	static TrackletKey EncodeTrackletKey (
			PatternDB::STATION,
  		const unsigned int X, const unsigned int Xp,
			const unsigned int U, const unsigned int Up,
			const unsigned int V, const unsigned int Vp);

	static TrackletKey GetTrackletKey(
			const Tracklet tracklet,
			const PatternDB::STATION station);

	static TrackletKey GetTrackletKey(
			const std::vector< std::pair<unsigned int, unsigned int> > & det_elem_pairs,
			const PatternDB::STATION &station);

	static int LooseMode() {
		return PatternDBUtil::_loose_mode;
	}

	static void LooseMode(bool a = true) {
		PatternDBUtil::_loose_mode = a;
	}

	static int ResScaleDC1() {
		return PatternDBUtil::_RESOLUTION1_;
	}

	static void ResScaleDC1(const int a) {
		PatternDBUtil::_RESOLUTION1_ = a;
	}

	static int ResScaleDC2() {
		return PatternDBUtil::_RESOLUTION2_;
	}

	static void ResScaleDC2(const int a) {
		PatternDBUtil::_RESOLUTION2_ = a;
	}

	static int ResScaleDC3() {
		return PatternDBUtil::_RESOLUTION3_;
	}

	static void ResScaleDC3(const int a) {
		PatternDBUtil::_RESOLUTION3_ = a;
	}

	static int Verbosity() {
		return PatternDBUtil::verbosity;
	}

	static void Verbosity(int verbosity) {
		PatternDBUtil::verbosity = verbosity;
	}

	protected:

	PatternDBUtil(){}
	virtual ~PatternDBUtil(){}


	private:

	static std::map<unsigned int, unsigned int> _detid_view;

	static int verbosity;
	static bool _loose_mode;
	static int _RESOLUTION1_;
	static int _RESOLUTION2_;
	static int _RESOLUTION3_;
};


#endif /*_PatternDBUtil_H*/
