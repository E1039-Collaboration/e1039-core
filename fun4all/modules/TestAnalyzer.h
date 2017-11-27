/*
 * TestAnalyzer.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */

#ifndef _H_TestAnalyzer_H_
#define _H_TestAnalyzer_H_

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

class SQRun;
class SQSpillMap;

class SQEvent;
class SQHitMap;
class SQHitVector;

class TFile;
class TTree;

class TestAnalyzer: public SubsysReco {

public:

	TestAnalyzer(const std::string &name = "TestAnalyzer");
	virtual ~TestAnalyzer() {
	}

	int Init(PHCompositeNode *topNode);
	int InitRun(PHCompositeNode *topNode);
	int process_event(PHCompositeNode *topNode);
	int End(PHCompositeNode *topNode);

	int InitEvalTree();
	int ResetEvalVars();

	const std::string& get_hit_container_choice() const {
		return _hit_container_choice;
	}

	void set_hit_container_choice(const std::string& hitContainerChoice) {
		_hit_container_choice = hitContainerChoice;
	}

	const std::string& get_out_name() const {
		return _out_name;
	}

	void set_out_name(const std::string& outName) {
		_out_name = outName;
	}

private:

	int GetNodes(PHCompositeNode *topNode);

	std::string _hit_container_choice;

	size_t _event;

	SQRun* _run_header;
	SQSpillMap * _spill_map;

	SQEvent * _event_header;
	SQHitMap *_hit_map;
	SQHitVector *_hit_vector;

	std::string _out_name;
	TTree* _tout;

	int _b_run_id;
	int _b_spill_id;
	float _b_live_proton;
	int _b_event_id;
	int _b_hit_id;

	int _b_n_hits;
	float _b_drift_distance[10000];
};


#endif /* _H_TestAnalyzer_H_ */
