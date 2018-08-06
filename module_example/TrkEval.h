/*
 * TrkEval.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */

#ifndef _H_TrkEval_H_
#define _H_TrkEval_H_

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

class PHG4TruthInfoContainer;

class SRecEvent;

class GeomSvc;

class TFile;
class TTree;

class TrkEval: public SubsysReco {

public:

	TrkEval(const std::string &name = "TrkEval");
	virtual ~TrkEval() {
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

private:

	int GetNodes(PHCompositeNode *topNode);

	std::string _hit_container_type;

	size_t _event;

	SQRun* _run_header;
	SQSpillMap * _spill_map;

	SQEvent * _event_header;
	SQHitMap *_hit_map;
	SQHitVector *_hit_vector;

	PHG4TruthInfoContainer* _truth;

	SRecEvent* _recEvent;

	std::string _out_name;
	TTree* _tout;

	int _b_run_id;
	int _b_spill_id;
	float _b_target_pos;
	int _b_event_id;

	int _b_n_hits;
	int _b_hit_id[10000];
	int _b_detector_id[10000];
	float _b_drift_distance[10000];
	float _b_pos[10000];
	float _b_detector_z[10000];

	float _b_truth_x[10000];
	float _b_truth_y[10000];
	float _b_truth_z[10000];
	float _b_truth_pos[10000];

	int n_particles;
	float gvx[1000];
	float gvy[1000];
	float gvz[1000];
	float gpx[1000];
	float gpy[1000];
	float gpz[1000];
	float gpt[1000];
	float geta[1000];
	float gphi[1000];
	int gnhits[1000];
	int gndc[1000];
	int gnhodo[1000];
	int gnprop[1000];

	int ntruhits[1000];
	float vx[1000];
	float vy[1000];
	float vz[1000];
	float px[1000];
	float py[1000];
	float pz[1000];
	float pt[1000];
	float eta[1000];
	float phi[1000];

	int gndimu;
	float dimu_gpx[100];
	float dimu_gpy[100];
	float dimu_gpz[100];
	float dimu_gpt[100];
	float dimu_geta[100];
	float dimu_gphi[100];

	float dimu_px[100];
	float dimu_py[100];
	float dimu_pz[100];
	float dimu_pt[100];
	float dimu_eta[100];
	float dimu_phi[100];




	GeomSvc *p_geomSvc;
};


#endif /* _H_TrkEval_H_ */
