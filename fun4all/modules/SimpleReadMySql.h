/*
 * SimpleReadMySql.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SimpleReadMySql_H_
#define _H_SimpleReadMySql_H_

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
//#include <algorithm>

class SHitMap;

class SimpleReadMySql: public SubsysReco {

public:

	SimpleReadMySql(const std::string &name = "SimpleReadMySql");
	virtual ~SimpleReadMySql() {
	}

	int Init(PHCompositeNode *topNode);
	int InitRun(PHCompositeNode *topNode);
	int process_event(PHCompositeNode *topNode);
	int End(PHCompositeNode *topNode);

private:

	int MakeNodes(PHCompositeNode *topNode);

	int makeQueryInput();
	bool nextEntry();

    int getInt(int id, int default_val = 0);
    float getFloat(int id, float default_val = 0.);
    double getDouble(int id, double default_val = 0.);
    std::string getString(int id, std::string default_val = "");


	std::string _server_name;
	int _port;
	std::string _user_name;
	std::string _password;
	std::string _input_shcema;
	std::string _table_name;
	int _run_id;
	int _spill_id;

    //Query string used in all clause
    char query[2000];

    //SQL server
    TSQLServer* inputServer;  //< Fetch input from this server
    TSQLResult* res;
    TSQLRow* row;

    std::vector<int> eventIDs;
    size_t _event;

    SHitMap *_hit_map;

};


#endif /* _H_SimpleReadMySql_H_ */
