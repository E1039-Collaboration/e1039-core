/*
 * DetectorConstructionWrapper.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */

#ifndef _H_DetectorConstructionWrapper_H_
#define _H_DetectorConstructionWrapper_H_

#include <jobopts_svc/JobOptsSvc.h>

#include <fun4all/SubsysReco.h>

#include <string>

class DetectorConstructionWrapper: public SubsysReco {

public:

	DetectorConstructionWrapper(const std::string &name = "DetectorConstructionWrapper");
	virtual ~DetectorConstructionWrapper() {
	}

	int Init(PHCompositeNode *topNode);
	int InitRun(PHCompositeNode *topNode) {};
	int process_event(PHCompositeNode *topNode) {};
	int End(PHCompositeNode *topNode);

	const std::string& get_out_name() const {
		return _out_name;
	}

	void set_out_name(const std::string& outName) {
		_out_name = outName;
	}

private:

	std::string _geometrySchema;

	double _fMagStr;
	double _kMagStr;

	std::string _out_name;

	JobOptsSvc* _jobOptsSvc;
};


#endif /* _H_DetectorConstructionWrapper_H_ */
