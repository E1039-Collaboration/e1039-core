/*
 * MainDAQDecoder.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_MainDAQDecoder_H_
#define _H_MainDAQDecoder_H_

// ROOT

// Fun4All includes
#include <fun4all/SubsysReco.h>

// STL includes
#include <vector>
#include <string>
#include <iostream>
#include <list>

class Event;
class SQHitMap;

class MainDAQDecoder: public SubsysReco {

public:

	MainDAQDecoder(const std::string &name = "MainDAQDecoder");
	virtual ~MainDAQDecoder() {
	}

	int Init(PHCompositeNode *topNode);
	int InitRun(PHCompositeNode *topNode);
	int process_event(PHCompositeNode *topNode);
	int End(PHCompositeNode *topNode);

private:

	int MakeNodes(PHCompositeNode *topNode);
	int GetNodes(PHCompositeNode *topNode);

	Event *_evio_event;

  SQHitMap *_hit_map;

};


#endif /* _H_MainDAQDecoder_H_ */
