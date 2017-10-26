/*
MySQLSvc.cxx

Implementation of class MySQLSvc.

Author: Kun Liu, liuk@fnal.gov
Created: 9-29-2013
*/

#include <cstring>
#include <cmath>
#include <cfloat>
#include <stdexcept>
#include <boost/lexical_cast.hpp>

#include <TLorentzVector.h>
#include <TObjArray.h>
#include <TLeaf.h>

#include "FastTracklet.h"
#include "MySQLSvc.h"

#define MaxQueryLen 50000

MySQLSvc* MySQLSvc::p_mysqlSvc = NULL;

MySQLSvc::MySQLSvc()
{
    runID = -1;
    spillID = -1;
    eventIDs.clear();

    inputServer = NULL;
    outputServer = NULL;
    res = NULL;
    row = NULL;
    p_triggerAna = NULL;

    p_geomSvc = GeomSvc::instance();

    JobOptsSvc* jobOptsSvc = JobOptsSvc::instance();

    inputSchema  = jobOptsSvc->m_inputSchema;
    outputSchema = jobOptsSvc->m_outputSchema;
    logSchema = "log";

    user = "seaguest";
    passwd = "qqbar2mu+mu-";

    nEvents = 0;
    nTracks = 0;
    nDimuons = 0;

    readQIE = true;
    readTriggerHits = true;
    readTargetPos = true;
    readTrackPos = true;

    subsetTableSuffix = getSubsetTableSuffix();
    subsetEventString = getMySQLEventSelection();

    rndm.SetSeed(0);
}

MySQLSvc::~MySQLSvc()
{
    if(inputServer != NULL) delete inputServer;
    if(outputServer != NULL) delete outputServer;
    if(res != NULL) delete res;
    if(row != NULL) delete row;
    if(p_triggerAna != NULL) delete p_triggerAna;
}

MySQLSvc* MySQLSvc::instance()
{
    if(p_mysqlSvc == NULL)
    {
        p_mysqlSvc = new MySQLSvc();
    }

    return p_mysqlSvc;
}

bool MySQLSvc::connectInput(std::string mysqlServer, int mysqlPort)
{
    if(inputServer)
    {
        delete inputServer;
        inputServer = NULL;
    }

    if(mysqlPort < 0)
    {
        JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();
        inputServer = TSQLServer::Connect(p_jobOptsSvc->GetInputMySQLURL().c_str(), user.c_str(), passwd.c_str());
    }
    else
    {
        char serverUrl[200];
        sprintf(serverUrl, "mysql://%s:%d", mysqlServer.c_str(), mysqlPort);
        inputServer = TSQLServer::Connect(serverUrl, user.c_str(), passwd.c_str());
    }

    if(inputServer == NULL)
    {
        LogInfo("Connection to database failed!");
        return false;
    }
    return true;
}

bool MySQLSvc::connectOutput(std::string mysqlServer, int mysqlPort)
{
    if(outputServer)
    {
        delete outputServer;
        inputServer = NULL;
    }

    int wait_count = 0;
    const int WAIT_TIME = 60;
    const int MAX_WAIT_COUNT = 60;
    while(outputServer == NULL)
    {
        if(mysqlPort < 0)
        {
            JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();
            outputServer = TSQLServer::Connect(p_jobOptsSvc->GetOutputMySQLURL().c_str(), user.c_str(), passwd.c_str());
        }
        else
        {
            char serverUrl[200];
            sprintf(serverUrl, "mysql://%s:%d", mysqlServer.c_str(), mysqlPort);
            outputServer = TSQLServer::Connect(serverUrl, user.c_str(), passwd.c_str());
        }

        if(outputServer == NULL)
        {
            //wait a minute and try again
            sleep(WAIT_TIME);
            ++wait_count;
        }

        if(wait_count == MAX_WAIT_COUNT)
        {
            LogInfo("Connection to database failed!");
            return false;
        }
    }
    return true;
}


void MySQLSvc::setInputSchema(std::string schema)
{
    inputSchema = schema;
    eventIDs.clear();
    index_eventID = 0;
}

bool MySQLSvc::initReader()
{
    //check the essential info
    sprintf(query, "USE %s", inputSchema.c_str());
    if(!inputServer->Exec(query))
    {
        std::cout << "MySQLSvc: working schema does not exist! Will exit..." << std::endl;
        return false;
    }

    if(!(inputServer->HasTable("Hit") && inputServer->HasTable("Spill")))
    {
        std::cout << "MySQLSvc: essential information is missing in this schema. Will exit..." << std::endl;
        return false;
    }

    if(!inputServer->HasTable("mHit"))
    {
        readTrackPos = false;
    }
    else
    {
        sprintf(query, "SELECT * FROM mHit LIMIT 1");
        if(makeQueryInput() == 0)
        {
            readTrackPos = false;
        }
        else
        {
            nextEntry();
            if(getInt(0) == 0) readTrackPos = false;
        }
    }

    //check additional information
    if(!inputServer->HasTable("QIE")) readQIE = false;
    if(!inputServer->HasTable("TriggerHit")) readTriggerHits = false;

    if(!readQIE) std::cout << "MySQLSvc: QIE information readout is disabled." << std::endl;
    if(!readTriggerHits) std::cout << "MySQLSvc: TriggerHits table readout is disabled." << std::endl;
    if(!readTargetPos) std::cout << "MySQLSvc: Target position readout is disabled." << std::endl;

    //Initialize trigger analyzer
    p_triggerAna = new TriggerAnalyzer();
    setTriggerEmu = p_triggerAna->init();
    if(setTriggerEmu)
    {
        p_triggerAna->buildTriggerTree();
    }
    else
    {
        std::cout << "MySQLSvc: Trigger emulation is disabled." << std::endl;
    }

    //Enable index if not enabled already
    bool indexEnabled = true;
    sprintf(query, "SELECT COMMENT FROM INFORMATION_SCHEMA.STATISTICS WHERE TABLE_SCHEMA='%s' AND TABLE_NAME='Hit' AND Column_name='detectorName'", inputSchema.c_str());
    if(makeQueryInput() == 0)
    {
        indexEnabled = false;
    }
    else
    {
        nextEntry();
        if(getString(0) == "disabled") indexEnabled = false;
    }

    if(!indexEnabled)
    {
        std::cout << "MySQLSvc: Index is not enabled for this schema, enable it now." << std::endl;

        //Temporarily connect as production user
        char address[200];
        sprintf(address, "mysql://%s:%d", inputServer->GetHost(), inputServer->GetPort());
        TSQLServer* server_temp = TSQLServer::Connect(address, "production", "qqbar2mu+mu-");

        sprintf(query, "USE %s", inputSchema.c_str());
        server_temp->Exec(query);
        sprintf(query, "ALTER TABLE Hit ENABLE KEYS");
        server_temp->Exec(query);
        if(readTriggerHits) sprintf(query, "ALTER TABLE TriggerHit ENABLE KEYS");
        server_temp->Exec(query);
        sprintf(query, "OPTIMIZE LOCAL TABLE `Run`, `Spill`, `Event`, `Hit`, `TriggerHit`, `Scaler`");
        server_temp->Exec(query);

        server_temp->Close();
    }

    return true;
}

bool MySQLSvc::isRunStopped()
{
    sprintf(query, "SELECT productionEnd from %s.production WHERE runID=%d", logSchema.c_str(), runID);
    if(makeQueryOutput() != 1) return false;

    nextEntry();
    if(row->GetField(0) == NULL)
    {
        return true;
    }
    return false;
}

bool MySQLSvc::getLatestEvt(SRawEvent* rawEvent)
{
    sprintf(query, "SELECT eventID FROM Event ORDER BY eventID DESC LIMIT 2");
    if(makeQueryInput() != 2) return false;

    nextEntry();
    nextEntry();
    int eventID = getInt(0);
    if(isEventLoaded(eventID)) return false;

    rawEvent->clear();
    if(!getEventHeader(rawEvent, eventID))
    {
        return false;
    }
    return getEvent(rawEvent, eventID);
}

//Cannot be used for now
bool MySQLSvc::getRandomEvt(SRawEvent* rawEvent)
{
    /*
       rawEvent->clear();
       if(!getEventHeader(rawEvent, eventID))
       {
       return false;
       }
       return getEvent(rawEvent, eventID);
       */

    return false;
}

bool MySQLSvc::getNextEvent(SRawEvent* rawEvent)
{
    int eventID = eventIDs[index_eventID++];

    rawEvent->clear();
    if(!getEventHeader(rawEvent, eventID))
    {
        return false;
    }
    return getEvent(rawEvent, eventID);
}

bool MySQLSvc::getNextEvent(SRawMCEvent* mcEvent)
{
    int eventID = eventIDs[index_eventID++];

    mcEvent->clear();
    if(!getEventHeader(mcEvent, eventID) || !getMCGenInfo(mcEvent, eventID))
    {
        return false;
    }
    return getEvent(mcEvent, eventID);
}

bool MySQLSvc::getEvent(SRawEvent* rawEvent, int eventID)
{
    //All hits but in station 4
    /* This query will enforce paired hits in station-4 hodo
    #ifdef USE_M_TABLES
    sprintf(query, "SELECT hitID,elementID,tdcTime,driftTime,driftDistance,detectorName,inTime,masked FROM mHit WHERE (detectorName LIKE 'D%%' "
    "OR detectorName LIKE 'H__' OR detectorName LIKE 'P%%') AND eventID=%d "
    "UNION "
    "SELECT h1.hitID,h1.elementID,0.5*(h1.tdcTime+h2.tdcTime),0.,0.,substr(h1.detectorName,1,3),h1.inTime AND h2.inTime,1 FROM "
    "(SELECT hitID,elementID,tdcTime,detectorName,inTime FROM mHit WHERE eventID=%d AND detectorName LIKE 'H4_u') AS h1,"
    "(SELECT hitID,elementID,tdcTime,detectorName,inTime FROM mHit WHERE eventID=%d AND detectorName LIKE 'H4_d') AS h2 "
    "WHERE substr(h1.detectorName,1,3) LIKE substr(h2.detectorName,1,3) AND Abs(h1.tdcTime-h2.tdcTime)<15. AND h1.elementID=h2.elementID "
    "UNION "
    "SELECT h3.hitID,h3.elementID,0.5*(h3.tdcTime+h4.tdcTime),0.,0.,substr(h3.detectorName,1,5),h3.inTime AND h4.inTime,1 FROM "
    "(SELECT hitID,elementID,tdcTime,detectorName,inTime FROM mHit WHERE eventID=%d AND detectorName LIKE 'H4Y__l') AS h3,"
    "(SELECT hitID,elementID,tdcTime,detectorName,inTime FROM mHit WHERE eventID=%d AND detectorName LIKE 'H4Y__r') AS h4 "
    "WHERE substr(h3.detectorName,1,3) LIKE substr(h4.detectorName,1,3) AND Abs(h3.tdcTime-h4.tdcTime)<15. AND h3.elementID=h4.elementID",
    eventID, eventID, eventID, eventID, eventID);

    #else
    sprintf(query, "SELECT hitID,elementID,tdcTime,driftTime,driftDistance,detectorName,inTime,masked FROM Hit WHERE (detectorName LIKE 'D%%' "
    "OR detectorName LIKE 'H__' OR detectorName LIKE 'P%%') AND eventID=%d "
    "UNION "
    "SELECT h1.hitID,h1.elementID,0.5*(h1.tdcTime+h2.tdcTime),0.,0.,substr(h1.detectorName,1,3),h1.inTime AND h2.inTime,1 FROM "
    "(SELECT hitID,elementID,tdcTime,detectorName,inTime FROM Hit WHERE eventID=%d AND detectorName LIKE 'H4_u') AS h1,"
    "(SELECT hitID,elementID,tdcTime,detectorName,inTime FROM Hit WHERE eventID=%d AND detectorName LIKE 'H4_d') AS h2 "
    "WHERE substr(h1.detectorName,1,3) LIKE substr(h2.detectorName,1,3) AND Abs(h1.tdcTime-h2.tdcTime)<15. AND h1.elementID=h2.elementID "
    "UNION "
    "SELECT h3.hitID,h3.elementID,0.5*(h3.tdcTime+h4.tdcTime),0.,0.,substr(h3.detectorName,1,5),h3.inTime AND h4.inTime,1 FROM "
    "(SELECT hitID,elementID,tdcTime,detectorName,inTime FROM Hit WHERE eventID=%d AND detectorName LIKE 'H4Y__l') AS h3,"
    "(SELECT hitID,elementID,tdcTime,detectorName,inTime FROM Hit WHERE eventID=%d AND detectorName LIKE 'H4Y__r') AS h4 "
    "WHERE substr(h3.detectorName,1,3) LIKE substr(h4.detectorName,1,3) AND Abs(h3.tdcTime-h4.tdcTime)<15. AND h3.elementID=h4.elementID",
    eventID, eventID, eventID, eventID, eventID);
    #endif
    */
#ifdef USE_M_TABLES
    sprintf(query, "SELECT hitID,elementID,tdcTime,driftDistance,detectorName,inTime,masked FROM mHit WHERE (detectorName LIKE 'D%%' "
            "OR detectorName LIKE 'H%%' OR detectorName LIKE 'P%%') AND eventID=%d", eventID);
#else
    sprintf(query, "SELECT hitID,elementID,tdcTime,driftDistance,detectorName,inTime,masked FROM Hit WHERE (detectorName LIKE 'D%%' "
            "OR detectorName LIKE 'H%%' OR detectorName LIKE 'P%%') AND eventID=%d", eventID);
#endif
    int nHits = makeQueryInput();
    if(nHits < 1) return false;

    for(int i = 0; i < nHits; ++i)
    {
        nextEntry();

        std::string detectorName(row->GetField(4));
        int elementID = getInt(1);
        p_geomSvc->toLocalDetectorName(detectorName, elementID);

        Hit h;
        h.index = getInt(0, 1);
        h.detectorID = p_geomSvc->getDetectorID(detectorName);
        h.elementID = elementID;
        h.tdcTime = getFloat(2);
        h.driftDistance = getFloat(3);
        h.pos = p_geomSvc->getMeasurement(h.detectorID, h.elementID);
        if(getInt(5, 0) > 0) h.setInTime();
        if(getInt(6, 0) > 0) h.setHodoMask();

        if(p_geomSvc->isCalibrationLoaded())
        {
            if((h.detectorID >= 1 && h.detectorID <= nChamberPlanes) || (h.detectorID >= nChamberPlanes+nHodoPlanes+1))
            {
                h.setInTime(p_geomSvc->isInTime(h.detectorID, h.tdcTime));
                if(h.isInTime()) h.driftDistance = p_geomSvc->getDriftDistance(h.detectorID, h.tdcTime);
            }
        }
        rawEvent->insertHit(h);
    }
    rawEvent->reIndex(true);

    //Set the trigger emulation info
    if(setTriggerEmu)
    {
        if(readTriggerHits)
        {
            rawEvent->setTriggerEmu(p_triggerAna->acceptEvent(rawEvent, USE_TRIGGER_HIT));
        }
        else
        {
            rawEvent->setTriggerEmu(p_triggerAna->acceptEvent(rawEvent, USE_HIT));
        }

        int nRoads[4] = {p_triggerAna->getNRoadsPosTop(), p_triggerAna->getNRoadsPosBot(), p_triggerAna->getNRoadsNegTop(), p_triggerAna->getNRoadsNegBot()};
        rawEvent->setNRoads(nRoads);
    }
    else
    {
        rawEvent->setTriggerEmu(false);

        int nRoads[4] = {0, 0, 0, 0};
        rawEvent->setNRoads(nRoads);
    }

    return true;
}

int MySQLSvc::getNEvents(int event_hi, int event_lo)
{
    TString eventQuery;
    if(!JobOptsSvc::instance()->m_mcMode)
    {
        //More cuts should apply, say spill quality cuts, event quality cuts, etc.
        eventQuery = "SELECT eventID FROM Event,Spill ";
        eventQuery += "WHERE Event.spillID=Spill.spillID ";
        eventQuery += "AND Spill.targetPos!=0 AND Spill.spillID!=0 AND Spill.targetPos>=1 AND Spill.targetPos<=7 ";
        eventQuery += "AND Spill.beamIntensity>1000 ";
        eventQuery += "AND Spill.dataQuality=0 ";
    }
    else
    {
        eventQuery = "SELECT eventID FROM mDimuon WHERE acceptHodoAll=1 AND acceptDriftAll=1";
    }

    if(event_hi > 0) eventQuery += Form(" AND eventID>=%d AND eventID<%d", event_lo, event_hi);

    //add eventID selection if appropriate
    if(!subsetEventString.empty()) eventQuery += " AND " + subsetEventString;

    //transfer string to query
    std::strcpy(query, eventQuery.Data());

    int nTotal = makeQueryInput();
    if(nTotal == 1) return 0;

    eventIDs.clear();
    eventIDs_loaded.clear();

    eventIDs.reserve(nTotal);
    eventIDs_loaded.reserve(nTotal);
    for(int i = 0; i < nTotal; ++i)
    {
        nextEntry();
        eventIDs.push_back(getInt(0));
    }
    return nTotal;
}

bool MySQLSvc::getEventHeader(SRawEvent* rawEvent, int eventID)
{
    eventIDs_loaded.push_back(eventID);

    //Get the event header
    sprintf(query, "SELECT runID,spillID,MATRIX1,MATRIX2,MATRIX3,MATRIX4,MATRIX5,NIM1,NIM2,NIM3,NIM4,NIM5 FROM Event WHERE eventID=%d", eventID);
    if(makeQueryInput() != 1) return false;

    nextEntry();
    runID = getInt(0);
    spillID = getInt(1);
    rawEvent->setEventInfo(runID, spillID, eventID);

    //Get the trigger bits
    int triggers[10];
    for(int i = 0; i < 10; ++i)
    {
        triggers[i] = getInt(i+2);
    }
    rawEvent->setTriggerBits(triggers);

    //Get target position
    if(readTargetPos)
    {
        sprintf(query, "SELECT targetPos FROM Spill WHERE spillID=%d", spillID);
        if(makeQueryInput() == 1)
        {
            nextEntry();
            rawEvent->setTargetPos(getInt(0));
        }
        else
        {
            rawEvent->setTargetPos(99);
        }
    }

    //Get beam information
    if(readQIE)
    {
        sprintf(query, "SELECT turnOnset,rfOnSet,`RF-16`,`RF-15`,`RF-14`,`RF-13`,`RF-12`,`RF-11`,`RF-10`,`RF-09`,"
                "`RF-08`,`RF-07`,`RF-06`,`RF-05`,`RF-04`,`RF-03`,`RF-02`,`RF-01`,`RF+00`,`RF+01`,`RF+02`,"
                "`RF+03`,`RF+04`,`RF+05`,`RF+06`,`RF+07`,`RF+08`,`RF+09`,`RF+10`,`RF+11`,`RF+12`,`RF+13`,"
                "`RF+14`,`RF+15`,`RF+16` FROM QIE WHERE eventID=%d", eventID);
        if(makeQueryInput() == 1)
        {
            nextEntry();

            rawEvent->setTurnID(getInt(0, -1));
            rawEvent->setRFID(getInt(1, -1));
            for(int i = 0; i < 33; ++i) rawEvent->setIntensity(i, getInt(i+2));
        }
        else
        {
            rawEvent->setTurnID(-2);
            rawEvent->setRFID(-2);
            for(int i = 0; i < 33; ++i) rawEvent->setIntensity(i, -1);
        }
    }

    //Get trigger hits
    if(readTriggerHits)
    {
        sprintf(query, "SELECT hitID,detectorName,elementID,tdcTime,inTime FROM TriggerHit WHERE detectorName LIKE 'H%%' AND eventID=%d", eventID);
        int nTriggerHits = makeQueryInput();

        for(int i = 0; i < nTriggerHits; ++i)
        {
            nextEntry();

            Hit h;
            h.index = getInt(0);
            h.elementID = getInt(2);
            h.tdcTime = getFloat(3);
            h.driftDistance = 0.;
            if(getInt(4, 0) > 0) h.setInTime();

            std::string detectorName(row->GetField(1));
            if(detectorName.find("H4T") != std::string::npos || detectorName.find("H4B") != std::string::npos)
            {
                detectorName.replace(3, detectorName.length(), "");
            }
            h.detectorID = p_geomSvc->getDetectorID(detectorName);
            h.pos = p_geomSvc->getMeasurement(h.detectorID, h.elementID);

            rawEvent->insertTriggerHit(h);
        }
    }

    return true;
}

bool MySQLSvc::getMCGenInfo(SRawMCEvent* mcEvent, int eventID)
{
    sprintf(query, "SELECT mTrackID1,mTrackID2,sigWeight,mass,xF,xB,xT,dx,dy,dz,dpx,dpy,runID,spillID,theta_mu FROM mDimuon WHERE acceptHodoAll=1 AND acceptDriftAll=1 AND eventID=%d", eventID);
    if(makeQueryInput() != 1) return false;
    nextEntry();

    runID = getInt(12);
    spillID = getInt(13);
    mcEvent->setEventInfo(runID, spillID, eventID);

    int trackID[2] = { getInt(0), getInt(1) };
    mcEvent->weight = getDouble(2);
    mcEvent->mass = getDouble(3);
    mcEvent->xF = getDouble(4);
    mcEvent->x1 = getDouble(5);
    mcEvent->x2 = getDouble(6);
    mcEvent->costh = cos(getDouble(14));
    mcEvent->vtx.SetXYZ(getDouble(7), getDouble(8), getDouble(9));

    double px = getDouble(10);
    double py = getDouble(11);
    mcEvent->pT = sqrt(px*px + py*py);

    if(!readTrackPos) return true;
    for(int i = 0; i < 2; ++i)
    {
        //At vertex
        sprintf(query, "SELECT px0,py0,pz0 FROM mTrack WHERE mTrackID=%d", trackID[i]);
        if(makeQueryInput() != 1) return false;

        nextEntry();
        mcEvent->p_vertex[i].SetXYZ(getDouble(0), getDouble(1), getDouble(2));

        //Number of chamber hits in one track
        sprintf(query, "SELECT COUNT(*) FROM mHit WHERE detectorName LIKE 'D%%' AND mTrackID=%d", trackID[i]);
        if(makeQueryInput() != 1) return false;

        nextEntry();
        mcEvent->nHits[i] = getInt(0);

        //At station 1,2,3,4
        sprintf(query, "(SELECT hpx,hpy,hpz,hx,hy,hz FROM mHit WHERE detectorName LIKE 'D1%%' AND mTrackID=%d LIMIT 1)"
                " UNION "
                "(SELECT hpx,hpy,hpz,hx,hy,hz FROM mHit WHERE detectorName LIKE 'D2%%' AND mTrackID=%d LIMIT 1)"
                " UNION "
                "(SELECT hpx,hpy,hpz,hx,hy,hz FROM mHit WHERE detectorName LIKE 'D3%%' AND mTrackID=%d LIMIT 1)"
                " UNION "
                "(SELECT hpx,hpy,hpz,hx,hy,hz FROM mHit WHERE detectorName LIKE 'P%%' AND mTrackID=%d LIMIT 1)"
                " UNION "
                "(SELECT hpx,hpy,hpz,hx,hy,hz FROM mHit WHERE detectorName RLIKE '^H[1-4][TB]$' AND mTrackID=%d)",
                trackID[i], trackID[i], trackID[i], trackID[i], trackID[i]);
        if(makeQueryInput() != 8) return false;

        nextEntry();
        mcEvent->p_station1[i].SetXYZ(getDouble(0), getDouble(1), getDouble(2));
        mcEvent->v_station1[i].SetXYZ(getDouble(3), getDouble(4), getDouble(5));

        nextEntry();
        mcEvent->p_station2[i].SetXYZ(getDouble(0), getDouble(1), getDouble(2));
        mcEvent->v_station2[i].SetXYZ(getDouble(3), getDouble(4), getDouble(5));

        nextEntry();
        mcEvent->p_station3[i].SetXYZ(getDouble(0), getDouble(1), getDouble(2));
        mcEvent->v_station3[i].SetXYZ(getDouble(3), getDouble(4), getDouble(5));

        nextEntry();
        mcEvent->p_station4[i].SetXYZ(getDouble(0), getDouble(1), getDouble(2));
        mcEvent->v_station4[i].SetXYZ(getDouble(3), getDouble(4), getDouble(5));

        nextEntry();
        mcEvent->p_stationH1[i].SetXYZ(getDouble(0), getDouble(1), getDouble(2));
        mcEvent->v_stationH1[i].SetXYZ(getDouble(3), getDouble(4), getDouble(5));

        nextEntry();
        mcEvent->p_stationH2[i].SetXYZ(getDouble(0), getDouble(1), getDouble(2));
        mcEvent->v_stationH2[i].SetXYZ(getDouble(3), getDouble(4), getDouble(5));

        nextEntry();
        mcEvent->p_stationH3[i].SetXYZ(getDouble(0), getDouble(1), getDouble(2));
        mcEvent->v_stationH3[i].SetXYZ(getDouble(3), getDouble(4), getDouble(5));

        nextEntry();
        mcEvent->p_stationH4[i].SetXYZ(getDouble(0), getDouble(1), getDouble(2));
        mcEvent->v_stationH4[i].SetXYZ(getDouble(3), getDouble(4), getDouble(5));
    }

    return true;
}

bool MySQLSvc::initWriter()
{
    using std::string;

    //If we are processing a subset of events, the an intermediate table
    //ill be used to store just the subset of events.
    //The intermediate table will have a suffix specific to the event range.
    //When the job is done, the intermediate tables are inserted into the final tables.
    const bool useSubsetTables = !subsetTableSuffix.empty();

    //create the output database if needed
    sprintf(query, "CREATE DATABASE IF NOT EXISTS %s", outputSchema.c_str());
    if(!outputServer->Exec(query))
    {
        std::cout << "MySQLSvc: working schema does not exist and cannot be created! Will exit..." << std::endl;
        return false;
    }

    sprintf(query, "USE %s", outputSchema.c_str());
    outputServer->Exec(query);

    //prepare the main output tables
    string tableNames[4] = {"kTrack", "kHit", "kDimuon", "kEvent"};
    for(int i = 0; i != 4; ++i)
    {
        const string& tableName = tableNames[i];
        //1. drop existing final tables unless processing an event subset
        if(!useSubsetTables)
        {
            sprintf(query, "DROP TABLE IF EXISTS %s", tableName.c_str() );
#ifndef OUT_TO_SCREEN
            outputServer->Exec(query);
#else
            std::cout << __FUNCTION__ << ": " << query << std::endl;
#endif
        }

        //2. get definition of table's fields and keys
        const string tableDefinition = getTableDefinition(tableName);

        //create final output tables
        sprintf(query, "CREATE TABLE IF NOT EXISTS %s (%s)", tableName.c_str(), tableDefinition.c_str());
#ifndef OUT_TO_SCREEN
        outputServer->Exec(query);
#else
        std::cout << __FUNCTION__ << ": " << query << std::endl;
#endif

        //prepare intermediate tables to hold subset of events
        if(useSubsetTables)
        {
            const string subsetTableName = Form("%s%s", tableName.c_str(), subsetTableSuffix.c_str());
            TSQLResult *res;

            //3. always drop subset table
            outputServer->Exec(Form("DROP TABLE IF EXISTS %s", subsetTableName.c_str()));

            //4. create the subset tbale
            sprintf(query, "CREATE TABLE IF NOT EXISTS %s (%s)", subsetTableName.c_str(), tableDefinition.c_str());
            outputServer->Exec(query);

            //5. see if data for the events in question already exist in the table (see if even a single row exists)
            sprintf(query, "SELECT * FROM %s WHERE %s LIMIT 1", tableName.c_str(), subsetEventString.c_str());
            res = outputServer->Query(query);

            // this will either be 1 if rows do exist, or 0 if none exist
            if(res->GetRowCount())
            {
                // delete subset's event range from the final table
                sprintf(query, "DELETE FROM %s WHERE %s", tableName.c_str(), subsetEventString.c_str());
                outputServer->Exec(query);
            }//end if(res->GetRowCount())
            // always close a TSQLResult
            res->Close();

        }//end if(useSubsetTable)
    }//end loop over tableNames


    //============
    // kInfo table
    //============
    std::cout << __FUNCTION__ << ": Now create kInfo table..." << std::endl;
    // remove kInfo table if doing an entire run
    // todo: Not clear what to do if using subset tables
    if(!useSubsetTables) outputServer->Exec("DROP TABLE IF EXISTS kInfo");

    //Book and fill kInfo table
    sprintf(query, "CREATE TABLE IF NOT EXISTS kInfo ("
                   "infoKey     VARCHAR(100),"
                   "infoValue   TEXT,"
                   "PRIMARY KEY(infoKey))");
    outputServer->Exec(query);

    std::cout << __FUNCTION__ << ": ...Done creating kInfo table." << std::endl;
    return true;
}

bool MySQLSvc::initBakWriter()
{
    using std::string;

    //If we are processing a subset of events, the an intermediate table
    //ill be used to store just the subset of events.
    //The intermediate table will have a suffix specific to the event range.
    //When the job is done, the intermediate tables are inserted into the final tables.
    const bool useSubsetTables = !subsetTableSuffix.empty();

    //create the output database if needed
    sprintf(query, "CREATE DATABASE IF NOT EXISTS %s", outputSchema.c_str());
    if(!outputServer->Exec(query))
    {
        std::cout << "MySQLSvc: working schema does not exist and cannot be created! Will exit..." << std::endl;
        return false;
    }

    sprintf(query, "USE %s", outputSchema.c_str());
    outputServer->Exec(query);

    //prepare the main output tables
    string tableNames[13] = {"kEventMix", "kTrackMix", "kDimuonMix", "kTrackPP", "kDimuonPP",
                            "kTrackMM", "kDimuonMM", "kEventMixPP", "kTrackMixPP", "kDimuonMixPP",
                            "kEventMixMM", "kTrackMixMM", "kDimuonMixMM"};
    for(int i = 0; i != 13; ++i)
    {
        const string& tableName = tableNames[i];

        //1. drop existing final tables unless processing an event subset
        if(!useSubsetTables)
        {
            sprintf(query, "DROP TABLE IF EXISTS %s", tableName.c_str() );
#ifndef OUT_TO_SCREEN
            outputServer->Exec(query);
#else
            std::cout << __FUNCTION__ << ": " << query << std::endl;
#endif
        }

        //2. get definition of table's fields and keys
        const string tableDefinition = getTableDefinition(tableName);

        //create final output tables
        sprintf(query, "CREATE TABLE IF NOT EXISTS %s (%s)", tableName.c_str(), tableDefinition.c_str());
#ifndef OUT_TO_SCREEN
        outputServer->Exec(query);
#else
        std::cout << __FUNCTION__ << ": " << query << std::endl;
#endif

        //prepare intermediate tables to hold subset of events
        if(useSubsetTables)
        {
            const string subsetTableName = Form("%s%s", tableName.c_str(), subsetTableSuffix.c_str());

            //3. always drop subset table
            outputServer->Exec(Form( "DROP TABLE IF EXISTS %s", subsetTableName.c_str()));

            //4. create the subset tbale
            sprintf(query, "CREATE TABLE IF NOT EXISTS %s (%s)", subsetTableName.c_str(), tableDefinition.c_str());
            outputServer->Exec(query);

            //5. delete subset's event range from the final table
            sprintf(query, "DELETE FROM %s WHERE %s", tableName.c_str(), subsetEventString.c_str());
            outputServer->Exec(query);
        }//end if(useSubsetTable)
    }//end loop over tableNames

    return true;
}

void MySQLSvc::resetWriter()
{
    //reset the track and dimuon counter
    nTracks = 0;
    nDimuons = 0;

    //reset the buffer query subset
    eventQuery = "";
    trackQuery = "";
}

void MySQLSvc::finalizeWriter()
{
    if(eventQuery.Length() > 10)  commitInsertion(eventQuery);
    if(dimuonQuery.Length() > 10) commitInsertion(dimuonQuery);
    if(trackQuery.Length() > 10)  commitInsertion(trackQuery);
    if(hitQuery.Length() > 10)    commitInsertion(hitQuery);
}

std::string MySQLSvc::getSubsetTableSuffix() const
{
    JobOptsSvc* jobOptsSvc = JobOptsSvc::instance();
    if(jobOptsSvc->ProcessAllEvents())
    {
        return "";
    }
    else if(jobOptsSvc->m_nEvents == -1) //if nEvents is -1, then we are going to the last event
    {
        return Form("_%d_END", jobOptsSvc->m_firstEvent);
    }
    else
    {
        return Form("_%d_%d", jobOptsSvc->m_firstEvent, jobOptsSvc->m_firstEvent+jobOptsSvc->m_nEvents-1);
    }
}

std::string MySQLSvc::getMySQLEventSelection() const
{
    JobOptsSvc* jobOptsSvc = JobOptsSvc::instance();
    if(jobOptsSvc->ProcessAllEvents())
    {
        return "";
    }
    else if(jobOptsSvc->m_nEvents == -1) //if nEvents is -1, then we are going to the last event
    {
        return Form("eventID >= %d", jobOptsSvc->m_firstEvent);
    }
    else
    {
        return Form("eventID BETWEEN %d AND %d", jobOptsSvc->m_firstEvent, jobOptsSvc->m_firstEvent+jobOptsSvc->m_nEvents-1);
    }
}

std::string MySQLSvc::getTableDefinition(const TString tableType) const
{
    if(tableType.Contains("kEvent"))
    {
        return "runID       SMALLINT, "
               "spillID     INTEGER, "
               "eventID     INTEGER, "
               "status      SMALLINT, "
               "source1     INTEGER,  "
               "source2     INTEGER,  "
               "PRIMARY KEY(eventID), "
               "INDEX(spillID)";
    }

    if(tableType.Contains("kTrack"))
    {
        return "trackID     INTEGER,"
               "runID       SMALLINT,"
               "spillID     INTEGER,"
               "eventID     INTEGER,"
               "charge      INTEGER,"
               "roadID      INTEGER,"
               "numHits     INTEGER,"
               "numHitsSt1  INTEGER,"
               "numHitsSt2  INTEGER,"
               "numHitsSt3  INTEGER,"
               "numHitsSt4H INTEGER,"
               "numHitsSt4V INTEGER,"
               "chisq       DOUBLE, "
               "x0          DOUBLE, "
               "y0          DOUBLE, "
               "z0          DOUBLE, "
               "xD          DOUBLE, "
               "yD          DOUBLE, "
               "xT          DOUBLE, "
               "yT          DOUBLE, "
               "pxD         DOUBLE, "
               "pyD         DOUBLE, "
               "pzD         DOUBLE, "
               "pxT         DOUBLE, "
               "pyT         DOUBLE, "
               "pzT         DOUBLE, "
               "z0x         DOUBLE, "
               "z0y         DOUBLE, "
               "px0         DOUBLE, "
               "py0         DOUBLE, "
               "pz0         DOUBLE, "
               "x1          DOUBLE, "
               "y1          DOUBLE, "
               "z1          DOUBLE, "
               "px1         DOUBLE, "
               "py1         DOUBLE, "
               "pz1         DOUBLE, "
               "x3          DOUBLE, "
               "y3          DOUBLE, "
               "z3          DOUBLE, "
               "px3         DOUBLE, "
               "py3         DOUBLE, "
               "pz3         DOUBLE, "
               "thbend      DOUBLE, "
               "tx_PT       DOUBLE, "
               "ty_PT       DOUBLE, "
               "chisq_target     DOUBLE, "
               "chisq_dump       DOUBLE, "
               "chisq_upstream   DOUBLE, "
               "kmstatus    SMALLINT,"
               "PRIMARY KEY(trackID), "
               "INDEX(eventID), "
               "INDEX(spillID)";
    }//end of kTrack

    if(tableType.Contains("kDimuon"))
    {
        return  "dimuonID    INTEGER,"
                "runID       SMALLINT,"
                "spillID     INTEGER,"
                "eventID     INTEGER,"
                "targetPos   INTEGER,"
                "posTrackID  INTEGER,"
                "negTrackID  INTEGER,"
                "dx          DOUBLE, "
                "dy          DOUBLE, "
                "dz          DOUBLE, "
                "dpx         DOUBLE, "
                "dpy         DOUBLE, "
                "dpz         DOUBLE, "
                "mass        DOUBLE, "
                "xF          DOUBLE, "
                "xB          DOUBLE, "
                "xT          DOUBLE, "
                "costh       DOUBLE, "
                "phi         DOUBLE, "
                "trackSeparation DOUBLE,"
                "chisq_dimuon    DOUBLE,"
                "px1         DOUBLE,"
                "py1         DOUBLE,"
                "pz1         DOUBLE,"
                "px2         DOUBLE,"
                "py2         DOUBLE,"
                "pz2         DOUBLE,"
                "isValid     INTEGER,"
                "isTarget    INTEGER,"
                "isDump      INTEGER,"
                "PRIMARY KEY(dimuonID), "
                "INDEX(eventID), "
                "INDEX(spillID)";
    }//end of kDimuon

    if(tableType.Contains("kHit"))
    {
        return "runID       SMALLINT,"
               "eventID     INTEGER, "
               "spillID     INTEGER, "
               "trackID     INTEGER, "
               "hitID       BIGINT,  "
               "driftSign   SMALLINT,"
               "residual    DOUBLE,  "
               "tdcTime     DOUBLE,  "
               "driftDistance DECIMAL(5,4), "
               "detectorName  CHAR(16), "
               "elementID     SMALLINT(6), "
               "PRIMARY KEY(trackID, hitID), "
               "INDEX(eventID), "
               "INDEX(spillID)";
    }//end of kHit

    // to mute warning, program won't come to this part if development is not stupid
    return "";
}

void MySQLSvc::writeTrackingRes(TString tableSuffix, SRecEvent* recEvent, TClonesArray* tracklets)
{
    runID = recEvent->getRunID();
    spillID = recEvent->getSpillID();
    int targetPos = recEvent->getTargetPos();
    if(!eventIDs_loaded.empty())
    {
        if(eventIDs_loaded.back() != recEvent->getEventID()) eventIDs_loaded.push_back(recEvent->getEventID());
    }
    else
    {
        eventIDs_loaded.push_back(recEvent->getEventID());
    }

    //Write the general event table
    writeEventTable(recEvent->getEventID(), recEvent->getRecStatus(), recEvent->getSourceID1(), recEvent->getSourceID2(), tableSuffix);

    //Fill Track table/TrackHit table
    int nTracks_local = recEvent->getNTracks();
    for(int i = 0; i < nTracks_local; ++i)
    {
        int trackID = nTracks + i;
        writeTrackTable(trackID, &recEvent->getTrack(i), tableSuffix);
        if(tracklets != NULL) writeTrackHitTable(trackID, (Tracklet*)tracklets->At(i));
    }

    int nDimuons_local = recEvent->getNDimuons();
    for(int i = 0; i < nDimuons_local; ++i)
    {
        writeDimuonTable(nDimuons+i, recEvent->getDimuon(i), tableSuffix, targetPos);
    }

    nTracks += nTracks_local;
    nDimuons += nDimuons_local;
}

void MySQLSvc::writeInfoTable(TTree* config)
{
    if(config->GetEntries() == 0) return;
    config->GetEntry(0);

    TObjArray* array = config->GetListOfLeaves();
    for(int i = 0; i < array->GetEntries(); ++i)
    {
        TString key = array->At(i)->GetName();
        if(key == "FirstEvent" || key == "NEvents") continue;

        if(TString(((TLeaf*)array->At(i))->GetTypeName()) == "TString")
        {
            TString val = *(TString*)(config->GetLeaf(key.Data())->GetValuePointer());
            outputServer->Exec(Form("INSERT INTO kInfo (infoKey,infoValue) VALUES('%s','%s')", key.Data(), val.Data()));
        }
        else
        {
            int val = int(config->GetLeaf(key.Data())->GetValue());
            outputServer->Exec(Form("INSERT INTO kInfo (infoKey,infoValue) VALUES('%s', '%d')", key.Data(), val));
        }
    }

    //Tracker version from software release
#ifndef GIT_VERSION
    char* trackerVersion = getenv("SEAQUEST_RELEASE");
    if(!trackerVersion) sprintf(trackerVersion, "unknown.version");
    sprintf(query, "INSERT INTO kInfo (infoKey,infoValue) "
                   "VALUES('%s','%s')", "version", trackerVersion);
    outputServer->Exec(query);
#endif

    //Run/upload time
    outputServer->Exec("INSERT INTO kInfo (infoKey,infoValue) VALUES('LastUpdated',NOW())");
}

void MySQLSvc::writeEventTable(int eventID, int statusCode, int source1, int source2, TString tableSuffix)
{
    if(!(tableSuffix == "" || tableSuffix.Contains("Mix"))) return;
    if(eventQuery.Length() == 0)
    {
        eventQuery = TString(Form("INSERT INTO kEvent%s (runID,spillID,eventID,status,source1,source2) VALUES", tableSuffix.Data()));
    }

    eventQuery += Form(" (%d,%d,%d,%d,%d,%d),", runID, spillID, eventID, statusCode, source1, source2);
    if(eventQuery.Length() > MaxQueryLen) commitInsertion(eventQuery);
}

void MySQLSvc::writeTrackTable(int trackID, SRecTrack* recTrack, TString bakSuffix)
{
    double px0, py0, pz0, x0, y0, z0;
    double px1, py1, pz1, x1, y1;
    double px3, py3, pz3, x3, y3;

    int charge;
    int roadID;
    int kmstatus;
    int numHits, numHitsSt1, numHitsSt2, numHitsSt3, numHitsSt4H, numHitsSt4V;
    double chisq, chisq_target, chisq_dump, chisq_upstream;

    TVector3 pos_target = recTrack->getTargetPos();
    TVector3 pos_dump = recTrack->getDumpPos();
    TVector3 mom_target = recTrack->getTargetMom();
    TVector3 mom_dump = recTrack->getDumpMom();
    TVector3 pos_xvertex = recTrack->getXVertexPos();
    TVector3 pos_yvertex = recTrack->getYVertexPos();

    //if(std::isnan(pos_xvertex.Z()) || std::isnan(pos_yvertex.Z())) return;

    double tx_prop = recTrack->getPTSlopeX();
    double ty_prop = recTrack->getPTSlopeY();

    //Track related
    charge = recTrack->getCharge();
    roadID = recTrack->getTriggerRoad();
    kmstatus = recTrack->isKalmanFitted();
    numHits = recTrack->getNHits();
    numHitsSt1 = recTrack->getNHitsInStation(1);
    numHitsSt2 = recTrack->getNHitsInStation(2);
    numHitsSt3 = recTrack->getNHitsInStation(3);
    numHitsSt4H = recTrack->getNHitsInPTY();
    numHitsSt4V = recTrack->getNHitsInPTX();
    chisq = recTrack->getChisq();
    chisq_target = recTrack->getChisqTarget();
    chisq_dump = recTrack->getChisqDump();
    chisq_upstream = recTrack->getChisqUpstream();

    //Vertex point
    x0 = recTrack->getVtxPar(0);
    y0 = recTrack->getVtxPar(1);
    z0 = recTrack->getVtxPar(2);
    recTrack->getMomentumVertex(px0, py0, pz0);
    //if(std::isnan(px0) || std::isnan(py0) || std::isnan(pz0)) return;

    //At station 1
    recTrack->getExpPositionFast(Z_ST1, x1, y1);
    recTrack->getExpMomentumFast(Z_ST1, px1, py1, pz1);
    //if(std::isnan(px1) || std::isnan(py1) || std::isnan(pz1)) return;

    //At station 3
    recTrack->getExpPositionFast(Z_ST3, x3, y3);
    recTrack->getExpMomentumFast(Z_ST3, px3, py3, pz3);
    //if(std::isnan(px3) || std::isnan(py3) || std::isnan(pz3)) return;

    if(trackQuery.Length() == 0)
    {
        trackQuery = Form("INSERT INTO kTrack%s%s", bakSuffix.Data(), subsetTableSuffix.c_str());
        trackQuery += TString("(trackID,runID,spillID,eventID,charge,roadID,"
                              "numHits,numHitsSt1,numHitsSt2,numHitsSt3,numHitsSt4H,numHitsSt4V,"
                              "chisq,x0,y0,z0,px0,py0,pz0,"
                              "xT,yT,xD,yD,z0x,z0y,"
                              "pxT,pyT,pzT,pxD,pyD,pzD,"
                              "x1,y1,z1,px1,py1,pz1,"
                              "x3,y3,z3,px3,py3,pz3,"
                              "thbend,tx_PT,ty_PT,"
                              "chisq_target,chisq_dump,chisq_upstream,kmstatus) VALUES");
    }

    TString insertQuery = Form(" (%d,%d,%d,%d,%d,%d,", trackID, runID, spillID, eventIDs_loaded.back(), charge, roadID);
    insertQuery += Form("%d,%d,%d,%d,%d,%d,", numHits, numHitsSt1, numHitsSt2, numHitsSt3, numHitsSt4H, numHitsSt4V);
    insertQuery += Form("%f,%f,%f,%f,%f,%f,%f,", chisq, x0, y0, z0, px0, py0, pz0);
    insertQuery += Form("%f,%f,%f,%f,%f,%f,", pos_target.X(), pos_target.Y(), pos_dump.X(), pos_dump.Y(), pos_xvertex.Z(), pos_yvertex.Z());
    insertQuery += Form("%f,%f,%f,%f,%f,%f,", mom_target.X(), mom_target.Y(), mom_target.Z(), mom_dump.X(), mom_dump.Y(), mom_dump.Z());
    insertQuery += Form("%f,%f,%f,%f,%f,%f,", x1, y1, Z_ST1, px1, py1, pz1);
    insertQuery += Form("%f,%f,%f,%f,%f,%f,", x3, y3, Z_ST3, px3, py3, pz3);
    insertQuery += Form("%f,%f,%f,", atan(px3/pz3)-atan(px1/pz1), tx_prop, ty_prop);
    insertQuery += Form("%f,%f,%f,%d),", chisq_target, chisq_dump, chisq_upstream, kmstatus);
    if(!insertQuery.Contains("nan")) trackQuery += insertQuery;

    if(trackQuery.Length() > MaxQueryLen) commitInsertion(trackQuery);
}

void MySQLSvc::writeTrackHitTable(int trackID, Tracklet* tracklet)
{
    if(hitQuery.Length() == 0)
    {
        hitQuery = Form("INSERT INTO kHit%s", subsetTableSuffix.c_str());
        hitQuery += TString("(runID,spillID,eventID,trackID,hitID,driftSign,residual,tdcTime,detectorName,elementID,driftDistance) VALUES");
    }

    //chamber hits upload
    for(std::list<SignedHit>::iterator iter = tracklet->hits.begin(); iter != tracklet->hits.end(); ++iter)
    {
        if(iter->hit.index < 0) continue;
        hitQuery += Form(" (%d,%d,%d,%d,%d,%d,%f,", runID, spillID, eventIDs_loaded.back(), trackID, iter->hit.index, iter->sign, tracklet->residual[iter->hit.detectorID-1]);
        hitQuery += Form("%f,'%s',%d,%f),", iter->hit.tdcTime, p_geomSvc->getDetectorName(iter->hit.detectorID).c_str(), iter->hit.elementID, iter->hit.driftDistance);
    }

    //x-hodo hits upload -- NOTE: for now this is obtained by parsing the trigger road ID
    int hitID = -1;
    TriggerRoad road(*tracklet);
    for(int i = 0; i < road.getNElements(); ++i)
    {
        int detectorID = road.getDetectorID(i);
        int elementID = road.getElementID(i);
        double res = tracklet->getExpPositionW(detectorID) - p_geomSvc->getMeasurement(detectorID, elementID);

        hitQuery += Form(" (%d,%d,%d,%d,%d,%d,%f,", runID, spillID, eventIDs_loaded.back(), trackID, hitID--, 0, res);
        hitQuery += Form("%f,'%s',%d,%f),", 0., p_geomSvc->getDetectorName(detectorID).c_str(), elementID, 0.);
    }

    //prop tube hits upload
    for(int i = 0; i < 4; ++i)
    {
        SignedHit hit1 = tracklet->seg_x.hits[i];
        if(hit1.hit.index > 0)
        {
            hitQuery += Form(" (%d,%d,%d,%d,%d,%d,%f,", runID, spillID, eventIDs_loaded.back(), trackID, hit1.hit.index, hit1.sign, 0.);
            hitQuery += Form("%f,'%s',%d,%f),", hit1.hit.tdcTime, p_geomSvc->getDetectorName(hit1.hit.detectorID).c_str(), hit1.hit.elementID, hit1.hit.driftDistance);
        }

        SignedHit hit2 = tracklet->seg_y.hits[i];
        if(hit2.hit.index > 0)
        {
            hitQuery += Form(" (%d,%d,%d,%d,%d,%d,%f,", runID, spillID, eventIDs_loaded.back(), trackID, hit2.hit.index, hit2.sign, 0.);
            hitQuery += Form("%f,'%s',%d,%f),", hit2.hit.tdcTime, p_geomSvc->getDetectorName(hit2.hit.detectorID).c_str(), hit2.hit.elementID, hit2.hit.driftDistance);
        }
    }

    if(hitQuery.Length() > MaxQueryLen) commitInsertion(hitQuery);
}

void MySQLSvc::writeDimuonTable(int dimuonID, SRecDimuon dimuon, TString bakSuffix, int targetPos)
{
    //basic validity check
    if(std::isnan(dimuon.mass)) return;

    double x0 = dimuon.vtx.X();
    double y0 = dimuon.vtx.Y();
    double z0 = dimuon.vtx.Z();

    TLorentzVector p_sum = dimuon.getVPhoton();
    double px0 = p_sum.Px();
    double py0 = p_sum.Py();
    double pz0 = p_sum.Pz();
    double dz = dimuon.vtx_pos.Z() - dimuon.vtx_neg.Z();

    if(dimuonQuery.Length() == 0)
    {
        dimuonQuery = Form("INSERT INTO kDimuon%s%s", bakSuffix.Data(), subsetTableSuffix.c_str());
        dimuonQuery += TString("(dimuonID,runID,spillID,eventID,targetPos,posTrackID,negTrackID,"
                               "dx,dy,dz,dpx,dpy,dpz,"
                               "mass,xF,xB,xT,costh,phi,"
                               "trackSeparation,chisq_dimuon,"
                               "px1,py1,pz1,px2,py2,pz2,"
                               "isValid,isTarget,isDump) VALUES");
    }

    TString insertQuery = Form(" (%d,%d,%d,%d,%d,%d,%d,", dimuonID, runID, spillID, eventIDs_loaded.back(), targetPos, dimuon.trackID_pos+nTracks, dimuon.trackID_neg+nTracks);
    insertQuery += Form("%f,%f,%f,%f,%f,%f,", x0, y0, z0, px0, py0, pz0);
    insertQuery += Form("%f,%f,%f,%f,%f,%f,", dimuon.mass, dimuon.xF, dimuon.x1, dimuon.x2, dimuon.costh, dimuon.phi);
    insertQuery += Form("%f,%f,", dz, dimuon.chisq_kf);
    insertQuery += Form("%f,%f,%f,%f,%f,%f,", dimuon.p_pos.Px(), dimuon.p_pos.Py(), dimuon.p_pos.Pz(), dimuon.p_neg.Px(), dimuon.p_neg.Py(), dimuon.p_neg.Pz());
    insertQuery += Form("%i,%i,%i),", dimuon.isValid(), dimuon.isTarget(), dimuon.isDump());
    if(!insertQuery.Contains("nan")) dimuonQuery += insertQuery;

    if(dimuonQuery.Length() > MaxQueryLen) commitInsertion(dimuonQuery);
}

void MySQLSvc::pushToFinalTables(bool dropSubsetTables)
{
    //if we didn't write to intermediate subset tables, nothing to be done
    if(subsetTableSuffix.empty()) return;

    std::string tableNames[3] = {"kTrack", "kHit", "kDimuon"};
    for(int i = 0; i != 3; ++i)
    {
        const std::string& finalTable = tableNames[i];
        const std::string subsetTable = finalTable + subsetTableSuffix;
        const TString cpQuery = Form("INSERT INTO %s SELECT * FROM %s;", finalTable.c_str(), subsetTable.c_str());
        outputServer->Exec(cpQuery);
        if(dropSubsetTables)
        {
            const TString rmQuery = Form( "DROP TABLE %s;", subsetTable.c_str() );
            outputServer->Exec(rmQuery);
        }
    }//end loop over tables
}//end pushToFinalTables

int MySQLSvc::getNEventsFast(int event_hi, int event_lo)
{
    if(nEvents < 1) nEvents = getNEvents(event_hi, event_lo);
    return nEvents;
}

int MySQLSvc::makeQueryInput()
{
    //std::cout << query << std::endl;
    if(inputServer == NULL) return 0;

    if(res != NULL) delete res;
    res = inputServer->Query(query);

    if(res != NULL) return res->GetRowCount();
    return 0;
}

int MySQLSvc::makeQueryOutput()
{
    //std::cout << query << std::endl;
    if(outputServer == NULL) return 0;

    if(res != NULL) delete res;
    res = outputServer->Query(query);

    if(res != NULL) return res->GetRowCount();
    return 0;
}


void MySQLSvc::commitInsertion(TString& insertion)
{
    //remove the last comma
    insertion.Chop();

#ifndef OUT_TO_SCREEN
    outputServer->Exec(insertion);
#else
    std::cout << std::endl << __FUNCTION__ << " " << insertion.Length() << ": " << insertion << std::endl;
#endif

    //clear up
    insertion.Clear();
}

bool MySQLSvc::nextEntry()
{
    if(res == NULL) return false;

    if(row != NULL) delete row;
    row = res->Next();

    if(row != NULL) return true;
    return false;
}

int MySQLSvc::getInt(int id, int default_val)
{
    if(row->GetField(id) == NULL)
    {
        return default_val;
    }

    try
    {
        return boost::lexical_cast<int>(row->GetField(id));
    }
    catch(boost::bad_lexical_cast&)
    {
        return default_val;
    }
}

float MySQLSvc::getFloat(int id, float default_val)
{
    if(row->GetField(id) == NULL)
    {
        return default_val;
    }

    return boost::lexical_cast<float>(row->GetField(id));
}

double MySQLSvc::getDouble(int id, double default_val)
{
    if(row->GetField(id) == NULL)
    {
        return default_val;
    }

    return boost::lexical_cast<double>(row->GetField(id));
}

std::string MySQLSvc::getString(int id, std::string default_val)
{
    if(row->GetField(id) == NULL)
    {
        return default_val;
    }

    return std::string(row->GetField(id));
}
