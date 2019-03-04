/*
 * SRawEventEmbed.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */

#ifndef _H_SRawEventEmbed_H_
#define _H_SRawEventEmbed_H_

// ROOT
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>

// Fun4All includes
#include <fun4all/SubsysReco.h>

// STL includes
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <map>
//#include <algorithm>

#include <ktracker/SRawEvent.h>

class SQRun;
class SQSpillMap;

class SQEvent;
class SQHitMap;
class SQHitVector;

class PHG4TruthInfoContainer;

class SRecEvent;

class GeomSvc;

class TFile;
class TTree;

class SRawEventEmbed: public SubsysReco {

public:

	SRawEventEmbed(const std::string &name = "SRawEventEmbed");
	virtual ~SRawEventEmbed() {
	}

	int Init(PHCompositeNode *topNode);
	int InitRun(PHCompositeNode *topNode);
	int process_event(PHCompositeNode *topNode);
	int End(PHCompositeNode *topNode);

	int InitEvalTree();
	int ResetEvalVars();

	const std::string& get_hit_container_choice() const {
		return _hit_container_type;
	}

	void set_hit_container_choice(const std::string& hitContainerChoice) {
		_hit_container_type = hitContainerChoice;
	}

	const std::string& get_out_name() const {
		return _out_name;
	}

	void set_out_name(const std::string& outName) {
		_out_name = outName;
	}

	void print_noise_rate() const {
		for(auto i : _noise_rate) {
			std::cout << i.first << " -> " << i.second << std::endl;
		}
	}

	void set_noise_rate(const std::string & name, const double & rate) {
		_noise_rate[name] = rate;
	}

	const std::string& get_in_name() const {
		return _in_name;
	}

	void set_in_name(const std::string& inName) {
		_in_name = inName;
	}

	int get_trigger_bit() const {
		return _trigger_bit;
	}

	void set_trigger_bit(int triggerBit) {
		_trigger_bit = triggerBit;
	}

private:

	int GetNodes(PHCompositeNode *topNode);

	std::string _hit_container_type;
	std::map<std::string, double> _noise_rate;

	size_t _event;

	GeomSvc *p_geomSvc;

	SQHitMap *_hit_map;
	SQHitVector *_hit_vector;

	std::string _in_name;
	TTree* _tin;
	int _tree_entry;
	int _trigger_bit;
	SRawEvent* _srawevent;

	std::string _out_name;
	TTree* _tout;
};


#endif /* _H_SRawEventEmbed_H_ */
