/*
IO manager to handle fast extraction of data from database or upload data to database

Author: Kun Liu, liuk@fnal.gov
Created: 2013.9.29
*/

#ifndef _MYSQLSVC_H
#define _MYSQLSVC_H

#include <iostream>
#include <vector>
#include <string>
#include <list>
#include <algorithm>

#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TRandom.h>
#include <TClonesArray.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TTree.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "FastTracklet.h"
#include "TriggerAnalyzer.h"
#include "JobOptsSvc.h"

//#define OUT_TO_SCREEN
//#define USE_M_TABLES

class MySQLSvc
{
public:
    MySQLSvc();
    virtual ~MySQLSvc();
    static MySQLSvc* instance();

    //Connect to the input server
    bool connectInput( std::string mysqlServer = "", int mysqlPort = -1);
    bool connectOutput(std::string mysqlServer = "", int mysqlPort = -1);

    //Get direct pointer to the mysql connection
    TSQLServer* getInputServer() { return inputServer; }
    TSQLServer* getOutputServer() { return outputServer; }

    //Set username/password
    void setUserPasswd(std::string user_input, std::string passwd_input) { user = user_input; passwd = passwd_input; }

    //check if the run is stopped
    bool isRunStopped();

    //Get run info
    int getNEventsFast(int event_hi = -1, int event_lo = 0);
    int getNEvents(int event_hi = -1, int event_lo = 0);

    //Gets
    bool getEvent(SRawEvent* rawEvent, int eventID);
    bool getLatestEvt(SRawEvent* rawEvent);
    bool getRandomEvt(SRawEvent* rawEvent);
    bool getNextEvent(SRawEvent* rawEvent);
    bool getNextEvent(SRawMCEvent* rawEvent);

    //Check if the event has been loaded
    bool isEventLoaded(int eventID) { return std::find(eventIDs_loaded.begin(), eventIDs_loaded.end(), eventID) != eventIDs_loaded.end(); }

    //Get the event header
    bool getEventHeader(SRawEvent* rawEvent, int eventID);
    bool getMCGenInfo(SRawMCEvent* mcEvent, int eventID);

    //initialize reader -- check the indexing, table existence
    bool initReader();

    //Output to database/txt file/screen
    bool initWriter();
    bool initBakWriter();
    void resetWriter();       // reset the track and dimuon counter to 0
    void finalizeWriter();    // commit the remaining part of insertion queries

    void writeInfoTable(TTree* config);
    void writeTrackingRes(TString tableSuffix, SRecEvent* recEvent, TClonesArray* tracklets = NULL);
    void writeTrackTable(int trackID, SRecTrack* recTrack, TString tableSuffix);
    void writeTrackHitTable(int trackID, Tracklet* tracklet);
    void writeDimuonTable(int dimuonID, SRecDimuon dimuon, TString tableSuffix, int targetPos);
    void writeEventTable(int eventID, int statusCode, int source1, int source2, TString tableSuffix);

    //helper functions for creating tables
    /// Get the suffix to add to end of intermediate event subrange tables (if any)
    std::string getSubsetTableSuffix() const;
    /// Get WHERE clause to add to MySQL query to select the event subrange (if any)
    std::string getMySQLEventSelection() const;
    /// Get the definition of fields, keys, indices for a table (kTrack, kDimuon, or kTrackHit)
    std::string getTableDefinition(const TString tableType) const;
    /// Copy output from intermediate subset tables to final table and delete subset tables (if desired)
    void pushToFinalTables(bool dropSubsetTables);

    //Set the data schema
    void setInputSchema(std::string schema);
    void setOutputSchema(std::string schema)  { outputSchema = schema; }
    void setLoggingSchema(std::string schema) { logSchema = schema; }
    void enableQIE(bool opt) { readQIE = opt; }
    void enableTargetPos(bool opt) { readTargetPos = opt; }
    void enableTriggerHits(bool opt) { readTriggerHits = opt; }

    //Memory-safe sql queries
    int makeQueryInput();
    int makeQueryOutput();
    void commitInsertion(TString& insertion);
    bool nextEntry();

    int getInt(int id, int default_val = 0);
    float getFloat(int id, float default_val = 0.);
    double getDouble(int id, double default_val = 0.);
    std::string getString(int id, std::string default_val = "");

private:
    //pointer to the only instance
    static MySQLSvc* p_mysqlSvc;

    //Username and password
    std::string user;
    std::string passwd;

    //pointer to the geometry service
    GeomSvc* p_geomSvc;

    //pointer to trigger analyzer
    TriggerAnalyzer* p_triggerAna;

    //SQL server
    TSQLServer* inputServer;  ///< Fetch input from this server
    TSQLServer* outputServer; ///< Write output to this server
    TSQLResult* res;
    TSQLRow* row;

    //information about where to put output
    std::string subsetTableSuffix; ///< Append suffix to table names for intermediate event subset tables
    std::string subsetEventString; ///< MySQL string used to select only the events to be processed (e.g. WHERE caluse)

    //Test if QIE/TriggerHits table exists
    bool readQIE;
    bool readTriggerHits;
    bool readTargetPos;
    bool readTrackPos;
    bool setTriggerEmu;

    //Random generator
    TRandom rndm;

    //run-related info
    int runID;
    int spillID;
    int nEvents;

    //event list
    std::vector<int> eventIDs;
    std::vector<int> eventIDs_loaded;
    int index_eventID;

    //Query string used in all clause
    char query[2000];

    //name of the production schema working on
    std::string inputSchema;
    std::string outputSchema;
    std::string logSchema;

    //uploading query buffer
    TString eventQuery;
    TString dimuonQuery;
    TString trackQuery;
    TString hitQuery;

    //Internal counter of tracks and dimuons
    int nTracks;
    int nDimuons;
};

#endif
