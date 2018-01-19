#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <wordexp.h> //to expand environmentals
#include <boost/algorithm/string.hpp> //for strip
#include <boost/lexical_cast.hpp>

#include "JobOptsSvc.h"
#include <TTree.h>

using namespace std;

//todo a msg service
//#define DEBUG_JobOptsSvc  //comment out for quiet

bool debug()
{
#ifdef DEBUG_JobOptsSvc
    return true;
#else
    return false;
#endif
}

JobOptsSvc* JobOptsSvc::p_jobOptsSvc = NULL;

JobOptsSvc* JobOptsSvc::instance()
{
    if(debug()) cout << "JobOptsSvc::instance()" << endl;

    if(NULL == p_jobOptsSvc)
    {
        p_jobOptsSvc = new JobOptsSvc;

        if(debug()) cout << " creating new JobOptsSvc object" << endl;
        p_jobOptsSvc->init();
    }

    return p_jobOptsSvc;
}

JobOptsSvc::JobOptsSvc()
{
    //Default values of all options
    m_configFile = "$KTRACKER_ROOT/opts/default.opts"; ///< Name of the config file loaded

    m_mcMode = false;
    m_alignmentMode = false;
    m_enableTriggerMask = true;
    m_enableKMag = true;
    m_enableOnlineAlignment = false;
    m_enableEvaluation = false;
    m_attachRawEvent = false;
    m_sagittaReducer = true;
    m_updateAlignment = true;
    m_hodomask = true;
    m_mergeHodo = false;
    m_realization = false;

    m_mySQLInputPort = 3306;
    m_mySQLOutputPort = 3396;
    m_nEvents = -1;
    m_firstEvent = 0;
    m_triggerL1 = 67;

    m_timingOffset = 0;

    m_st0_reject = 0.13;
    m_st1_reject = 0.25;
    m_st2_reject = 0.18;
    m_st3p_reject = 0.16;
    m_st3m_reject = 0.16;

    m_x_vtx = 0.;
    m_y_vtx = 0.;
    m_x_spot_size = 0.5;
    m_y_spot_size = 0.5;

    m_inputFile = "";
    m_outputFile = "";
    m_inputSchema = "";
    m_outputSchema = "";

    m_alignmentFileHodo    = "$KTRACKER_ROOT/alignment/run2/alignment_hodo.txt";
    m_alignmentFileChamber = "$KTRACKER_ROOT/alignment/run2/alignment_cham.txt";
    m_alignmentFileProp    = "$KTRACKER_ROOT/alignment/run2/alignment_prop.txt";
    m_alignmentFileMille   = "$KTRACKER_ROOT/alignment/run2/align_mille.txt";

    m_triggerRepo = "$TRIGGER_ROOT";

    m_calibrationsFile;

    m_fMagFile = "$GEOMETRY_ROOT/magnetic_fields/tab.Fmag";
    m_kMagFile = "$GEOMETRY_ROOT/magnetic_fields/tab.Kmag";

    m_geomVersion = "geometry_G18_run3";
    m_mySQLInputServer  = "e906-db1.fnal.gov";
    m_mySQLOutputServer = "e906-db1.fnal.gov";
}

void JobOptsSvc::close()
{
    if(debug()) cout << "   JobOptsSvc::close()" << endl;

    if(NULL != p_jobOptsSvc)
    {
        delete p_jobOptsSvc;
    }
    else
    {
        std::cout << "Error: no instance of job options service found! " << std::endl;
    }
}

bool JobOptsSvc::init(bool forceInit /* = false */ )
{
    if(debug()) cout << "JobOptsSvc::init()" << endl;


    //don't accidentally load defaults over other settings
    if(m_isInit && !forceInit) return true;

    //hardcoded default standard job options file
    return init("$KTRACKER_ROOT/opts/default.opts");
}

bool JobOptsSvc::init(const char* configfile)
{
    if(debug()) cout << "JobOptsSvc::init( " << configfile << " )" << endl;


    // expand any environment variables in the file name
    m_configFile = ExpandEnvironmentals( configfile );
    if(debug()) cout << "My config file is :" << m_configFile << endl;

    std::ifstream fin( m_configFile.c_str() );
    if(!fin)
    {
        cout << "JobOptsSvc::init - ERROR - Your config file '" << m_configFile << "' cannot be opened!" << endl;
        throw 1;
    }


    //define the types of options to look for
    map<string, string*> stringOpts;
    stringOpts["InputFile"] = &m_inputFile;
    stringOpts["OutputFile"] = &m_outputFile;
    stringOpts["InputSchema"] = &m_inputSchema;
    stringOpts["OutputSchema"] = &m_outputSchema;

    stringOpts["AlignmentFile_Hodo"] = &m_alignmentFileHodo;
    stringOpts["AlignmentFile_Mille"] = &m_alignmentFileMille;
    stringOpts["AlignmentFile_Prop"] = &m_alignmentFileProp;
    stringOpts["AlignmentFile_Chamber"] = &m_alignmentFileChamber;

    stringOpts["Trigger_Repository"] = &m_triggerRepo;

    stringOpts["CalibrationsFile"] = &m_calibrationsFile;

    stringOpts["fMagFile"] = &m_fMagFile;
    stringOpts["kMagFile"] = &m_kMagFile;

    stringOpts["MySQL_InputServer"] = &m_mySQLInputServer;
    stringOpts["MySQL_OutputServer"] = &m_mySQLOutputServer;
    stringOpts["Geometry_Version"] = &m_geomVersion;

    map<string, int*> intOpts;
    intOpts["MySQL_InputPort"] = &m_mySQLInputPort;
    intOpts["MySQL_OutputPort"] = &m_mySQLOutputPort;
    intOpts["N_Events"] = &m_nEvents;
    intOpts["FirstEvent"] = &m_firstEvent;
    intOpts["Trigger_L1"] = &m_triggerL1;

    map<string, bool*> boolOpts;
    boolOpts["MCMode_enable"] = &m_mcMode;
    boolOpts["AlignmentMode_enable"] = &m_alignmentMode;
    boolOpts["TriggerMask_enable"] = &m_enableTriggerMask;
    boolOpts["kMag_enable"] = &m_enableKMag;
    boolOpts["Evaluation_enable"] = &m_enableEvaluation;
    boolOpts["OnlineAlignment_enable"] = &m_enableOnlineAlignment;
    boolOpts["AttachRawEvent"] = &m_attachRawEvent;
    boolOpts["SagittaReducer"] = &m_sagittaReducer;
    boolOpts["UpdateAlignment"] = &m_updateAlignment;
    boolOpts["HodoscopeMasking"] = &m_hodomask;
    boolOpts["MergeHodoHits"] = &m_mergeHodo;
    boolOpts["MCRealization"] = &m_realization;

    map<string, double*> doubleOpts;
    doubleOpts["TimingOffset"] = &m_timingOffset;
    doubleOpts["ST0_Reject"] = &m_st0_reject;
    doubleOpts["ST1_Reject"] = &m_st1_reject;
    doubleOpts["ST2_Reject"] = &m_st2_reject;
    doubleOpts["ST3p_Reject"] = &m_st3p_reject;
    doubleOpts["ST3m_Reject"] = &m_st3m_reject;

    //read the file and find matching options
    string line;
    while(getline(fin, line))
    {
        if(debug()) cout << "line = " << line << endl;
        boost::algorithm::trim(line);

        //skip empty and comment lines
        if(line.empty() || line[0]=='#') continue;

        stringstream ss(line);
        string key, val;
        ss >> key >> val;
        if(val.empty())
        {
            if(debug()) cout << "JobOptsSvc::init - WARNING - caught std::logic_error on line.  Possible value missing in line: " << line << endl;
            continue;
        }


        if(debug()) cout << " key = " << key << ", val = " << val << endl;

        //is this a string option?
        {
            map<string,string*>::iterator it = stringOpts.find(key);
            if(stringOpts.end() != it)
            {
                string expandedVal = ExpandEnvironmentals( val );
                it->second->assign(expandedVal);
                if(debug()) cout << " ... assign string key" << endl;
                continue;
            }
        }

        //is this an int option?
        {
            map<string,int*>::iterator it = intOpts.find(key);
            if(intOpts.end() != it )
            {
                *(it->second) = atoi(val.c_str());
                if(debug()) cout << " ... assign int key" << endl;
                continue;
            }
        }

        //is this a bool option?
        {
            map<string,bool*>::iterator it = boolOpts.find(key);
            if(boolOpts.end() != it)
            {
                *(it->second) = atoi(val.c_str()); //todo:support true/false
                if(debug()) cout << " ... assign bool key" << endl;
                continue;
            }
        }

        //is this a double option?
        {
            map<string,double*>::iterator it = doubleOpts.find(key);
            if(doubleOpts.end() != it)
            {
                *(it->second) = atof(val.c_str()); //todo:support true/false
                if(debug()) cout << " ... assign double key" << endl;
                continue;
            }
        }

        if(debug()) cout << " ... key not found.  handle error?" << endl;
    }

    m_isInit = true;
    return true;
}



std::string JobOptsSvc::ExpandEnvironmentals( const std::string& input ) const
{
    // expand any environment variables in the file name
    wordexp_t exp_result;
    if(wordexp(input.c_str(), &exp_result, 0) != 0)
    {
        //this is a fatal error so throw
        cout << "JobOptsSvc::init - ERROR - Your string '" << input << "' cannot be understood!" << endl;
        throw 1;
    }

    const string output( exp_result.we_wordv[0]);
    if(debug()) cout << "JobOptsSvc::ExpandEnvironmentals - expanded " << input << " -> " << output << endl;

    return output;
}

std::string JobOptsSvc::GetRoadsFilePlusTop() const
{
    if(debug()) cout << "JobOptsSvc::GetRoadsFilePlusTop" << endl;
    return Form( "%s/firmware/roads/L1/%d/roads_plus_top.txt", m_triggerRepo.c_str(), m_triggerL1 );
}

std::string JobOptsSvc::GetRoadsFilePlusBottom() const
{
    if(debug()) cout << "JobOptsSvc::GetRoadsFilePlusBottom" << endl;
    return Form( "%s/firmware/roads/L1/%d/roads_plus_bottom.txt", m_triggerRepo.c_str(), m_triggerL1 );
}


std::string JobOptsSvc::GetRoadsFileMinusTop() const
{
    if(debug()) cout << "JobOptsSvc::GetRoadsFileMinusTop" << endl;
    return Form( "%s/firmware/roads/L1/%d/roads_minus_top.txt", m_triggerRepo.c_str(), m_triggerL1 );
}


std::string JobOptsSvc::GetRoadsFileMinusBottom() const
{
    if(debug()) cout << "JobOptsSvc::GetRoadsFileMinusBottom" << endl;
    return Form( "%s/firmware/roads/L1/%d/roads_minus_bottom.txt", m_triggerRepo.c_str(), m_triggerL1 );
}

std::string JobOptsSvc::GetInputMySQLURL() const
{
    return Form( "mysql://%s:%d", m_mySQLInputServer.c_str(), m_mySQLInputPort);
}

std::string JobOptsSvc::GetOutputMySQLURL() const
{
    return Form( "mysql://%s:%d", m_mySQLOutputServer.c_str(), m_mySQLOutputPort);
}

bool JobOptsSvc::ProcessAllEvents() const
{
    //if we are skipping any events or stopping after some number,
    // then we are NOT processing all events
    if( m_firstEvent > 0 )
        return false;
    if( m_nEvents > 0 )
        return false;

    return true;
}

void JobOptsSvc::save(TFile* saveFile, TString name)
{
    saveFile->cd();
    TTree* saveTree = new TTree(name.Data(), name.Data());

    TString s_configFile = m_configFile;

    int s_mcMode = m_mcMode;
    int s_alignmentMode = m_alignmentMode;
    int s_enableTriggerMask = m_enableTriggerMask;
    int s_enableKMag = m_enableKMag;
    int s_enableOnlineAlignment = m_enableOnlineAlignment;
    int s_enableEvaluation = m_enableEvaluation;
    int s_attachRawEvent = m_attachRawEvent;
    int s_sagittaReducer = m_sagittaReducer;
    int s_updateAlignment = m_updateAlignment;
    int s_hodomask = m_hodomask;
    int s_mergeHodo = m_mergeHodo;
    int s_realization = m_realization;

    TString s_mySQLInputURL = GetInputMySQLURL();
    TString s_mySQLOutputURL = GetOutputMySQLURL();
    int s_nevents = m_nEvents;
    int s_firstEvent = m_firstEvent;

    TString s_roadsPT = GetRoadsFilePlusTop();
    TString s_roadsPB = GetRoadsFilePlusBottom();
    TString s_roadsMT = GetRoadsFileMinusTop();
    TString s_roadsMB = GetRoadsFileMinusBottom();

    TString s_inputFile = m_inputFile;
    TString s_outputFile = m_outputFile;

    TString s_alignmentFileHodo = m_alignmentFileHodo;
    TString s_alignmentFileCham = m_alignmentFileChamber;
    TString s_alignmentFileProp = m_alignmentFileProp;
    TString s_alignmentFileMille = m_alignmentFileMille;
    TString s_calibrationFile = m_calibrationsFile;

    TString s_fmagFile = m_fMagFile;
    TString s_kmagFile = m_kMagFile;
    TString s_fmagStr  = Form("%.3f", FMAGSTR);
    TString s_kmagStr  = Form("%.3f", KMAGSTR);

    TString s_st0Reject = Form("%.4f", m_st0_reject);
    TString s_st1Reject = Form("%.4f", m_st1_reject);
    TString s_st2Reject = Form("%.4f", m_st2_reject);
    TString s_st3pReject = Form("%.4f", m_st3p_reject);
    TString s_st3mReject = Form("%.4f", m_st3m_reject);

    TString s_x_vtx = Form("%.4f", X_VTX);
    TString s_y_vtx = Form("%.4f", Y_VTX);

    TString s_geomVersion = m_geomVersion;

#ifdef GIT_VERSION
    TString s_softver = GIT_VERSION;
#else
    TString s_softver = "Unknown";
#endif

    double s_timingOffset = m_timingOffset;

    saveTree->Branch("ConfigFile", &s_configFile);
    saveTree->Branch("MCMode", &s_mcMode);
    saveTree->Branch("AlignmentMode", &s_alignmentMode);
    saveTree->Branch("TriggerMask", &s_enableTriggerMask);
    saveTree->Branch("KMagON", &s_enableKMag);
    saveTree->Branch("OnlineAlignment", &s_enableOnlineAlignment);
    saveTree->Branch("Evaluation", &s_enableEvaluation);
    saveTree->Branch("AttachRaw", &s_attachRawEvent);
    saveTree->Branch("SagittaReducer", &s_sagittaReducer);
    saveTree->Branch("UpdateAlignment", &s_updateAlignment);
    saveTree->Branch("HodoMask", &s_hodomask);
    saveTree->Branch("MergeHodo", &s_mergeHodo);
    saveTree->Branch("Realization", &s_realization);
    saveTree->Branch("MySQLInput", &s_mySQLInputURL);
    saveTree->Branch("MySQLOutput", &s_mySQLOutputURL);
    saveTree->Branch("NEvents", &s_nevents);
    saveTree->Branch("FirstEvent", &s_firstEvent);
    saveTree->Branch("InputFile", &s_inputFile);
    saveTree->Branch("OutputFile", &s_outputFile);
    saveTree->Branch("RoadFilePT", &s_roadsPT);
    saveTree->Branch("RoadFilePB", &s_roadsPB);
    saveTree->Branch("RoadFileMT", &s_roadsMT);
    saveTree->Branch("RoadFileMB", &s_roadsMB);
    saveTree->Branch("AlignmentHodo", &s_alignmentFileHodo);
    saveTree->Branch("AlignmentCham", &s_alignmentFileCham);
    saveTree->Branch("AlignmentProp", &s_alignmentFileProp);
    saveTree->Branch("AlignmentMille", &s_alignmentFileMille);
    saveTree->Branch("Calibration", &s_calibrationFile);
    saveTree->Branch("FMag", &s_fmagFile);
    saveTree->Branch("KMag", &s_kmagFile);
    saveTree->Branch("FMagStr", &s_fmagStr);
    saveTree->Branch("KMagStr", &s_kmagStr);
    saveTree->Branch("Geometry", &s_geomVersion);
    saveTree->Branch("TimingOffset", &s_timingOffset);
    saveTree->Branch("kTrackerVer", &s_softver);

    saveTree->Branch("ST0RejectWin", &s_st0Reject);
    saveTree->Branch("ST1RejectWin", &s_st1Reject);
    saveTree->Branch("ST2RejectWin", &s_st2Reject);
    saveTree->Branch("ST3pRejectWin", &s_st3pReject);
    saveTree->Branch("ST3mRejectWin", &s_st3mReject);

    saveTree->Branch("X_VTX", &s_x_vtx);
    saveTree->Branch("Y_VTX", &s_y_vtx);

    saveTree->Fill();
    saveTree->Write();
}
