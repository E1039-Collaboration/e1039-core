/*! @brief Read a job options file and apply choices at runtime.
@author Brian G. Tice
*/

#ifndef _JOBOPTSSVC_H
#define _JOBOPTSSVC_H

#include <string>
#include <TFile.h>
#include <TString.h>

#include "GlobalConsts.h"

class JobOptsSvc
{
public:
    ///singlton instance
    static JobOptsSvc* instance();

    ///Constructor for default values
    JobOptsSvc();

    ///Initialization with defaults
    bool init(bool forceInit = false);
    ///Initialization using this config file
    bool init(const char* filename);
    ///Close the service and cleanup
    void close();

    ///Save the content to a TFile
    void save(TFile* saveFile, TString name = "config");

    ///Return a string with environmental variables expanded
    std::string ExpandEnvironmentals( const std::string& input ) const;

    ///Return the location of PlusTop roads file
    std::string GetRoadsFilePlusTop() const;
    ///Return the location of PlusBottom roads file
    std::string GetRoadsFilePlusBottom() const;
    ///Return the location of MinusTop roads file
    std::string GetRoadsFileMinusTop() const;
    ///Return the location of MinusBottom roads file
    std::string GetRoadsFileMinusBottom() const;

    ///Return the URL for the input MySQL server
    std::string GetInputMySQLURL() const;
    ///Return the URL for the output MySQL server
    std::string GetOutputMySQLURL() const;

    ///Returns true if all events will be processing, false if only a subset
    bool ProcessAllEvents() const;

    //@todo should store smart pointers instead of variable length variables

    std::string m_configFile; ///< Name of the config file loaded

    bool m_mcMode;           ///< Running on MC?
    bool m_alignmentMode;    ///< Running in alignment mode?
    bool m_enableTriggerMask;///< Enable hodo masking with trigger road info
    bool m_enableKMag;       ///< Turn kMag on
    bool m_enableOnlineAlignment;   ///< Get alignment params from database?
    bool m_enableEvaluation; ///< Enable evaluation output
    bool m_attachRawEvent;   ///< Attach the raw event in the reconstructed data
    bool m_sagittaReducer;   ///< Enable the sagitta ratio reducer
    bool m_updateAlignment;  ///< Update the alignment/calibration in tracking, i.e. re-calculate the wire position and drift distance during tracking
    bool m_hodomask;         ///< Enable hodoscope masking
    bool m_mergeHodo;        ///< merge the TW-TDC and v1495 TDC
    bool m_realization;      ///< randomly drop hits according to efficiency, and smear drift distance according to resolution

    int m_mySQLInputPort;    ///< mysql database input port
    int m_mySQLOutputPort;   ///< mysql database output port
    int m_nEvents;           ///< number of events to process
    int m_firstEvent;        ///< first event to process

    int m_triggerL1;         ///< L1 firmware version (trigger roads)

    double m_timingOffset;   ///< timing offset for chamber hits (mostly)

    double m_st0_reject;     ///< Station-0 rejection window
    double m_st1_reject;     ///< station-1 rejection window
    double m_st2_reject;     ///< Station-2 rejection window
    double m_st3p_reject;     ///< Station-3 rejection window
    double m_st3m_reject;     ///< Station-3 rejection window

    double m_x_vtx;          ///< Beam X offset
    double m_y_vtx;          ///< Beam Y offset
    double m_x_spot_size;    ///< Beam spot size in X
    double m_y_spot_size;    ///< Beam spot size in X

    std::string m_inputFile;  ///< Name of the input file
    std::string m_outputFile; ///< Name of the output file
    std::string m_inputSchema; ///< Name of the input schema
    std::string m_outputSchema; ///< Name of the output schema

    std::string m_alignmentFileHodo; ///< Name of hodoscope alignment file
    std::string m_alignmentFileChamber; ///< Name of chamber alignment file
    std::string m_alignmentFileProp; ///< Name of prop tune alignment file
    std::string m_alignmentFileMille; ///< Name of mille alignment file

    std::string m_triggerRepo;  ///< Location of the trigger repository on local disk

    std::string m_calibrationsFile; ///< Name of calibrations file

    std::string m_fMagFile; ///< Name of fMag ascii file
    std::string m_kMagFile; ///< Name of kMag ascii file

    std::string m_geomVersion; ///< Name of geometry version
    std::string m_mySQLInputServer;  ///< Name of input MySQL server
    std::string m_mySQLOutputServer;  ///< Name of output MySQL server

private:
    int m_isInit; ///< Has this service been initialized?
    static JobOptsSvc* p_jobOptsSvc; ///< Singleton pointer
};

#endif
