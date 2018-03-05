/*
 * ReadMySql.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_ReadMySql_H_
#define _H_ReadMySql_H_

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

class GeomSvc;

class SQRun;
class SQSpillMap;

class SQEvent;
class SQHitMap;
class SQHitVector;

class ReadMySql: public SubsysReco {

public:

	ReadMySql(const std::string &name = "ReadMySql");
	virtual ~ReadMySql() {
	}

	int Init(PHCompositeNode *topNode);
	int InitRun(PHCompositeNode *topNode);
	int process_event(PHCompositeNode *topNode);
	int End(PHCompositeNode *topNode);

	int FillSQRun(SQRun* run_header, TSQLServer* server, const int run_id, const char* table = "Run");
	int FillSQSpill(SQSpillMap* spill_map, TSQLServer* server, const int run_id, const char* table = "Spill");

	int FillSQEvent(SQEvent* event_header, TSQLServer* server, const int event_id, const char* table = "Event");
	int FillSQHitMap(SQHitMap* hit_map, TSQLServer* server, const int event_id, const char* table = "Hit");
	int FillSQHitVector(SQHitVector* hit_vector, const std::string hit_type, TSQLServer* server, const int event_id, const char* table = "Hit");

	int FillSQTriggerHitVector(SQHitVector* hit_vector, const std::string hit_type, TSQLServer* server, const int event_id, const char* table = "TriggerHit");

	static int getInt(TSQLRow* row, int id, int default_val = 0);
	static float getFloat(TSQLRow* row, int id, float default_val = 0.);
	static double getDouble(TSQLRow* row, int id, double default_val = 0.);
	static std::string getString(TSQLRow* row, int id, std::string default_val = "");

	const std::string& get_hit_container_type() const {
		return _hit_container_type;
	}

	void set_hit_container_type(const std::string& hitContainerChoice) {
		_hit_container_type = hitContainerChoice;
	}

	const std::string& get_hit_type() const {
		return _hit_type;
	}

	void set_hit_type(const std::string& hitType) {
		_hit_type = hitType;
	}

private:

	int MakeNodes(PHCompositeNode *topNode);
	int GetNodes(PHCompositeNode *topNode);

	int makeQueryInput();
	bool nextEntry();

	std::string _hit_type;
	std::string _hit_container_type;

	std::string _server_name;
	int _port;
	std::string _user_name;
	std::string _password;
	std::string _input_shcema;
	std::string _table_name;
	int _run_id;
	int _spill_id;

	//Query string used in all clause
	char _query[2000];

	//SQL server
	TSQLServer* _input_server;  //< Fetch input from this server
	TSQLResult* _res;
	TSQLRow* _row;

	//GeomSvc
	GeomSvc *p_geomSvc;

	std::vector<int> _event_ids;
	size_t _event;

	SQRun* _run_header;
	SQSpillMap * _spill_map;

	SQEvent * _event_header;
	SQHitMap *_hit_map;
	SQHitVector *_hit_vector;
	SQHitVector *_triggerhit_vector;

	typedef std::map<std::string, short> _m_detector_name_to_id;

};


#endif /* _H_ReadMySql_H_ */
