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
//#include <TRandom.h>
//#include <TClonesArray.h>
//#include <TVector3.h>
//#include <TLorentzVector.h>
//#include <TTree.h>

// Fun4All includes
#include <fun4all/SubsysReco.h>

// STL includes
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <map>
//#include <algorithm>

class SHitMap;

class ReadMySql: public SubsysReco {

public:

	ReadMySql(const std::string &name = "ReadMySql");
	virtual ~ReadMySql() {
	}

	int Init(PHCompositeNode *topNode);
	int InitRun(PHCompositeNode *topNode);
	int process_event(PHCompositeNode *topNode);
	int End(PHCompositeNode *topNode);

	static int FillHitMap(SHitMap* hitmap, TSQLServer* server, const int event_id, const char* source = "");

	static int getInt(TSQLRow* row, int id, int default_val = 0);
	static float getFloat(TSQLRow* row, int id, float default_val = 0.);
	static double getDouble(TSQLRow* row, int id, double default_val = 0.);
	static std::string getString(TSQLRow* row, int id, std::string default_val = "");

private:

	int MakeNodes(PHCompositeNode *topNode);
	int GetNodes(PHCompositeNode *topNode);

	int makeQueryInput();
	bool nextEntry();



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

    std::vector<int> _event_ids;
    size_t _event;

    SHitMap *_hit_map;

    typedef std::map<std::string, short> _m_detector_name_to_id;

};


#endif /* _H_ReadMySql_H_ */
