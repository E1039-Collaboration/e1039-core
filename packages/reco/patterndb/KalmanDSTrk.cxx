/**
 * \class KalmanDSTrk
 * \brief Pattern Dictionary Filter built based on Kun Liu's ktracker
 * \author Haiwang Yu, yuhw@nmsu.edu
 *
 * Created: 08-27-2018
 */


//#include "KalmanFitter.h"
#include "KalmanDSTrk.h"
#include "TriggerRoad.h"
#include "PatternDBUtil.h"


#include <phfield/PHField.h>
#include <phool/PHTimer.h>
#include <fun4all/Fun4AllBase.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <limits>

#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TBox.h>
#include <TMatrixD.h>
#include <TFile.h>
#include <TNtuple.h>

//#define _DEBUG_ON
#define _DEBUG_YUHW_

KalmanDSTrk::KalmanDSTrk(
		const PHField* field,
		const TGeoManager *geom,
		bool enable_KF,
		int  DS_level,
    const std::string sim_db_name,
    const std::string pattern_db_name
		)
: verbosity(10)
,_enable_KF(enable_KF)
,_DS_level(DS_level)
,_sim_db_name(sim_db_name)
,_pattern_db_name(pattern_db_name)
, _pattern_db(nullptr)
{
    using namespace std;

#ifdef _DEBUG_ON
    cout << "Initialization of KalmanDSTrk ..." << endl;
    cout << "========================================" << endl;
#endif

    _timers.insert(std::make_pair<std::string, PHTimer*>("build_db", new PHTimer("build_db")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("load_db", new PHTimer("load_db")));

    _timers.insert(std::make_pair<std::string, PHTimer*>("event", new PHTimer("event")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("st2", new PHTimer("st2")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("search_db_2", new PHTimer("search_db_2")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("st3", new PHTimer("st3")));

    _timers.insert(std::make_pair<std::string, PHTimer*>("st23", new PHTimer("st23")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("search_db_23", new PHTimer("search_db_23")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("st23_fit1", new PHTimer("st23_fit1")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("st23_prop", new PHTimer("st23_prop")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("st23_fit2", new PHTimer("st23_fit2")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("st23_hodo", new PHTimer("st23_hodo")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("st23_lr40", new PHTimer("st23_lr40")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("st23_lr150", new PHTimer("st23_lr150")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("st23_rm_hits", new PHTimer("st23_rm_hits")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("st23_acc", new PHTimer("st23_acc")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("st23_reduce", new PHTimer("st23_reduce")));

    _timers.insert(std::make_pair<std::string, PHTimer*>("global", new PHTimer("global")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("search_db_glb", new PHTimer("search_db_glb")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("global_st1", new PHTimer("global_st1")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("global_link", new PHTimer("global_link")));
    _timers.insert(std::make_pair<std::string, PHTimer*>("global_kalman", new PHTimer("global_kalman")));

    _timers.insert(std::make_pair<std::string, PHTimer*>("kalman", new PHTimer("kalman")));

    //Initialize jobOpts service
    p_jobOptsSvc = JobOptsSvc::instance();

    //Initialize Kalman fitter
    if(_enable_KF)
    {
        kmfitter = new KalmanFitter(field, geom);
        kmfitter->setControlParameter(50, 0.001);
    }

    //Load Patter DB from file


  	if(_DS_level > KalmanDSTrk::NO_DS) {

			if(_pattern_db_name != "") {
			  std::cout <<"KalmanDSTrk::KalmanDSTrk: load DB from pattern db: "<< _pattern_db_name << std::endl;
        _timers["load_db"]->restart();
			  _pattern_db = PatternDBUtil::LoadPatternDB(_pattern_db_name);
        _timers["load_db"]->stop();
			} else if(_sim_db_name != "") {
        std::cout <<"KalmanDSTrk::KalmanDSTrk: load DB from sim db: "<< _sim_db_name << std::endl;
			  _timers["build_db"]->restart();
        _pattern_db = new PatternDB();
        PatternDBUtil::BuildPatternDB(_sim_db_name, "PatternDB_tmp.root", *_pattern_db);
        _timers["build_db"]->stop();
			} else {
			  std::cout <<"KalmanDSTrk::KalmanDSTrk: no sim or pattern DB" << std::endl;
			}

			if(_pattern_db) {
				std::cout <<"KalmanDSTrk::KalmanDSTrk: DB loaded. St23 size: "<< _pattern_db->St23.size() << std::endl;
	      std::cout << "================================================================" << std::endl;
	      std::cout << "Build DB                    "<<_timers["build_db"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	      std::cout << "Load DB                     "<<_timers["load_db"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	      std::cout << "================================================================" << std::endl;
			} else {
				std::cout <<"KalmanDSTrk::KalmanDSTrk: DB NOT loaded - _DS_level set to 0 " << std::endl;
				_DS_level = 0;
			}
    }

    //Initialize minuit minimizer
    minimizer[0] = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Simplex");
    minimizer[1] = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Combined");
    fcn = ROOT::Math::Functor(&tracklet_curr, &Tracklet::Eval, p_jobOptsSvc->m_enableKMag ? 5 : 4);

    for(int i = 0; i < 2; ++i)
    {
        minimizer[i]->SetMaxFunctionCalls(1000000);
        minimizer[i]->SetMaxIterations(100);
        minimizer[i]->SetTolerance(1E-2);
        minimizer[i]->SetFunction(fcn);
        minimizer[i]->SetPrintLevel(0);
    }

    //Minimize ROOT output
    extern Int_t gErrorIgnoreLevel;
    gErrorIgnoreLevel = 9999;

    //Initialize geometry service
    p_geomSvc = GeomSvc::instance();
#ifdef _DEBUG_ON
    p_geomSvc->printTable();
    p_geomSvc->printWirePosition();
    p_geomSvc->printAlignPar();
#endif

    //Initialize plane angles for all planes
    for(int i = 1; i <= nChamberPlanes; ++i)
    {
        costheta_plane[i] = p_geomSvc->getCostheta(i);
        sintheta_plane[i] = p_geomSvc->getSintheta(i);
    }

    //Initialize hodoscope IDs
    detectorIDs_mask[0] = p_geomSvc->getDetectorIDs("H1");
    detectorIDs_mask[1] = p_geomSvc->getDetectorIDs("H2");
    detectorIDs_mask[2] = p_geomSvc->getDetectorIDs("H3");
    detectorIDs_mask[3] = p_geomSvc->getDetectorIDs("H4");
    detectorIDs_maskX[0] = p_geomSvc->getDetectorIDs("H1[TB]");
    detectorIDs_maskX[1] = p_geomSvc->getDetectorIDs("H2[TB]");
    detectorIDs_maskX[2] = p_geomSvc->getDetectorIDs("H3[TB]");
    detectorIDs_maskX[3] = p_geomSvc->getDetectorIDs("H4[TB]");
    detectorIDs_maskY[0] = p_geomSvc->getDetectorIDs("H1[LR]");
    detectorIDs_maskY[1] = p_geomSvc->getDetectorIDs("H2[LR]");
    detectorIDs_maskY[2] = p_geomSvc->getDetectorIDs("H4Y1[LR]");
    detectorIDs_maskY[3] = p_geomSvc->getDetectorIDs("H4Y2[LR]");
    detectorIDs_muidHodoAid[0] = p_geomSvc->getDetectorIDs("H4[TB]");
    detectorIDs_muidHodoAid[1] = p_geomSvc->getDetectorIDs("H4Y");

    //Register masking stations for tracklets in station-0/1, 2, 3+/-
    stationIDs_mask[0].push_back(1);
    stationIDs_mask[1].push_back(1);
    stationIDs_mask[2].push_back(2);
    stationIDs_mask[3].push_back(3);
    stationIDs_mask[4].push_back(3);

    //Masking stations for back partial
    stationIDs_mask[5].push_back(2);
    stationIDs_mask[5].push_back(3);
    stationIDs_mask[5].push_back(4);

    //Masking stations for global track
    stationIDs_mask[6].push_back(1);
    stationIDs_mask[6].push_back(2);
    stationIDs_mask[6].push_back(3);
    stationIDs_mask[6].push_back(4);

    //prop. tube IDs for mu id
    detectorIDs_muid[0][0] = p_geomSvc->getDetectorID("P1X1");
    detectorIDs_muid[0][1] = p_geomSvc->getDetectorID("P1X2");
    detectorIDs_muid[0][2] = p_geomSvc->getDetectorID("P2X1");
    detectorIDs_muid[0][3] = p_geomSvc->getDetectorID("P2X2");
    detectorIDs_muid[1][0] = p_geomSvc->getDetectorID("P1Y1");
    detectorIDs_muid[1][1] = p_geomSvc->getDetectorID("P1Y2");
    detectorIDs_muid[1][2] = p_geomSvc->getDetectorID("P2Y1");
    detectorIDs_muid[1][3] = p_geomSvc->getDetectorID("P2Y2");

    //Reference z_ref for mu id
    z_ref_muid[0][0] = MUID_Z_REF;
    z_ref_muid[0][1] = MUID_Z_REF;
    z_ref_muid[0][2] = 0.5*(p_geomSvc->getPlanePosition(detectorIDs_muid[0][0]) + p_geomSvc->getPlanePosition(detectorIDs_muid[0][1]));
    z_ref_muid[0][3] = z_ref_muid[0][2];

    z_ref_muid[1][0] = MUID_Z_REF;
    z_ref_muid[1][1] = MUID_Z_REF;
    z_ref_muid[1][2] = 0.5*(p_geomSvc->getPlanePosition(detectorIDs_muid[1][0]) + p_geomSvc->getPlanePosition(detectorIDs_muid[1][1]));
    z_ref_muid[1][3] = z_ref_muid[1][2];

    //Initialize masking window sizes, with optimized contingency
    for(int i = nChamberPlanes+1; i <= nChamberPlanes+nHodoPlanes+nPropPlanes; i++)
    {
        double factor = 0.;
        if(i > nChamberPlanes    && i <= nChamberPlanes+4)           factor = 0.25;    //for station-1 hodo
        if(i > nChamberPlanes+4  && i <= nChamberPlanes+8)           factor = 0.2;     //for station-2 hodo
        if(i > nChamberPlanes+8  && i <= nChamberPlanes+10)          factor = 0.15;    //for station-3 hodo
        if(i > nChamberPlanes+10 && i <= nChamberPlanes+nHodoPlanes) factor = 0.;      //for station-4 hodo
        if(i > nChamberPlanes+nHodoPlanes)                           factor = 0.15;    //for station-4 proptube

        z_mask[i-nChamberPlanes-1] = p_geomSvc->getPlanePosition(i);
        for(int j = 1; j <= p_geomSvc->getPlaneNElements(i); j++)
        {
            double x_min, x_max, y_min, y_max;
            p_geomSvc->get2DBoxSize(i, j, x_min, x_max, y_min, y_max);

            x_min -= (factor*(x_max - x_min));
            x_max += (factor*(x_max - x_min));
            y_min -= (factor*(y_max - y_min));
            y_max += (factor*(y_max - y_min));

            x_mask_min[i-nChamberPlanes-1][j-1] = x_min;
            x_mask_max[i-nChamberPlanes-1][j-1] = x_max;
            y_mask_min[i-nChamberPlanes-1][j-1] = y_min;
            y_mask_max[i-nChamberPlanes-1][j-1] = y_max;
        }
    }

#ifdef _DEBUG_ON
    cout << "========================" << endl;
    cout << "Hodo. masking settings: " << endl;
    for(int i = 0; i < 4; i++)
    {
        cout << "For station " << i+1 << endl;
        for(std::vector<int>::iterator iter = detectorIDs_mask[i].begin();  iter != detectorIDs_mask[i].end();  ++iter) cout << "All: " << *iter << endl;
        for(std::vector<int>::iterator iter = detectorIDs_maskX[i].begin(); iter != detectorIDs_maskX[i].end(); ++iter) cout << "X:   " << *iter << endl;
        for(std::vector<int>::iterator iter = detectorIDs_maskY[i].begin(); iter != detectorIDs_maskY[i].end(); ++iter) cout << "Y:   " << *iter << endl;
    }

    for(int i = 0; i < nStations; ++i)
    {
        std::cout << "Masking stations for tracklets with stationID = " << i + 1 << ": " << std::endl;
        for(std::vector<int>::iterator iter = stationIDs_mask[i].begin(); iter != stationIDs_mask[i].end(); ++iter)
        {
            std::cout << *iter << "  ";
        }
        std::cout << std::endl;
    }
#endif

    //Initialize super stationIDs
    for(int i = 0; i < nChamberPlanes/6+2; i++) superIDs[i].clear();
    superIDs[0].push_back((p_geomSvc->getDetectorIDs("D0X")[0] + 1)/2);
    superIDs[0].push_back((p_geomSvc->getDetectorIDs("D0U")[0] + 1)/2);
    superIDs[0].push_back((p_geomSvc->getDetectorIDs("D0V")[0] + 1)/2);
    superIDs[1].push_back((p_geomSvc->getDetectorIDs("D1X")[0] + 1)/2);
    superIDs[1].push_back((p_geomSvc->getDetectorIDs("D1U")[0] + 1)/2);
    superIDs[1].push_back((p_geomSvc->getDetectorIDs("D1V")[0] + 1)/2);
    superIDs[2].push_back((p_geomSvc->getDetectorIDs("D2X")[0] + 1)/2);
    superIDs[2].push_back((p_geomSvc->getDetectorIDs("D2U")[0] + 1)/2);
    superIDs[2].push_back((p_geomSvc->getDetectorIDs("D2V")[0] + 1)/2);
    superIDs[3].push_back((p_geomSvc->getDetectorIDs("D3pX")[0] + 1)/2);
    superIDs[3].push_back((p_geomSvc->getDetectorIDs("D3pU")[0] + 1)/2);
    superIDs[3].push_back((p_geomSvc->getDetectorIDs("D3pV")[0] + 1)/2);
    superIDs[4].push_back((p_geomSvc->getDetectorIDs("D3mX")[0] + 1)/2);
    superIDs[4].push_back((p_geomSvc->getDetectorIDs("D3mU")[0] + 1)/2);
    superIDs[4].push_back((p_geomSvc->getDetectorIDs("D3mV")[0] + 1)/2);

    superIDs[5].push_back((p_geomSvc->getDetectorIDs("P1X")[0] + 1)/2);
    superIDs[5].push_back((p_geomSvc->getDetectorIDs("P2X")[0] + 1)/2);
    superIDs[6].push_back((p_geomSvc->getDetectorIDs("P1Y")[0] + 1)/2);
    superIDs[6].push_back((p_geomSvc->getDetectorIDs("P2Y")[0] + 1)/2);

#ifdef _DEBUG_ON
    cout << "=============" << endl;
    cout << "Chamber IDs: " << endl;
    TString stereoNames[3] = {"X", "U", "V"};
    for(int i = 0; i < nChamberPlanes/6; i++)
    {
        for(int j = 0; j < 3; j++) cout << i << "  " << stereoNames[j].Data() << ": " << superIDs[i][j] << endl;
    }

    cout << "Proptube IDs: " << endl;
    for(int i = nChamberPlanes/6; i < nChamberPlanes/6+2; i++)
    {
        for(int j = 0; j < 2; j++) cout << i << "  " << j << ": " << superIDs[i][j] << endl;
    }

    //Initialize widow sizes for X-U matching and z positions of all chambers
    cout << "======================" << endl;
    cout << "U plane window sizes: " << endl;
#endif

    double u_factor[] = {5., 5., 5., 15., 15.};
    for(int i = 0; i < nChamberPlanes/6; i++)
    {
        int xID = 2*superIDs[i][0] - 1;
        int uID = 2*superIDs[i][1] - 1;
        int vID = 2*superIDs[i][2] - 1;
        double spacing = p_geomSvc->getPlaneSpacing(uID);
        double x_span = p_geomSvc->getPlaneScaleY(uID);

        z_plane_x[i] = 0.5*(p_geomSvc->getPlanePosition(xID) + p_geomSvc->getPlanePosition(xID+1));
        z_plane_u[i] = 0.5*(p_geomSvc->getPlanePosition(uID) + p_geomSvc->getPlanePosition(uID+1));
        z_plane_v[i] = 0.5*(p_geomSvc->getPlanePosition(vID) + p_geomSvc->getPlanePosition(vID+1));

        u_costheta[i] = costheta_plane[uID];
        u_sintheta[i] = sintheta_plane[uID];

        //u_win[i] = fabs(0.5*x_span/(spacing/sintheta_plane[uID])) + 2.*spacing + u_factor[i];
        u_win[i] = fabs(0.5*x_span*sintheta_plane[uID]) + TX_MAX*fabs((z_plane_u[i] - z_plane_x[i])*u_costheta[i]) + TY_MAX*fabs((z_plane_u[i] - z_plane_x[i])*u_sintheta[i]) + 2.*spacing + u_factor[i];

#ifdef _DEBUG_ON
        cout << "Station " << i << ": " << xID << "  " << uID << "  " << vID << "  " << u_win[i] << endl;

        cout
				<< "x_span = " << x_span << endl
				<< "sintheta_plane = " << sintheta_plane[uID] << endl
				<< "u_sintheta = " << u_sintheta[i] << endl
				<< "z_plane_u[i] - z_plane_x[i] = " << z_plane_u[i] - z_plane_x[i] << endl
				<< "fabs(0.5*x_span*sintheta_plane[uID]) = " << fabs(0.5*x_span*sintheta_plane[uID]) << endl
				<< "TX_MAX*fabs((z_plane_u[i] - z_plane_x[i])*u_costheta[i]) = " << TX_MAX*fabs((z_plane_u[i] - z_plane_x[i])*u_costheta[i]) << endl
				<< "TY_MAX*fabs((z_plane_u[i] - z_plane_x[i])*u_sintheta[i]) = " << TY_MAX*fabs((z_plane_u[i] - z_plane_x[i])*u_sintheta[i]) << endl
				<< "2.*spacing = " << 2.*spacing << endl
				<< "u_factor[i] = " << u_factor[i] << endl;
#endif
    }

    //Initialize Z positions and maximum parameters of all planes
    for(int i = 1; i <= nChamberPlanes; i++)
    {
        z_plane[i] = p_geomSvc->getPlanePosition(i);
        slope_max[i] = costheta_plane[i]*TX_MAX + sintheta_plane[i]*TY_MAX;
        intersection_max[i] = costheta_plane[i]*X0_MAX + sintheta_plane[i]*Y0_MAX;

#ifdef COARSE_MODE
        resol_plane[i] = 3.*p_geomSvc->getPlaneSpacing(i)/sqrt(12.);
#else
        if(i <= 6)
        {
            resol_plane[i] = p_jobOptsSvc->m_st0_reject;
        }
        else if(i <= 12)
        {
            resol_plane[i] = p_jobOptsSvc->m_st1_reject;
        }
        else if(i <= 18)
        {
            resol_plane[i] = p_jobOptsSvc->m_st2_reject;
        }
        else if(i <= 24)
        {
            resol_plane[i] = p_jobOptsSvc->m_st3p_reject;
        }
        else
        {
            resol_plane[i] = p_jobOptsSvc->m_st3m_reject;
        }
#endif
        spacing_plane[i] = p_geomSvc->getPlaneSpacing(i);
    }

#ifdef _DEBUG_ON
    cout << "======================================" << endl;
    cout << "Maximum local slope and intersection: " << endl;
#endif
    for(int i = 1; i <= nChamberPlanes/2; i++)
    {
        double d_slope = (p_geomSvc->getPlaneResolution(2*i - 1) + p_geomSvc->getPlaneResolution(2*i))/(z_plane[2*i] - z_plane[2*i-1]);
        double d_intersection = d_slope*z_plane[2*i];

        slope_max[2*i-1] += d_slope;
        intersection_max[2*i-1] += d_intersection;
        slope_max[2*i] += d_slope;
        intersection_max[2*i] += d_intersection;

#ifdef _DEBUG_ON
        cout << "Super plane " << i << ": " << slope_max[2*i-1] << "  " << intersection_max[2*i-1] << endl;
#endif
    }

    //Initialize sagitta ratios, index 0, 1, 2 are for X, U, V, this is the incrementing order of plane type
    s_detectorID[0] = p_geomSvc->getDetectorID("D2X");
    s_detectorID[1] = p_geomSvc->getDetectorID("D2Up");
    s_detectorID[2] = p_geomSvc->getDetectorID("D2Vp");

    _ana_mode = true;
    if(_ana_mode) {
			_fana = TFile::Open("ktracker_ana.root","recreate");
			_tana_Event = new TNtuple("Event",  "Event",
					"NSt2:NSt3:NSt23:NSt1:NGlobal:NKalman:"
					"TSt2:TSt3:TSt23:TGKalman:TGlobal:TKalman:"
					"TEvent"
			);
			_tana_St1   = new TNtuple("St1",  "St1",   "in:DS:out");
			_tana_St2   = new TNtuple("St2",  "St2",   "in:DS:out");
			_tana_St3   = new TNtuple("St3",  "St3",   "in:DS:out");
			_tana_St23  = new TNtuple("St23", "St23",  "in:DS:out:time");
			_tana_St123 = new TNtuple("St123","St123", "in:DS:out:time");
    }

    _event = 0;
}

KalmanDSTrk::~KalmanDSTrk()
{
    if(_enable_KF) delete kmfitter;
    delete minimizer[0];
    delete minimizer[1];

    if(_ana_mode) {
			_fana->cd();
			_tana_Event->Write();
			_tana_St1->Write();
			_tana_St2->Write();
			_tana_St3->Write();
			_tana_St23->Write();
			_tana_St123->Write();
			_fana->Close();
    }
}

void KalmanDSTrk::setRawEventDebug(SRawEvent* event_input)
{
    rawEvent = event_input;
    hitAll = event_input->getAllHits();
}


int KalmanDSTrk::setRawEvent(SRawEvent* event_input)
{
	// reset all timers
	for(auto iter=_timers.begin(); iter!=_timers.end();++iter) {
		iter->second->reset();
	}

	_timers["event"]->restart();
	int ret = this->setRawEventWorker(event_input);
	_timers["event"]->stop();


  if(_ana_mode) {
		float data[] = {
			static_cast<float>(trackletsInSt[1].size()), // St2
			static_cast<float>(trackletsInSt[2].size()), // St3
			static_cast<float>(trackletsInSt[3].size()), // St23
			static_cast<float>(trackletsInSt[0].size()), // St1
			static_cast<float>(trackletsInSt[4].size()), // Global
			static_cast<float>(stracks.size()),
			static_cast<float>(_timers["st2"]->get_accumulated_time()/1000.),
			static_cast<float>(_timers["st3"]->get_accumulated_time()/1000.),
			static_cast<float>(_timers["st23"]->get_accumulated_time()/1000.),
			static_cast<float>(_timers["global_kalman"]->get_accumulated_time()/1000.),
			static_cast<float>(_timers["global"]->get_accumulated_time()/1000.),
			static_cast<float>(_timers["kalman"]->get_accumulated_time()/1000.),
			static_cast<float>(_timers["event"]->get_accumulated_time()/1000.),
  	};

  	_tana_Event->Fill(data);
  }

  ++_event;

	return ret;
}

int KalmanDSTrk::setRawEventWorker(SRawEvent* event_input)
{
    //Initialize tracklet lists
    for(int i = 0; i < 5; i++) trackletsInSt[i].clear();
    stracks.clear();

    //pre-tracking cuts
    rawEvent = event_input;
    if(!acceptEvent(rawEvent)) return TFEXIT_FAIL_MULTIPLICITY;
    hitAll = event_input->getAllHits();
#ifdef _DEBUG_ON
    for(std::vector<Hit>::iterator iter = hitAll.begin(); iter != hitAll.end(); ++iter) iter->print();
#endif

    //Initialize hodo and masking IDs
    for(int i = 0; i < 4; i++)
    {
        //std::cout << "For station " << i << std::endl;
        hitIDs_mask[i].clear();
        if(p_jobOptsSvc->m_mcMode || rawEvent->isFPGATriggered())
        {
            hitIDs_mask[i] = rawEvent->getHitsIndexInDetectors(detectorIDs_maskX[i]);
        }
        else
        {
            hitIDs_mask[i] = rawEvent->getHitsIndexInDetectors(detectorIDs_maskY[i]);
        }

        //for(std::list<int>::iterator iter = hitIDs_mask[i].begin(); iter != hitIDs_mask[i].end(); ++iter) std::cout << *iter << " " << hitAll[*iter].detectorID << " === ";
        //std::cout << std::endl;
    }

    //Initialize prop. tube IDs
    for(int i = 0; i < 2; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            hitIDs_muid[i][j].clear();
            hitIDs_muid[i][j] = rawEvent->getHitsIndexInDetector(detectorIDs_muid[i][j]);
        }
        hitIDs_muidHodoAid[i].clear();
        hitIDs_muidHodoAid[i] = rawEvent->getHitsIndexInDetectors(detectorIDs_muidHodoAid[i]);
    }

#ifndef ALIGNMENT_MODE
#ifdef _DEBUG_ON
        LogInfo("ALIGNMENT_MODE OFF");
#endif
    buildPropSegments();
    if(propSegs[0].empty() || propSegs[1].empty())
    {
#ifdef _DEBUG_ON
        LogInfo("Failed in prop tube segment building: " << propSegs[0].size() << ", " << propSegs[1].size());
#endif
        //return TFEXIT_FAIL_ROUGH_MUONID;
    }
#endif

    //Build tracklets in station 2, 3+, 3-
    _timers["st2"]->restart();
    buildTrackletsInStation(3, 1);   //3 for station-2, 1 for list position 1
    _timers["st2"]->stop();

    if(verbosity >= 2) {
    	LogInfo("NTracklets in St2: " << trackletsInSt[1].size());
    }

    if(trackletsInSt[1].empty())
    {
#ifdef _DEBUG_ON
        LogInfo("Failed in tracklet build at station 2");
#endif
        return TFEXIT_FAIL_ST2_TRACKLET;
    }

    _timers["st3"]->restart();
    buildTrackletsInStation(4, 2);   //4 for station-3+
    buildTrackletsInStation(5, 2);   //5 for station-3-
    _timers["st3"]->stop();

    if(verbosity >= 2) {
    	LogInfo("NTracklets in St3: " << trackletsInSt[2].size());
    }

    if(trackletsInSt[2].empty())
    {
#ifdef _DEBUG_ON
        LogInfo("Failed in tracklet build at station 3");
#endif
        return TFEXIT_FAIL_ST3_TRACKLET;
    }

    //Build back partial tracks in station 2, 3+ and 3-
    buildBackPartialTracks();

    if(verbosity >= 2) {
    	LogInfo("NTracklets St2+St3: " << trackletsInSt[3].size());
    }

    if(trackletsInSt[3].empty())
    {
#ifdef _DEBUG_ON
        LogInfo("Failed in connecting station-2 and 3 tracks");
#endif
        return TFEXIT_FAIL_BACKPARTIAL;
    }

    //Connect tracklets in station 2/3 and station 1 to form global tracks
    buildGlobalTracks();

    if(verbosity >= 2) {
    	LogInfo("NTracklets Global: " << trackletsInSt[4].size());
    }

#ifdef _DEBUG_ON
    for(int i = 0; i < 2; ++i)
    {
        std::cout << "=======================================================================================" << std::endl;
        LogInfo("Prop tube segments in " << (i == 0 ? "X-Z" : "Y-Z"));
        for(std::list<PropSegment>::iterator seg = propSegs[i].begin(); seg != propSegs[i].end(); ++seg)
        {
            seg->print();
        }
        std::cout << "=======================================================================================" << std::endl;
    }

    for(int i = 0; i <= 4; i++)
    {
        std::cout << "=======================================================================================" << std::endl;
        LogInfo("Final tracklets in station: " << i+1 << " is " << trackletsInSt[i].size());
        for(std::list<Tracklet>::iterator tracklet = trackletsInSt[i].begin(); tracklet != trackletsInSt[i].end(); ++tracklet)
        {
            tracklet->print();
        }
        std::cout << "=======================================================================================" << std::endl;
    }
#endif

    if(trackletsInSt[4].empty()) return TFEXIT_FAIL_GLOABL;
    if(!_enable_KF) return TFEXIT_SUCCESS;

    //Build kalman tracks
    _timers["kalman"]->restart();
    for(std::list<Tracklet>::iterator tracklet = trackletsInSt[4].begin(); tracklet != trackletsInSt[4].end(); ++tracklet)
    {
        SRecTrack strack = processOneTracklet(*tracklet);
        stracks.push_back(strack);
    }
    _timers["kalman"]->stop();

#ifdef _DEBUG_ON
    LogInfo(stracks.size() << " final tracks:");
    for(std::list<SRecTrack>::iterator strack = stracks.begin(); strack != stracks.end(); ++strack)
    {
        strack->print();
    }
#endif

    return TFEXIT_SUCCESS;
}

bool KalmanDSTrk::acceptEvent(SRawEvent* rawEvent)
{

	if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
    LogInfo("D0: " << rawEvent->getNHitsInD0());
    LogInfo("D1: " << rawEvent->getNHitsInD1());
    LogInfo("D2: " << rawEvent->getNHitsInD2());
    LogInfo("D3p: " << rawEvent->getNHitsInD3p());
    LogInfo("D3m: " << rawEvent->getNHitsInD3m());
    LogInfo("H1: " << rawEvent->getNHitsInDetectors(detectorIDs_maskX[0]));
    LogInfo("H2: " << rawEvent->getNHitsInDetectors(detectorIDs_maskX[1]));
    LogInfo("H3: " << rawEvent->getNHitsInDetectors(detectorIDs_maskX[2]));
    LogInfo("H4: " << rawEvent->getNHitsInDetectors(detectorIDs_maskX[3]));
    LogInfo("Prop:" << rawEvent->getNPropHitsAll());
    LogInfo("NRoadsPos: " << rawEvent->getNRoadsPos());
    LogInfo("NRoadsNeg: " << rawEvent->getNRoadsNeg());
	}

	if(rawEvent->getNHitsInD0() > 350) return false;
	if(rawEvent->getNHitsInD1() > 350) return false; // 31% - 58 hit per plane
	if(rawEvent->getNHitsInD2() > 170) return false; // 23% - 28 hit per plane
	if(rawEvent->getNHitsInD3p() > 140) return false;// 18% - 23 hit per plane
	if(rawEvent->getNHitsInD3m() > 140) return false;// 18% - 23 hit per plane

	/*
	if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[0]) > 15) return false;
	if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[1]) > 10) return false;
	if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[2]) > 10) return false;
	if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[3]) > 10) return false;
	if(rawEvent->getNPropHitsAll() > 300) return false;

	if(rawEvent->getNRoadsPos() > 5) return false;
	if(rawEvent->getNRoadsNeg() > 5) return false;
	*/

	return true;
}

void KalmanDSTrk::buildBackPartialTracks()
{
  _timers["st23"]->restart();
#ifndef ALIGNMENT_MODE
    //Temporary container for a simple chisq fit
    int nHitsX2, nHitsX3;
    double z_fit[4], x_fit[4];
    double a, b;
#endif

#ifdef _DEBUG_YUHW_
    std::map<std::string, int> counter = {
    		{"in", 0},
				{"DS", 0},
				{"out", 0}
    };
#endif

    for(std::list<Tracklet>::iterator tracklet3 = trackletsInSt[2].begin(); tracklet3 != trackletsInSt[2].end(); ++tracklet3)
    {
#ifndef ALIGNMENT_MODE
        //Extract the X hits only from station-3 tracks
        nHitsX3 = 0;
        for(std::list<SignedHit>::iterator ptr_hit = tracklet3->hits.begin(); ptr_hit != tracklet3->hits.end(); ++ptr_hit)
        {
            if(ptr_hit->hit.index < 0) continue;
            if(p_geomSvc->getPlaneType(ptr_hit->hit.detectorID) == 1)
            {
                z_fit[nHitsX3] = z_plane[ptr_hit->hit.detectorID];
                x_fit[nHitsX3] = ptr_hit->hit.pos;
                ++nHitsX3;
            }
        }
#endif
        Tracklet tracklet_best;
        for(std::list<Tracklet>::iterator tracklet2 = trackletsInSt[1].begin(); tracklet2 != trackletsInSt[1].end(); ++tracklet2)
        {
            if(fabs(tracklet2->tx - tracklet3->tx) > 0.15 || fabs(tracklet2->ty - tracklet3->ty) > 0.1) continue;
#ifdef _DEBUG_YUHW_
            counter["in"]++;
#endif
            // Pattern dictionary search
            if(_DS_level >= KalmanDSTrk::ST23_DS) {
              _timers["search_db_23"]->restart();
              bool matched = false;

            	auto key2  = PatternDBUtil::GetTrackletKey(*tracklet2, PatternDB::DC2);
            	auto key3p = PatternDBUtil::GetTrackletKey(*tracklet3, PatternDB::DC3p);
            	auto key3m = PatternDBUtil::GetTrackletKey(*tracklet3, PatternDB::DC3m);

            	PartTrackKey key23;
            	if(key2!=PatternDB::ERR_KEY and key3p!=PatternDB::ERR_KEY) {key23 = PartTrackKey(key2, key3p);}
            	else if(key2!=PatternDB::ERR_KEY and key3m!=PatternDB::ERR_KEY) {key23 = PartTrackKey(key2, key3m);}
            	else continue;

              //LogInfo(key23);
            	if(_pattern_db->St23.find(key23)!=_pattern_db->St23.end()) matched = true;

              _timers["search_db_23"]->stop();
            	if(!matched) {
            		if(Verbosity() > 20) {
            			LogInfo("St23 Pattern NOT Found!");
            		}
            		continue;
            	}

#ifdef _DEBUG_YUHW_
            	counter["DS"]++;
#endif
            }

#ifndef ALIGNMENT_MODE
            //Extract the X hits from station-2 tracke
            nHitsX2 = nHitsX3;
            for(std::list<SignedHit>::iterator ptr_hit = tracklet2->hits.begin(); ptr_hit != tracklet2->hits.end(); ++ptr_hit)
            {
                if(ptr_hit->hit.index < 0) continue;
                if(p_geomSvc->getPlaneType(ptr_hit->hit.detectorID) == 1)
                {
                    z_fit[nHitsX2] = z_plane[ptr_hit->hit.detectorID];
                    x_fit[nHitsX2] = ptr_hit->hit.pos;
                    ++nHitsX2;
                }
            }

            if(verbosity>2) _timers["st23_fit1"]->restart();
            //Apply a simple linear fit to get rough estimation of X-Z slope and intersection
            chi2fit(nHitsX2, z_fit, x_fit, a, b);
            if(verbosity>2) _timers["st23_fit1"]->stop();
            if(fabs(a) > 2.*TX_MAX || fabs(b) > 2.*X0_MAX) continue;

            if(verbosity>2) _timers["st23_prop"]->restart();
            //Project to proportional tubes to see if there is enough
            int nPropHits = 0;
            for(int i = 0; i < 4; ++i)
            {
                double x_exp = a*z_mask[detectorIDs_muid[0][i] - nChamberPlanes - 1] + b;
                for(std::list<int>::iterator iter = hitIDs_muid[0][i].begin(); iter != hitIDs_muid[0][i].end(); ++iter)
                {
                    if(fabs(hitAll[*iter].pos - x_exp) < 5.08)
                    {
                        ++nPropHits;
                        break;
                    }
                }
                if(nPropHits > 0) break;
            }
            if(verbosity>2) _timers["st23_prop"]->stop();
            if(nPropHits == 0) continue;
#endif

            Tracklet tracklet_23 = (*tracklet2) + (*tracklet3);
#ifdef _DEBUG_ON
            LogInfo("Using following two tracklets:");
            tracklet2->print();
            tracklet3->print();
            LogInfo("Yield this combination:");
            tracklet_23.print();
#endif
            if(verbosity>2) _timers["st23_fit2"]->restart();
            fitTracklet(tracklet_23);
            if(verbosity>2) _timers["st23_fit2"]->stop();

            if(tracklet_23.chisq > 9000.)
            {
#ifdef _DEBUG_ON
                tracklet_23.print();
                LogInfo("Impossible combination!");
#endif
                continue;
            }

            if(verbosity>2) _timers["st23_hodo"]->restart();
            if(!hodoMask(tracklet_23))
            {
            	if(verbosity>2) _timers["st23_hodo"]->stop();
#ifdef _DEBUG_ON
            	LogInfo("Hodomasking failed!");
#endif
            	continue;
            }
#ifdef _DEBUG_ON
            LogInfo("Hodomasking Scucess!");
#endif

#ifndef COARSE_MODE
            if(verbosity>2) _timers["st23_lr40"]->restart();
            resolveLeftRight(tracklet_23, 40.);
            if(verbosity>2) _timers["st23_lr40"]->stop();
            if(verbosity>2) _timers["st23_lr150"]->restart();
            resolveLeftRight(tracklet_23, 150.);
            if(verbosity>2) _timers["st23_lr150"]->stop();
#endif
            if(verbosity>2) _timers["st23_rm_hits"]->restart();
            ///Remove bad hits if needed
            removeBadHits(tracklet_23);
            if(verbosity>2) _timers["st23_rm_hits"]->stop();

#ifdef _DEBUG_ON
            LogInfo("New tracklet: ");
            tracklet_23.print();

            LogInfo("Current best:");
            tracklet_best.print();

            LogInfo("Comparison: " << (tracklet_23 < tracklet_best));
            LogInfo("Quality: " << acceptTracklet(tracklet_23));
#endif

            if(verbosity>2) _timers["st23_acc"]->restart();
            //If current tracklet is better than the best tracklet up-to-now
            if(acceptTracklet(tracklet_23) && tracklet_23 < tracklet_best)
            {
                tracklet_best = tracklet_23;
            }
#ifdef _DEBUG_YUHW_
            counter["out"]++;
#endif
            if(verbosity>2) _timers["st23_acc"]->stop();
#ifdef _DEBUG_ON
            else
            {
                LogInfo("Rejected!!");
            }
#endif
        }

        if(verbosity>2) _timers["st23_reduce"]->restart();
        if(tracklet_best.isValid()) trackletsInSt[3].push_back(tracklet_best);
        if(verbosity>2) _timers["st23_reduce"]->stop();
    }

    reduceTrackletList(trackletsInSt[3]);
    trackletsInSt[3].sort();

    _timers["st23"]->stop();

#ifdef _DEBUG_YUHW_
    if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
			LogInfo("");
			std::cout
			<< counter["in"] << " => "
			<< counter["DS"] << " => "
			<< counter["out"] << std::endl;
    }

    if(_ana_mode) {
			float tana_data[] = {
					static_cast<float>(counter["in"]),
					static_cast<float>(counter["DS"]),
					static_cast<float>(counter["out"]),
					static_cast<float>(_timers["st23"]->get_accumulated_time()/1000.)
			};
			_tana_St23->Fill(tana_data);
    }
#endif
}

void KalmanDSTrk::buildGlobalTracks()
{
  _timers["global"]->restart();

#ifdef _DEBUG_YUHW_
    std::map<std::string, int> counter = {
    		{"in", 0},
				{"DS", 0},
				{"out", 0}
    };
#endif

    double pos_exp[3], window[3];
    for(std::list<Tracklet>::iterator tracklet23 = trackletsInSt[3].begin(); tracklet23 != trackletsInSt[3].end(); ++tracklet23)
    {
        Tracklet tracklet_best[2];
        for(int i = 0; i < 2; ++i) //for two station-1 chambers
        {
            trackletsInSt[0].clear();

            //Calculate the window in station 1
            if(p_jobOptsSvc->m_enableKMag)
            {
                getSagittaWindowsInSt1(*tracklet23, pos_exp, window, i+1);
            }
            else
            {
                getExtrapoWindowsInSt1(*tracklet23, pos_exp, window, i+1);
            }

#ifdef _DEBUG_ON
            LogInfo("Using this back partial: ");
            tracklet23->print();
            for(int j = 0; j < 3; j++) LogInfo("Extrapo: " << pos_exp[j] << "  " << window[j]);
#endif

            _timers["global_st1"]->restart();
            buildTrackletsInStation(i+1, 0, pos_exp, window);
            _timers["global_st1"]->stop();

            _timers["global_link"]->restart();
            Tracklet tracklet_best_prob, tracklet_best_vtx;
            for(std::list<Tracklet>::iterator tracklet1 = trackletsInSt[0].begin(); tracklet1 != trackletsInSt[0].end(); ++tracklet1)
            {
#ifdef _DEBUG_ON
                LogInfo("With this station 1 track:");
                tracklet1->print();
#endif

#ifdef _DEBUG_YUHW_
                counter["in"]++;
#endif
                // Pattern dictionary search
                if(_DS_level >= KalmanDSTrk::ST123_DS) {
                  _timers["search_db_glb"]->restart();
                  bool matched = false;

                	auto key1  = PatternDBUtil::GetTrackletKey(*tracklet1, PatternDB::DC1);
                	auto key2  = PatternDBUtil::GetTrackletKey(*tracklet23, PatternDB::DC2);
                	auto key3p = PatternDBUtil::GetTrackletKey(*tracklet23, PatternDB::DC3p);
                	auto key3m = PatternDBUtil::GetTrackletKey(*tracklet23, PatternDB::DC3m);

                	GlobTrackKey key123;
                	if(
                			key1!=PatternDB::ERR_KEY
                			and key2!=PatternDB::ERR_KEY
											and key3p!=PatternDB::ERR_KEY) {
                		key123 = GlobTrackKey(key1, key2, key3p);
                	} else if (
                			key1!=PatternDB::ERR_KEY
                			and key2!=PatternDB::ERR_KEY
											and key3m!=PatternDB::ERR_KEY) {
                		key123 = GlobTrackKey(key1, key2, key3m);
                	} else continue;

                	//std::cout << key123;
                	if(_pattern_db->St123.find(key123)!=_pattern_db->St123.end()) matched = true;

                  _timers["search_db_glb"]->stop();


              		if(Verbosity() > 20) {
              			std::cout << key123;
              			if(matched) {
                			std::cout<< "St123 Pattern Found!";
              			} else {
                			std::cout<< "St123 Pattern NOT Found!";
              			}
              			std::cout << std::endl;
              		}

                	if(!matched) {
                		continue;
                	}

#ifdef _DEBUG_YUHW_
                	counter["DS"]++;
#endif
                }

                Tracklet tracklet_global = (*tracklet23) * (*tracklet1);
                fitTracklet(tracklet_global);
                if(!hodoMask(tracklet_global)) continue;

#ifndef COARSE_MODE
                ///Resolve the left-right with a tight pull cut, then a loose one, then resolve by single projections
                resolveLeftRight(tracklet_global, 75.);
                resolveLeftRight(tracklet_global, 150.);
                resolveSingleLeftRight(tracklet_global);
#endif
                ///Remove bad hits if needed
                removeBadHits(tracklet_global);

                //Most basic cuts
                if(!acceptTracklet(tracklet_global)) continue;

                //Get the tracklets that has the best prob
                if(tracklet_global < tracklet_best_prob) tracklet_best_prob = tracklet_global;

#if !defined(ALIGNMENT_MODE) && defined(_ENABLE_KF)
                ///Set vertex information
                _timers["global_kalman"]->restart();
                SRecTrack recTrack = processOneTracklet(tracklet_global);
                _timers["global_kalman"]->stop();
                tracklet_global.chisq_vtx = recTrack.getChisqVertex();

                if(recTrack.isValid() && tracklet_global.chisq_vtx < tracklet_best_vtx.chisq_vtx) tracklet_best_vtx = tracklet_global;
#endif

#ifdef _DEBUG_ON
                LogInfo("New tracklet: ");
                tracklet_global.print();

                LogInfo("Current best by prob:");
                tracklet_best_prob.print();

                LogInfo("Comparison I: " << (tracklet_global < tracklet_best_prob));
                LogInfo("Quality I   : " << acceptTracklet(tracklet_global));

#if !defined(ALIGNMENT_MODE) && defined(_ENABLE_KF)
                LogInfo("Current best by vtx:");
                tracklet_best_vtx.print();

                LogInfo("Comparison II: " << (tracklet_global.chisq_vtx < tracklet_best_vtx.chisq_vtx));
                LogInfo("Quality II   : " << recTrack.isValid());
#endif
#endif
            }
            _timers["global_link"]->stop();


#if !defined(ALIGNMENT_MODE) && defined(_ENABLE_KF)
            //The selection logic is, prefer the tracks with best p-value, as long as it's not low-pz
            if(tracklet_best_prob.isValid() && 1./tracklet_best_prob.invP > 18.)
            {
                tracklet_best[i] = tracklet_best_prob;
            }
            else if(tracklet_best_vtx.isValid()) //otherwise select the one with best vertex chisq, TODO: maybe add a z-vtx constraint
            {
                tracklet_best[i] = tracklet_best_vtx;
            }
            else if(tracklet_best_prob.isValid()) //then fall back to the default only choice
            {
                tracklet_best[i] = tracklet_best_prob;
            }
#else
            if(tracklet_best_prob.isValid()) //then fall back to the default only choice
            {
                tracklet_best[i] = tracklet_best_prob;
            }
#endif
        }

        //Merge the tracklets from two stations if necessary
        Tracklet tracklet_merge;
        if(fabs(tracklet_best[0].getMomentum() - tracklet_best[1].getMomentum())/tracklet_best[0].getMomentum() < MERGE_THRES)
        {
            //Merge the track and re-fit
            tracklet_merge = tracklet_best[0].merge(tracklet_best[1]);
            fitTracklet(tracklet_merge);

#ifdef _DEBUG_ON
            LogInfo("Merging two track candidates with momentum: " << tracklet_best[0].getMomentum() << "  " << tracklet_best[1].getMomentum());
            LogInfo("tracklet_best_1:"); tracklet_best[0].print();
            LogInfo("tracklet_best_2:"); tracklet_best[1].print();
            LogInfo("tracklet_merge:"); tracklet_merge.print();
#endif
        }

        if(tracklet_merge.isValid() && tracklet_merge < tracklet_best[0] && tracklet_merge < tracklet_best[1])
        {
#ifdef _DEBUG_ON
            LogInfo("Choose merged tracklet");
#endif
#ifdef _DEBUG_YUHW_
            counter["out"]++;
#endif
            trackletsInSt[4].push_back(tracklet_merge);
        }
        else if(tracklet_best[0].isValid() && tracklet_best[0] < tracklet_best[1])
        {
#ifdef _DEBUG_ON
            LogInfo("Choose tracklet with station-0");
#endif
#ifdef _DEBUG_YUHW_
            counter["out"]++;
#endif
            trackletsInSt[4].push_back(tracklet_best[0]);
        }
        else if(tracklet_best[1].isValid())
        {
#ifdef _DEBUG_ON
            LogInfo("Choose tracklet with station-1");
#endif
#ifdef _DEBUG_YUHW_
            counter["out"]++;
#endif
            trackletsInSt[4].push_back(tracklet_best[1]);
        }
    }

    trackletsInSt[4].sort();

    _timers["global"]->stop();

#ifdef _DEBUG_YUHW_
    if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
			LogInfo("");
			std::cout
			<< counter["in"] << " => "
			<< counter["DS"] << " => "
			<< counter["out"] << std::endl;
    }

		if(_ana_mode) {
			float tana_data[] = {
					static_cast<float>(counter["in"]),
					static_cast<float>(counter["DS"]),
					static_cast<float>(counter["out"]),
					static_cast<float>(_timers["global"]->get_accumulated_time()/1000.)
			};
			_tana_St123->Fill(tana_data);
		}
#endif
}

void KalmanDSTrk::resolveLeftRight(Tracklet& tracklet, double threshold)
{
#ifdef _DEBUG_ON
    LogInfo("Left right for this track..");
    tracklet.print();
#endif

    //Check if the track has been updated
    bool isUpdated = false;

    //Four possibilities
    int possibility[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    //Total number of hit pairs in this tracklet
    int nPairs = tracklet.hits.size()/2;

    int nResolved = 0;
    std::list<SignedHit>::iterator hit1 = tracklet.hits.begin();
    std::list<SignedHit>::iterator hit2 = tracklet.hits.begin();
    ++hit2;
    while(true)
    {
#ifdef _DEBUG_ON
        LogInfo(hit1->hit.index << "  " << hit2->sign << " === " << hit2->hit.index << "  " << hit2->sign);
        int detectorID1 = hit1->hit.detectorID;
        int detectorID2 = hit2->hit.detectorID;
        LogInfo("Hit1: " << tracklet.getExpPositionX(z_plane[detectorID1])*costheta_plane[detectorID1] + tracklet.getExpPositionY(z_plane[detectorID1])*sintheta_plane[detectorID1] << "  " << hit1->hit.pos + hit1->hit.driftDistance << "  " << hit1->hit.pos - hit1->hit.driftDistance);
        LogInfo("Hit2: " << tracklet.getExpPositionX(z_plane[detectorID2])*costheta_plane[detectorID2] + tracklet.getExpPositionY(z_plane[detectorID2])*sintheta_plane[detectorID2] << "  " << hit2->hit.pos + hit2->hit.driftDistance << "  " << hit2->hit.pos - hit2->hit.driftDistance);
#endif

        if(hit1->hit.index > 0 && hit2->hit.index > 0 && hit1->sign*hit2->sign == 0)
        {
            int index_min = -1;
            double pull_min = 1E6;
            for(int i = 0; i < 4; i++)
            {
                double slope_local = (hit1->pos(possibility[i][0]) - hit2->pos(possibility[i][1]))/(z_plane[hit1->hit.detectorID] - z_plane[hit2->hit.detectorID]);
                double inter_local = hit1->pos(possibility[i][0]) - slope_local*z_plane[hit1->hit.detectorID];

                if(fabs(slope_local) > slope_max[hit1->hit.detectorID] || fabs(inter_local) > intersection_max[hit1->hit.detectorID]) continue;

                double tx, ty, x0, y0;
                double err_tx, err_ty, err_x0, err_y0;
                if(tracklet.stationID == 6 && hit1->hit.detectorID <= 6)
                {
                    tracklet.getXZInfoInSt1(tx, x0);
                    tracklet.getXZErrorInSt1(err_tx, err_x0);
                }
                else
                {
                    tx = tracklet.tx;
                    x0 = tracklet.x0;
                    err_tx = tracklet.err_tx;
                    err_x0 = tracklet.err_x0;
                }
                ty = tracklet.ty;
                y0 = tracklet.y0;
                err_ty = tracklet.err_ty;
                err_y0 = tracklet.err_y0;

                double slope_exp = costheta_plane[hit1->hit.detectorID]*tx + sintheta_plane[hit1->hit.detectorID]*ty;
                double err_slope = fabs(costheta_plane[hit1->hit.detectorID]*err_tx) + fabs(sintheta_plane[hit2->hit.detectorID]*err_ty);
                double inter_exp = costheta_plane[hit1->hit.detectorID]*x0 + sintheta_plane[hit1->hit.detectorID]*y0;
                double err_inter = fabs(costheta_plane[hit1->hit.detectorID]*err_x0) + fabs(sintheta_plane[hit2->hit.detectorID]*err_y0);

                double pull = sqrt((slope_exp - slope_local)*(slope_exp - slope_local)/err_slope/err_slope + (inter_exp - inter_local)*(inter_exp - inter_local)/err_inter/err_inter);
                if(pull < pull_min)
                {
                    index_min = i;
                    pull_min = pull;
                }

#ifdef _DEBUG_ON
                LogInfo(hit1->hit.detectorID << ": " << i << "  " << possibility[i][0] << "  " << possibility[i][1]);
                LogInfo(tx << "  " << x0 << "  " << ty << "  " << y0);
                LogInfo("Slope: " << slope_local << "  " << slope_exp << "  " << err_slope);
                LogInfo("Intersection: " << inter_local << "  " << inter_exp << "  " << err_inter);
                LogInfo("Current: " << pull << "  " << index_min << "  " << pull_min);
#endif
            }

            //LogInfo("Final: " << index_min << "  " << pull_min);
            if(index_min >= 0 && pull_min < threshold)//((tracklet.stationID == 5 && pull_min < 25.) || (tracklet.stationID == 6 && pull_min < 100.)))
            {
            	//Origin imp.
							hit1->sign = possibility[index_min][0];
							hit2->sign = possibility[index_min][1];
              isUpdated = true;
            }
        }

        ++nResolved;
        if(nResolved >= nPairs) break;

        ++hit1;
        ++hit1;
        ++hit2;
        ++hit2;
    }
#ifdef _DEBUG_ON
    tracklet.print();
#endif
    if(isUpdated) fitTracklet(tracklet);
#ifdef _DEBUG_ON
    tracklet.print();
#endif
}

void KalmanDSTrk::resolveSingleLeftRight(Tracklet& tracklet)
{
#ifdef _DEBUG_ON
    LogInfo("Single left right for this track..");
    tracklet.print();
#endif

    //Check if the track has been updated
    bool isUpdated = false;
    for(std::list<SignedHit>::iterator hit_sign = tracklet.hits.begin(); hit_sign != tracklet.hits.end(); ++hit_sign)
    {
        if(hit_sign->hit.index < 0 || hit_sign->sign != 0) continue;

        int detectorID = hit_sign->hit.detectorID;
        double pos_exp = tracklet.getExpPositionX(z_plane[detectorID])*costheta_plane[detectorID] + tracklet.getExpPositionY(z_plane[detectorID])*sintheta_plane[detectorID];
        hit_sign->sign = pos_exp > hit_sign->hit.pos ? 1 : -1;

        isUpdated = true;
    }

    if(isUpdated) fitTracklet(tracklet);
}

void KalmanDSTrk::removeBadHits(Tracklet& tracklet)
{
#ifdef _DEBUG_ON
    LogInfo("Removing hits for this track..");
    tracklet.calcChisq();
    tracklet.print();
#endif

    //Check if the track has beed updated
    int signflipflag[nChamberPlanes];
    for(int i = 0; i < nChamberPlanes; ++i) signflipflag[i] = 0;

    bool isUpdated = true;
    while(isUpdated)
    {
        isUpdated = false;
        tracklet.calcChisq();

        SignedHit* hit_remove = NULL;
        SignedHit* hit_neighbour = NULL;
        double res_remove1 = -1.;
        double res_remove2 = -1.;
        for(std::list<SignedHit>::iterator hit_sign = tracklet.hits.begin(); hit_sign != tracklet.hits.end(); ++hit_sign)
        {
            if(hit_sign->hit.index < 0) continue;

            int detectorID = hit_sign->hit.detectorID;
            double res_curr = fabs(tracklet.residual[detectorID-1]);
            if(res_remove1 < res_curr)
            {
                res_remove1 = res_curr;
                res_remove2 = fabs(tracklet.residual[detectorID-1] - 2.*hit_sign->sign*hit_sign->hit.driftDistance);
                hit_remove = &(*hit_sign);

                std::list<SignedHit>::iterator iter = hit_sign;
                hit_neighbour = detectorID % 2 == 0 ? &(*(--iter)) : &(*(++iter));
            }
        }
        if(hit_remove == NULL) continue;
        if(hit_remove->sign == 0 && tracklet.isValid()) continue;  //if sign is undecided, and chisq is OKay, then pass

        double cut = hit_remove->sign == 0 ? hit_remove->hit.driftDistance + resol_plane[hit_remove->hit.detectorID] : resol_plane[hit_remove->hit.detectorID];
        if(res_remove1 > cut)
        {
#ifdef _DEBUG_ON
            LogInfo("Dropping this hit: " << res_remove1 << "  " << res_remove2 << "   " << signflipflag[hit_remove->hit.detectorID-1] << "  " << cut);
            hit_remove->hit.print();
            hit_neighbour->hit.print();
#endif

            //can only be changed less than twice
            if(res_remove2 < cut && signflipflag[hit_remove->hit.detectorID-1] < 2)
            {
                hit_remove->sign = -hit_remove->sign;
                hit_neighbour->sign = 0;
                ++signflipflag[hit_remove->hit.detectorID-1];
#ifdef _DEBUG_ON
                LogInfo("Only changing the sign.");
#endif
            }
            else
            {
                //Set the index of the hit to be removed to -1 so it's not used anymore
                //also set the sign assignment of the neighbour hit to 0 (i.e. undecided)
                hit_remove->hit.index = -1;
                hit_neighbour->sign = 0;
                int planeType = p_geomSvc->getPlaneType(hit_remove->hit.detectorID);
                if(planeType == 1)
                {
                    --tracklet.nXHits;
                }
                else if(planeType == 2)
                {
                    --tracklet.nUHits;
                }
                else
                {
                    --tracklet.nVHits;
                }

                //If both hit pairs are not included, the track can be rejected
                if(hit_neighbour->hit.index < 0)
                {
#ifdef _DEBUG_ON
                    LogInfo("Both hits in a view are missing! Will exit the bad hit removal...");
#endif
                    return;
                }
            }
            isUpdated = true;
        }

        if(isUpdated)
        {
            fitTracklet(tracklet);
            resolveSingleLeftRight(tracklet);
        }
    }
}

void KalmanDSTrk::resolveLeftRight(SRawEvent::hit_pair hpair, int& LR1, int& LR2)
{
    LR1 = 0;
    LR2 = 0;

    //If either hit is missing, no left-right can be assigned
    if(hpair.first < 0 || hpair.second < 0)
    {
        return;
    }

    int possibility[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    int nResolved = 0;
    for(int i = 0; i < 4; i++)
    {
        if(nResolved > 1) break;

        int hitID1 = hpair.first;
        int hitID2 = hpair.second;
        double slope_local = (hitAll[hitID1].pos + possibility[i][0]*hitAll[hitID1].driftDistance - hitAll[hitID2].pos - possibility[i][1]*hitAll[hitID2].driftDistance)/(z_plane[hitAll[hitID1].detectorID] - z_plane[hitAll[hitID2].detectorID]);
        double intersection_local = hitAll[hitID1].pos + possibility[i][0]*hitAll[hitID1].driftDistance - slope_local*z_plane[hitAll[hitID1].detectorID];

        //LogInfo(i << "  " << nResolved << "  " << slope_local << "  " << intersection_local);
        if(fabs(slope_local) < slope_max[hitAll[hitID1].detectorID] && fabs(intersection_local) < intersection_max[hitAll[hitID1].detectorID])
        {
            nResolved++;
            LR1 = possibility[i][0];
            LR2 = possibility[i][1];
        }
    }

    if(nResolved > 1)
    {
        LR1 = 0;
        LR2 = 0;
    }

    //LogInfo("Final: " << LR1 << "  " << LR2);
}

void KalmanDSTrk::buildTrackletsInStation(int stationID, int listID, double* pos_exp, double* window)
{
#ifdef _DEBUG_ON
    LogInfo("Building tracklets in station " << stationID);
#endif

    //actuall ID of the tracklet lists
    int sID = stationID - 1;

    //Extract the X, U, V hit pairs
    std::list<SRawEvent::hit_pair> pairs_X, pairs_U, pairs_V;
    if(pos_exp == NULL)
    {
        pairs_X = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][0]);
        pairs_U = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][1]);
        pairs_V = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][2]);
    }
    else
    {
        //Note that in pos_exp[], index 0 stands for X, index 1 stands for U, index 2 stands for V
        pairs_X = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][0], pos_exp[0], window[0]);
        pairs_U = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][1], pos_exp[1], window[1]);
        pairs_V = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][2], pos_exp[2], window[2]);
    }

#ifdef _DEBUG_ON_
    LogInfo("Hit pairs in this event: ");
    for(std::list<SRawEvent::hit_pair>::iterator iter = pairs_X.begin(); iter != pairs_X.end(); ++iter) LogInfo("X :" << iter->first << "  " << iter->second << "  " << hitAll[iter->first].index << " " << (iter->second < 0 ? -1 : hitAll[iter->second].index));
    for(std::list<SRawEvent::hit_pair>::iterator iter = pairs_U.begin(); iter != pairs_U.end(); ++iter) LogInfo("U :" << iter->first << "  " << iter->second << "  " << hitAll[iter->first].index << " " << (iter->second < 0 ? -1 : hitAll[iter->second].index));
    for(std::list<SRawEvent::hit_pair>::iterator iter = pairs_V.begin(); iter != pairs_V.end(); ++iter) LogInfo("V :" << iter->first << "  " << iter->second << "  " << hitAll[iter->first].index << " " << (iter->second < 0 ? -1 : hitAll[iter->second].index));
#endif

    if(pairs_X.empty() || pairs_U.empty() || pairs_V.empty())
    {
#ifdef _DEBUG_ON
        LogInfo("Not all view has hits in station " << stationID);
#endif
        return;
    }

#ifdef _DEBUG_YUHW_
    std::map<std::string, int> counter = {
    		{"in", 0},
				{"DS", 0},
				{"out", 0}
    };
#endif

    //X-U combination first, then add V pairs
    for(std::list<SRawEvent::hit_pair>::iterator xiter = pairs_X.begin(); xiter != pairs_X.end(); ++xiter)
    {
        //U projections from X plane
        double x_pos = xiter->second >= 0 ? 0.5*(hitAll[xiter->first].pos + hitAll[xiter->second].pos) : hitAll[xiter->first].pos;
        double u_min = x_pos*u_costheta[sID] - u_win[sID];
        double u_max = u_min + 2.*u_win[sID];

#ifdef _DEBUG_ON
        LogInfo("Trying X hits " << xiter->first << "  " << xiter->second << "  " << hitAll[xiter->first].elementID << " at " << x_pos);
        LogInfo("U plane window:" << u_min << "  " << u_max);
#endif
        for(std::list<SRawEvent::hit_pair>::iterator uiter = pairs_U.begin(); uiter != pairs_U.end(); ++uiter)
        {
            double u_pos = uiter->second >= 0 ? 0.5*(hitAll[uiter->first].pos + hitAll[uiter->second].pos) : hitAll[uiter->first].pos;
#ifdef _DEBUG_ON
            LogInfo("Trying U hits " << uiter->first << "  " << uiter->second << "  " << hitAll[uiter->first].elementID << " at " << u_pos);
#endif
            if(u_pos < u_min || u_pos > u_max) continue;

            //V projections from X and U plane
            double z_x = xiter->second >= 0 ? z_plane_x[sID] : z_plane[hitAll[xiter->first].detectorID];
            double z_u = uiter->second >= 0 ? z_plane_u[sID] : z_plane[hitAll[uiter->first].detectorID];
            double z_v = z_plane_v[sID];
            double v_win1 = spacing_plane[hitAll[uiter->first].detectorID]*2.*u_costheta[sID];
            double v_win2 = fabs((z_u + z_v - 2.*z_x)*u_costheta[sID]*TX_MAX);
            double v_win3 = fabs((z_v - z_u)*u_sintheta[sID]*TY_MAX);
            double v_win = v_win1 + v_win2 + v_win3 + 2.*spacing_plane[hitAll[uiter->first].detectorID];
            double v_min = 2*x_pos*u_costheta[sID] - u_pos - v_win;
            double v_max = v_min + 2.*v_win;

#ifdef _DEBUG_ON
            LogInfo("V plane window:" << v_min << "  " << v_max);

            std::cout
						<< "v_win1 = " << v_win1 << std::endl
						<< "v_win2 = " << v_win2 << std::endl
						<< "v_win3 = " << v_win3 << std::endl
						<< "2.*spacing_plane[hitAll[uiter->first].detectorID] = " << 2.*spacing_plane[hitAll[uiter->first].detectorID] << std::endl;
#endif
            for(std::list<SRawEvent::hit_pair>::iterator viter = pairs_V.begin(); viter != pairs_V.end(); ++viter)
            {
#ifdef _DEBUG_YUHW_
            	counter["in"]++;
#endif
            	if(_DS_level >= KalmanDSTrk::IN_ST_DS) {
            		if(stationID==3)
                  _timers["search_db_2"]->restart();

            		PatternDB::STATION station = PatternDB::ERROR_STATION;
            		if(stationID==1 or stationID==2) station = PatternDB::DC1;
            		if(stationID==3) station = PatternDB::DC2;
            		if(stationID==4) station = PatternDB::DC3p;
            		if(stationID==5) station = PatternDB::DC3m;
            		bool matched = false;

            		std::vector< std::pair<unsigned int, unsigned int> > det_elem_pairs;

            		det_elem_pairs.push_back({hitAll[xiter->first].detectorID,  hitAll[xiter->first].elementID});
            		det_elem_pairs.push_back({hitAll[xiter->second].detectorID, hitAll[xiter->second].elementID});
            		det_elem_pairs.push_back({hitAll[uiter->first].detectorID,  hitAll[uiter->first].elementID});
            		det_elem_pairs.push_back({hitAll[uiter->second].detectorID, hitAll[uiter->second].elementID});
            		det_elem_pairs.push_back({hitAll[viter->first].detectorID,  hitAll[viter->first].elementID});
            		det_elem_pairs.push_back({hitAll[viter->second].detectorID, hitAll[viter->second].elementID});

            		TrackletKey key = PatternDBUtil::GetTrackletKey(det_elem_pairs, station);

            		if(station == PatternDB::DC1
            				and _pattern_db->St1.find(key)!=_pattern_db->St1.end()) matched = true;
            		else if(station == PatternDB::DC2
            				and _pattern_db->St2.find(key)!=_pattern_db->St2.end()) matched = true;
            		else if( ( station == PatternDB::DC3p or station == PatternDB::DC3m )
            				and _pattern_db->St3.find(key)!=_pattern_db->St3.end()) matched = true;

            		if(stationID==3)
                  _timers["search_db_2"]->stop();

            		if(Verbosity() > 20) {
            			std::cout
									<< hitAll[xiter->first].index << ", "
									<< hitAll[xiter->second].index << ", "
									<< hitAll[uiter->first].index << ", "
									<< hitAll[uiter->second].index << ", "
									<< hitAll[viter->first].index << ", "
									<< hitAll[viter->second].index
									<< std::endl;

            			std::cout << key;
            			if(matched) {
              			std::cout<< "St" << station << " Pattern Found!";
            			} else {
              			std::cout<< "St" << station << " Pattern NOT Found!";
            			}
            			std::cout << std::endl;
            		}

            		if(!matched) {
            			continue;
            		}

#ifdef _DEBUG_YUHW_
              	counter["DS"]++;
#endif
            	}

                double v_pos = viter->second >= 0 ? 0.5*(hitAll[viter->first].pos + hitAll[viter->second].pos) : hitAll[viter->first].pos;
#ifdef _DEBUG_ON
                LogInfo("Trying V hits " << viter->first << "  " << viter->second << "  " << hitAll[viter->first].elementID << " at " << v_pos);
#endif
                if(v_pos < v_min || v_pos > v_max) continue;

                //Now add the tracklet
                int LR1 = 0;
                int LR2 = 0;
                Tracklet tracklet_new;
                tracklet_new.stationID = stationID;

                //resolveLeftRight(*xiter, LR1, LR2);
                if(xiter->first >= 0)
                {
                    tracklet_new.hits.push_back(SignedHit(hitAll[xiter->first], LR1));
                    tracklet_new.nXHits++;
                }
                if(xiter->second >= 0)
                {
                    tracklet_new.hits.push_back(SignedHit(hitAll[xiter->second], LR2));
                    tracklet_new.nXHits++;
                }

                //resolveLeftRight(*uiter, LR1, LR2);
                if(uiter->first >= 0)
                {
                    tracklet_new.hits.push_back(SignedHit(hitAll[uiter->first], LR1));
                    tracklet_new.nUHits++;
                }
                if(uiter->second >= 0)
                {
                    tracklet_new.hits.push_back(SignedHit(hitAll[uiter->second], LR2));
                    tracklet_new.nUHits++;
                }

                //resolveLeftRight(*viter, LR1, LR2);
                if(viter->first >= 0)
                {
                    tracklet_new.hits.push_back(SignedHit(hitAll[viter->first], LR1));
                    tracklet_new.nVHits++;
                }
                if(viter->second >= 0)
                {
                    tracklet_new.hits.push_back(SignedHit(hitAll[viter->second], LR2));
                    tracklet_new.nVHits++;
                }

                tracklet_new.sortHits();
                if(!tracklet_new.isValid())
                {
                    fitTracklet(tracklet_new);
                }
                else
                {
                    continue;
                }
#ifdef _DEBUG_ON
                if(Verbosity()>=Fun4AllBase::VERBOSITY_A_LOT) {
                	tracklet_new.print();
                }
#endif
                if(acceptTracklet(tracklet_new))
                {
                    trackletsInSt[listID].push_back(tracklet_new);
#ifdef _DEBUG_YUHW_
                    counter["out"]++;
#endif
                }
#ifdef _DEBUG_ON
                else
                {
                    LogInfo("Rejected!!!");
                }
#endif
            }
        }
    }

#ifdef _DEBUG_YUHW_
    if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
			LogInfo("");
			std::cout
			<< counter["in"] << " => "
			<< counter["DS"] << " => "
			<< counter["out"] << std::endl;
    }

    if(_ana_mode) {
			float tana_data[] = {
					static_cast<float>(counter["in"]),
					static_cast<float>(counter["DS"]),
					static_cast<float>(counter["out"])
			};
			if(stationID==2) _tana_St1->Fill(tana_data);
			if(stationID==3) _tana_St2->Fill(tana_data);
			if(stationID==4||stationID==5) _tana_St3->Fill(tana_data);
    }
#endif

    //Reduce the tracklet list and add dummy hits
    //reduceTrackletList(trackletsInSt[listID]);
    for(std::list<Tracklet>::iterator iter = trackletsInSt[listID].begin(); iter != trackletsInSt[listID].end(); ++iter)
    {
        iter->addDummyHits();
    }

    //Only retain the best 200 tracklets if exceeded
    if(trackletsInSt[listID].size() > 200)
    {
        trackletsInSt[listID].sort();
        trackletsInSt[listID].resize(200);
    }
}

bool KalmanDSTrk::acceptTracklet(Tracklet& tracklet)
{
    //Tracklet itself is okay with enough hits (4-out-of-6) and small chi square
    if(!tracklet.isValid())
    {
#ifdef _DEBUG_ON
        LogInfo("Failed in quality check!");
#endif
        return false;
    }

    //Hodoscope masking requirement
    if(!hodoMask(tracklet)) return false;

    //For back partials, require projection inside KMAG, and muon id in prop. tubes
    if(tracklet.stationID > nStations-2)
    {
        if(!p_geomSvc->isInKMAG(tracklet.getExpPositionX(Z_KMAG_BEND), tracklet.getExpPositionY(Z_KMAG_BEND))) return false;
#ifndef ALIGNMENT_MODE
        if(!(muonID_comp(tracklet) || muonID_search(tracklet))) return false;
#endif
    }

    //If everything is fine ...
#ifdef _DEBUG_ON
    LogInfo("AcceptTracklet!!!");
#endif
    return true;
}

bool KalmanDSTrk::hodoMask(Tracklet& tracklet)
{
    //LogInfo(tracklet.stationID);
    int nHodoHits = 0;
    for(std::vector<int>::iterator stationID = stationIDs_mask[tracklet.stationID-1].begin(); stationID != stationIDs_mask[tracklet.stationID-1].end(); ++stationID)
    {
        bool masked = false;
        for(std::list<int>::iterator iter = hitIDs_mask[*stationID-1].begin(); iter != hitIDs_mask[*stationID-1].end(); ++iter)
        {
            int detectorID = hitAll[*iter].detectorID;
            int elementID = hitAll[*iter].elementID;

            int idx1 = detectorID - nChamberPlanes - 1;
            int idx2 = elementID - 1;

            double factor = tracklet.stationID == nChamberPlanes/6-2 ? 5. : 3.;   //special for station-2, based on real data tuning
            double xfudge = tracklet.stationID < nStations-1 ? 0.5*(x_mask_max[idx1][idx2] - x_mask_min[idx1][idx2]) : 0.15*(x_mask_max[idx1][idx2] - x_mask_min[idx1][idx2]);
            double z_hodo = z_mask[idx1];
            double x_hodo = tracklet.getExpPositionX(z_hodo);
            double y_hodo = tracklet.getExpPositionY(z_hodo);
            double err_x = factor*tracklet.getExpPosErrorX(z_hodo) + xfudge;
            double err_y = factor*tracklet.getExpPosErrorY(z_hodo);

            double x_min = x_mask_min[idx1][idx2] - err_x;
            double x_max = x_mask_max[idx1][idx2] + err_x;
            double y_min = y_mask_min[idx1][idx2] - err_y;
            double y_max = y_mask_max[idx1][idx2] + err_y;

#ifdef _DEBUG_ON
            LogInfo(*iter);
            hitAll[*iter].print();
            LogInfo(nHodoHits << "/" << stationIDs_mask[tracklet.stationID-1].size() << ":  " << z_hodo << "  " << x_hodo << " +/- " << err_x << "  " << y_hodo << " +/-" << err_y << " : " << x_min << "  " << x_max << "  " << y_min << "  " << y_max);
#endif
            if(x_hodo > x_min && x_hodo < x_max && y_hodo > y_min && y_hodo < y_max)
            {
                nHodoHits++;
                masked = true;

                break;
            }
        }

        if(!masked) return false;
    }

#ifdef _DEBUG_ON
    LogInfo(tracklet.stationID << "  " << nHodoHits << "  " << stationIDs_mask[tracklet.stationID-1].size());
#endif
    return true;
}

bool KalmanDSTrk::muonID_search(Tracklet& tracklet)
{
    //Set the cut value on multiple scattering
    //multiple scattering: sigma = 0.0136*sqrt(L/L0)*(1. + 0.038*ln(L/L0))/P, L = 1m, L0 = 1.76cm
    double cut = 0.03;
    if(tracklet.stationID == nStations)
    {
        double cut_the = MUID_THE_P0*tracklet.invP;
        double cut_emp = MUID_EMP_P0 + MUID_EMP_P1/tracklet.invP + MUID_EMP_P2/tracklet.invP/tracklet.invP;
        cut = MUID_REJECT*(cut_the > cut_emp ? cut_the : cut_emp);
    }

    double slope[2] = {tracklet.tx, tracklet.ty};
    double pos_absorb[2] = {tracklet.getExpPositionX(MUID_Z_REF), tracklet.getExpPositionY(MUID_Z_REF)};
    PropSegment* segs[2] = {&(tracklet.seg_x), &(tracklet.seg_y)};
    for(int i = 0; i < 2; ++i)
    {
        //this shorting circuting can only be done to X-Z, Y-Z needs more complicated thing
        //if(i == 0 && segs[i]->getNHits() > 2 && segs[i]->isValid() && fabs(slope[i] - segs[i]->a) < cut) continue;

        segs[i]->init();
        for(int j = 0; j < 4; ++j)
        {
            int index = detectorIDs_muid[i][j] - nChamberPlanes - 1;
            double pos_ref = j < 2 ? pos_absorb[i] : segs[i]->getPosRef(pos_absorb[i] + slope[i]*(z_ref_muid[i][j] - MUID_Z_REF));
            double pos_exp = slope[i]*(z_mask[index] - z_ref_muid[i][j]) + pos_ref;

            if(!p_geomSvc->isInPlane(detectorIDs_muid[i][j], tracklet.getExpPositionX(z_mask[index]), tracklet.getExpPositionY(z_mask[index]))) continue;

            double win_tight = cut*(z_mask[index] - z_ref_muid[i][j]);
            win_tight = win_tight > 2.54 ? win_tight : 2.54;
            double win_loose = win_tight*2;
            double dist_min = 1E6;
            for(std::list<int>::iterator iter = hitIDs_muid[i][j].begin(); iter != hitIDs_muid[i][j].end(); ++iter)
            {
                double pos = hitAll[*iter].pos;
                double dist = pos - pos_exp;
                if(dist < -win_loose) continue;
                if(dist > win_loose) break;

                double dist_l = fabs(pos - hitAll[*iter].driftDistance - pos_exp);
                double dist_r = fabs(pos + hitAll[*iter].driftDistance - pos_exp);
                dist = dist_l < dist_r ? dist_l : dist_r;
                if(dist < dist_min)
                {
                    dist_min = dist;
                    if(dist < win_tight)
                    {
                        segs[i]->hits[j].hit = hitAll[*iter];
                        segs[i]->hits[j].sign = fabs(pos - hitAll[*iter].driftDistance - pos_exp) < fabs(pos + hitAll[*iter].driftDistance - pos_exp) ? -1 : 1;
                    }
                }
            }
        }
        segs[i]->fit();

        //this shorting circuting can only be done to X-Z, Y-Z needs more complicated thing
        //if(i == 0 && !(segs[i]->isValid() && fabs(slope[i] - segs[i]->a) < cut)) return false;
    }

    muonID_hodoAid(tracklet);
    if(segs[0]->getNHits() + segs[1]->getNHits() >= 5)
    {
        return true;
    }
    else if(segs[1]->getNHits() == 1 || segs[1]->getNPlanes() == 1)
    {
        return segs[1]->nHodoHits >= 2;
    }
    return false;
}

bool KalmanDSTrk::muonID_comp(Tracklet& tracklet)
{
    //Set the cut value on multiple scattering
    //multiple scattering: sigma = 0.0136*sqrt(L/L0)*(1. + 0.038*ln(L/L0))/P, L = 1m, L0 = 1.76cm
    double cut = 0.03;
    if(tracklet.stationID == nStations)
    {
        double cut_the = MUID_THE_P0*tracklet.invP;
        double cut_emp = MUID_EMP_P0 + MUID_EMP_P1/tracklet.invP + MUID_EMP_P2/tracklet.invP/tracklet.invP;
        cut = MUID_REJECT*(cut_the > cut_emp ? cut_the : cut_emp);
    }
#ifdef _DEBUG_ON
    LogInfo("Muon ID cut is: " << cut << " rad.");
#endif

    double slope[2] = {tracklet.tx, tracklet.ty};
    PropSegment* segs[2] = {&(tracklet.seg_x), &(tracklet.seg_y)};

    for(int i = 0; i < 2; ++i)
    {
#ifdef _DEBUG_ON
        if(i == 0) LogInfo("Working in X-Z:");
        if(i == 1) LogInfo("Working in Y-Z:");
#endif

        double pos_ref = i == 0 ? tracklet.getExpPositionX(MUID_Z_REF) : tracklet.getExpPositionY(MUID_Z_REF);
        if(segs[i]->getNHits() > 2 && segs[i]->isValid() && fabs(slope[i] - segs[i]->a) < cut && fabs(segs[i]->getExpPosition(MUID_Z_REF) - pos_ref) < MUID_R_CUT)
        {
#ifdef _DEBUG_ON
            LogInfo("Muon ID are already avaiable!");
#endif
            continue;
        }

        for(std::list<PropSegment>::iterator iter = propSegs[i].begin(); iter != propSegs[i].end(); ++iter)
        {
#ifdef _DEBUG_ON
            LogInfo("Testing this prop segment, with ref pos = " << pos_ref << ", slope_ref = " << slope[i]);
            iter->print();
#endif
            if(fabs(iter->a - slope[i]) < cut && fabs(iter->getExpPosition(MUID_Z_REF) - pos_ref) < MUID_R_CUT)
            {
                *(segs[i]) = *iter;
#ifdef _DEBUG_ON
                LogInfo("Accepted!");
#endif
                break;
            }
        }

        if(!segs[i]->isValid()) return false;
    }

    if(segs[0]->getNHits() + segs[1]->getNHits() < 5) return false;
    return true;
}

bool KalmanDSTrk::muonID_hodoAid(Tracklet& tracklet)
{
    double win = 0.03;
    double factor = 5.;
    if(tracklet.stationID == nStations)
    {
        double win_the = MUID_THE_P0*tracklet.invP;
        double win_emp = MUID_EMP_P0 + MUID_EMP_P1/tracklet.invP + MUID_EMP_P2/tracklet.invP/tracklet.invP;
        win = MUID_REJECT*(win_the > win_emp ? win_the : win_emp);
        factor = 3.;
    }

    PropSegment* segs[2] = {&(tracklet.seg_x), &(tracklet.seg_y)};
    for(int i = 0; i < 2; ++i)
    {
        segs[i]->nHodoHits = 0;
        for(std::list<int>::iterator iter = hitIDs_muidHodoAid[i].begin(); iter != hitIDs_muidHodoAid[i].end(); ++iter)
        {
            int detectorID = hitAll[*iter].detectorID;
            int elementID = hitAll[*iter].elementID;

            int idx1 = detectorID - nChamberPlanes - 1;
            int idx2 = elementID - 1;

            double z_hodo = z_mask[idx1];
            double x_hodo = tracklet.getExpPositionX(z_hodo);
            double y_hodo = tracklet.getExpPositionY(z_hodo);
            double err_x = factor*tracklet.getExpPosErrorX(z_hodo) + win*(z_hodo - MUID_Z_REF);
            double err_y = factor*tracklet.getExpPosErrorY(z_hodo) + win*(z_hodo - MUID_Z_REF);

            err_x = err_x/(x_mask_max[idx1][idx2] - x_mask_min[idx1][idx2]) > 0.25 ? 0.25*err_x/(x_mask_max[idx1][idx2] - x_mask_min[idx1][idx2]) : err_x;
            err_y = err_y/(y_mask_max[idx1][idx2] - y_mask_min[idx1][idx2]) > 0.25 ? 0.25*err_y/(y_mask_max[idx1][idx2] - y_mask_min[idx1][idx2]) : err_y;

            double x_min = x_mask_min[idx1][idx2] - err_x;
            double x_max = x_mask_max[idx1][idx2] + err_x;
            double y_min = y_mask_min[idx1][idx2] - err_y;
            double y_max = y_mask_max[idx1][idx2] + err_y;

            if(x_hodo > x_min && x_hodo < x_max && y_hodo > y_min && y_hodo < y_max)
            {
                segs[i]->hodoHits[segs[i]->nHodoHits++] = hitAll[*iter];
                if(segs[i]->nHodoHits > 4) break;
            }
        }
    }

    return true;
}

void KalmanDSTrk::buildPropSegments()
{
#ifdef _DEBUG_ON
    LogInfo("Building prop. tube segments");
#endif

    for(int i = 0; i < 2; ++i)
    {
        propSegs[i].clear();

        //note for prop tubes superID index starts from 4
        std::list<SRawEvent::hit_pair> pairs_forward  = rawEvent->getPartialHitPairsInSuperDetector(superIDs[i+5][0]);
        std::list<SRawEvent::hit_pair> pairs_backward = rawEvent->getPartialHitPairsInSuperDetector(superIDs[i+5][1]);

#ifdef _DEBUG_ON
        std::cout << "superID: " << superIDs[i+5][0] << ", " << superIDs[i+5][1] << std::endl;
        for(std::list<SRawEvent::hit_pair>::iterator iter = pairs_forward.begin(); iter != pairs_forward.end(); ++iter)
        	LogInfo("Forward: " << iter->first << "  " << iter->second << "  " << hitAll[iter->first].index << "  " << (iter->second < 0 ? -1 : hitAll[iter->second].index));
        for(std::list<SRawEvent::hit_pair>::iterator iter = pairs_backward.begin(); iter != pairs_backward.end(); ++iter)
        	LogInfo("Backward: " << iter->first << "  " << iter->second << "  " << hitAll[iter->first].index << "  " << (iter->second < 0 ? -1 : hitAll[iter->second].index));
#endif

        for(std::list<SRawEvent::hit_pair>::iterator fiter = pairs_forward.begin(); fiter != pairs_forward.end(); ++fiter)
        {
#ifdef _DEBUG_ON
            LogInfo("Trying forward pair " << fiter->first << "  " << fiter->second);
#endif
            for(std::list<SRawEvent::hit_pair>::iterator biter = pairs_backward.begin(); biter != pairs_backward.end(); ++biter)
            {
#ifdef _DEBUG_ON
                LogInfo("Trying backward pair " << biter->first << "  " << biter->second);
#endif

                PropSegment seg;

                //Note that the backward plane comes as the first in pair
                if(fiter->first >= 0) seg.hits[1] = SignedHit(hitAll[fiter->first], 0);
                if(fiter->second >= 0) seg.hits[0] = SignedHit(hitAll[fiter->second], 0);
                if(biter->first >= 0) seg.hits[3] = SignedHit(hitAll[biter->first], 0);
                if(biter->second >= 0) seg.hits[2] = SignedHit(hitAll[biter->second], 0);

#ifdef _DEBUG_ON
                seg.print();
#endif
                seg.fit();
#ifdef _DEBUG_ON
                seg.print();
#endif

                if(seg.isValid())
                {
                    propSegs[i].push_back(seg);
                }
#ifdef _DEBUG_ON
                else
                {
                    LogInfo("Rejected!");
                }
#endif
            }
        }
    }
}


int KalmanDSTrk::fitTracklet(Tracklet& tracklet)
{
    tracklet_curr = tracklet;

    //idx = 0, using simplex; idx = 1 using migrad
    int idx = 1;
#ifdef _ENABLE_MULTI_MINI
    if(tracklet.stationID < nStations-1) idx = 0;
#endif

    minimizer[idx]->SetLimitedVariable(0, "tx", tracklet.tx, 0.001, -TX_MAX, TX_MAX);
    minimizer[idx]->SetLimitedVariable(1, "ty", tracklet.ty, 0.001, -TY_MAX, TY_MAX);
    minimizer[idx]->SetLimitedVariable(2, "x0", tracklet.x0, 0.1, -X0_MAX, X0_MAX);
    minimizer[idx]->SetLimitedVariable(3, "y0", tracklet.y0, 0.1, -Y0_MAX, Y0_MAX);
    if(p_jobOptsSvc->m_enableKMag)
    {
        minimizer[idx]->SetLimitedVariable(4, "invP", tracklet.invP, 0.001*tracklet.invP, INVP_MIN, INVP_MAX);
    }
    minimizer[idx]->Minimize();

    tracklet.tx = minimizer[idx]->X()[0];
    tracklet.ty = minimizer[idx]->X()[1];
    tracklet.x0 = minimizer[idx]->X()[2];
    tracklet.y0 = minimizer[idx]->X()[3];

    tracklet.err_tx = minimizer[idx]->Errors()[0];
    tracklet.err_ty = minimizer[idx]->Errors()[1];
    tracklet.err_x0 = minimizer[idx]->Errors()[2];
    tracklet.err_y0 = minimizer[idx]->Errors()[3];

    if(p_jobOptsSvc->m_enableKMag && tracklet.stationID == nStations)
    {
        tracklet.invP = minimizer[idx]->X()[4];
        tracklet.err_invP = minimizer[idx]->Errors()[4];
    }

    tracklet.chisq = minimizer[idx]->MinValue();

    int status = minimizer[idx]->Status();
    return status;
}

int KalmanDSTrk::reduceTrackletList(std::list<Tracklet>& tracklets)
{
    std::list<Tracklet> targetList;

    tracklets.sort();
    while(!tracklets.empty())
    {
        targetList.push_back(tracklets.front());
        tracklets.pop_front();

#ifdef _DEBUG_ON_LEVEL_2
        LogInfo("Current best tracklet in reduce");
        targetList.back().print();
#endif

        for(std::list<Tracklet>::iterator iter = tracklets.begin(); iter != tracklets.end(); )
        {
            if(iter->similarity(targetList.back()))
            {
#ifdef _DEBUG_ON_LEVEL_2
                LogInfo("Removing this tracklet: ");
                iter->print();
#endif
                iter = tracklets.erase(iter);
                continue;
            }
            else
            {
                ++iter;
            }
        }
    }

    tracklets.assign(targetList.begin(), targetList.end());
    return 0;
}

void KalmanDSTrk::getExtrapoWindowsInSt1(Tracklet& tracklet, double* pos_exp, double* window, int st1ID)
{
    if(tracklet.stationID != nStations-1)
    {
        for(int i = 0; i < 3; i++)
        {
            pos_exp[i] = 9999.;
            window[i] = 0.;
        }
        return;
    }

    for(int i = 0; i < 3; i++)
    {
        int detectorID = (st1ID-1)*6 + 2*i + 2;
        int idx = p_geomSvc->getPlaneType(detectorID) - 1;

        double z_st1 = z_plane[detectorID];
        double x_st1 = tracklet.getExpPositionX(z_st1);
        double y_st1 = tracklet.getExpPositionY(z_st1);
        double err_x = tracklet.getExpPosErrorX(z_st1);
        double err_y = tracklet.getExpPosErrorY(z_st1);

        pos_exp[idx] = p_geomSvc->getUinStereoPlane(detectorID, x_st1, y_st1);
        window[idx]  = 5.*(fabs(costheta_plane[detectorID]*err_x) + fabs(sintheta_plane[detectorID]*err_y));
    }
}

void KalmanDSTrk::getSagittaWindowsInSt1(Tracklet& tracklet, double* pos_exp, double* window, int st1ID)
{
    if(tracklet.stationID != nStations-1)
    {
        for(int i = 0; i < 3; i++)
        {
            pos_exp[i] = 9999.;
            window[i] = 0.;
        }
        return;
    }

    double z_st3 = z_plane[tracklet.hits.back().hit.detectorID];
    double x_st3 = tracklet.getExpPositionX(z_st3);
    double y_st3 = tracklet.getExpPositionY(z_st3);

    //For U, X, and V planes
    for(int i = 0; i < 3; i++)
    {
        int detectorID = (st1ID-1)*6 + 2*i + 2;
        int idx = p_geomSvc->getPlaneType(detectorID) - 1;

        if(!(idx >= 0 && idx <3)) continue;

        double pos_st3 = p_geomSvc->getUinStereoPlane(s_detectorID[idx], x_st3, y_st3);

        double z_st1 = z_plane[detectorID];
        double z_st2 = z_plane[s_detectorID[idx]];
        double x_st2 = tracklet.getExpPositionX(z_st2);
        double y_st2 = tracklet.getExpPositionY(z_st2);
        double pos_st2 = p_geomSvc->getUinStereoPlane(s_detectorID[idx], x_st2, y_st2);

        double s2_target = pos_st2 - pos_st3*(z_st2 - Z_TARGET)/(z_st3 - Z_TARGET);
        double s2_dump   = pos_st2 - pos_st3*(z_st2 - Z_DUMP)/(z_st3 - Z_DUMP);

        double pos_exp_target = SAGITTA_TARGET_CENTER*s2_target + pos_st3*(z_st1 - Z_TARGET)/(z_st3 - Z_TARGET);
        double pos_exp_dump   = SAGITTA_DUMP_CENTER*s2_dump + pos_st3*(z_st1 - Z_DUMP)/(z_st3 - Z_DUMP);
        double win_target = fabs(s2_target*SAGITTA_TARGET_WIN);
        double win_dump   = fabs(s2_dump*SAGITTA_DUMP_WIN);

        double p_min = std::min(pos_exp_target - win_target, pos_exp_dump - win_dump);
        double p_max = std::max(pos_exp_target + win_target, pos_exp_dump + win_dump);

        pos_exp[idx] = 0.5*(p_max + p_min);
        window[idx]  = 0.5*(p_max - p_min);
    }
}

void KalmanDSTrk::printAtDetectorBack(int stationID, std::string outputFileName)
{
    TCanvas c1;

    std::vector<double> x, y, dx, dy;
    for(std::list<Tracklet>::iterator iter = trackletsInSt[stationID].begin(); iter != trackletsInSt[stationID].end(); ++iter)
    {
        double z = p_geomSvc->getPlanePosition(iter->stationID*6);
        x.push_back(iter->getExpPositionX(z));
        y.push_back(iter->getExpPositionY(z));
        dx.push_back(iter->getExpPosErrorX(z));
        dy.push_back(iter->getExpPosErrorY(z));
    }

    TGraphErrors gr(x.size(), &x[0], &y[0], &dx[0], &dy[0]);
    gr.SetMarkerStyle(8);

    //Add detector frames
    std::vector<double> x_f, y_f, dx_f, dy_f;
    x_f.push_back(p_geomSvc->getPlaneCenterX(stationID*6 + 6));
    y_f.push_back(p_geomSvc->getPlaneCenterY(stationID*6 + 6));
    dx_f.push_back(p_geomSvc->getPlaneScaleX(stationID*6 + 6)*0.5);
    dy_f.push_back(p_geomSvc->getPlaneScaleY(stationID*6 + 6)*0.5);

    if(stationID == 2)
    {
        x_f.push_back(p_geomSvc->getPlaneCenterX(stationID*6 + 12));
        y_f.push_back(p_geomSvc->getPlaneCenterY(stationID*6 + 12));
        dx_f.push_back(p_geomSvc->getPlaneScaleX(stationID*6 + 12)*0.5);
        dy_f.push_back(p_geomSvc->getPlaneScaleY(stationID*6 + 12)*0.5);
    }

    TGraphErrors gr_frame(x_f.size(), &x_f[0], &y_f[0], &dx_f[0], &dy_f[0]);
    gr_frame.SetLineColor(kRed);
    gr_frame.SetLineWidth(2);
    gr_frame.SetFillColor(15);

    c1.cd();
    gr_frame.Draw("A2[]");
    gr.Draw("Psame");

    c1.SaveAs(outputFileName.c_str());
}

SRecTrack KalmanDSTrk::processOneTracklet(Tracklet& tracklet)
{
    //tracklet.print();
    KalmanTrack kmtrk;

    //Set the whole hit and node list
    for(std::list<SignedHit>::iterator iter = tracklet.hits.begin(); iter != tracklet.hits.end(); ++iter)
    {
        if(iter->hit.index < 0) continue;

        Node node_add(*iter);
        kmtrk.getNodeList().push_back(node_add);
        kmtrk.getHitIndexList().push_back(iter->sign*iter->hit.index);
    }

    //Set initial state
    TrkPar trkpar_curr;
    trkpar_curr._z = p_geomSvc->getPlanePosition(kmtrk.getNodeList().back().getHit().detectorID);
    //FIXME Debug Testing: sign reverse
    trkpar_curr._state_kf[0][0] = -tracklet.getCharge()*tracklet.invP/sqrt(1. + tracklet.tx*tracklet.tx + tracklet.ty*tracklet.ty);
    trkpar_curr._state_kf[1][0] = tracklet.tx;
    trkpar_curr._state_kf[2][0] = tracklet.ty;
    trkpar_curr._state_kf[3][0] = tracklet.getExpPositionX(trkpar_curr._z);
    trkpar_curr._state_kf[4][0] = tracklet.getExpPositionY(trkpar_curr._z);

    trkpar_curr._covar_kf.Zero();
    trkpar_curr._covar_kf[0][0] = 0.001;//1E6*tracklet.err_invP*tracklet.err_invP;
    trkpar_curr._covar_kf[1][1] = 0.01;//1E6*tracklet.err_tx*tracklet.err_tx;
    trkpar_curr._covar_kf[2][2] = 0.01;//1E6*tracklet.err_ty*tracklet.err_ty;
    trkpar_curr._covar_kf[3][3] = 100;//1E6*tracklet.getExpPosErrorX(trkpar_curr._z)*tracklet.getExpPosErrorX(trkpar_curr._z);
    trkpar_curr._covar_kf[4][4] = 100;//1E6*tracklet.getExpPosErrorY(trkpar_curr._z)*tracklet.getExpPosErrorY(trkpar_curr._z);

    kmtrk.setCurrTrkpar(trkpar_curr);
    kmtrk.getNodeList().back().getPredicted() = trkpar_curr;

    //Fit the track first with possibily a few nodes unresolved
    if(!fitTrack(kmtrk))
    {
#ifdef _DEBUG_ON
    	LogInfo("!fitTrack(kmtrk) - try flip charge");
#endif
    	trkpar_curr._state_kf[0][0] *= -1.;
      kmtrk.setCurrTrkpar(trkpar_curr);
      kmtrk.getNodeList().back().getPredicted() = trkpar_curr;
      if(!fitTrack(kmtrk))
      {
#ifdef _DEBUG_ON
      	LogInfo("!fitTrack(kmtrk) - failed flip charge also");
#endif
  			SRecTrack strack = tracklet.getSRecTrack();
  			strack.setKalmanStatus(-1);
  			return strack;
      }
    }

    if(!kmtrk.isValid()) {
#ifdef _DEBUG_ON
    	LogInfo("!kmtrk.isValid() Chi2 = " << kmtrk.getChisq() << " - try flip charge");
#endif
    	trkpar_curr._state_kf[0][0] *= -1.;
      kmtrk.setCurrTrkpar(trkpar_curr);
      kmtrk.getNodeList().back().getPredicted() = trkpar_curr;
      if(!fitTrack(kmtrk))
      {
#ifdef _DEBUG_ON
      	LogInfo("!fitTrack(kmtrk) - failed flip charge also");
#endif
  			SRecTrack strack = tracklet.getSRecTrack();
  			strack.setKalmanStatus(-1);
  			return strack;
      }

#ifdef _DEBUG_ON
      LogInfo("Chi2 after flip charge: " << kmtrk.getChisq());
      if(kmtrk.isValid()) {
      	LogInfo("flip charge worked!");
      }
#endif
    }

#ifdef _DEBUG_ON
    LogInfo("kmtrk.print()");
    kmtrk.print();
    LogInfo("kmtrk.printNodes()");
    kmtrk.printNodes();
#endif

    //Resolve left-right based on the current solution, re-fit if anything changed
    //resolveLeftRight(kmtrk);
    if(kmtrk.isValid())
    {
        SRecTrack strack = kmtrk.getSRecTrack();

        //Set trigger road ID
        TriggerRoad road(tracklet);
        strack.setTriggerRoad(road.getRoadID());

        //Set prop tube slopes
        strack.setNHitsInPT(tracklet.seg_x.getNHits(), tracklet.seg_y.getNHits());
        strack.setPTSlope(tracklet.seg_x.a, tracklet.seg_y.a);

        strack.setKalmanStatus(1);

        return strack;
    }
    else
    {
#ifdef _DEBUG_ON
    	LogInfo("!kmtrk.isValid()");
#endif
        SRecTrack strack = tracklet.getSRecTrack();
        strack.setKalmanStatus(-1);

        return strack;
    }
}

bool KalmanDSTrk::fitTrack(KalmanTrack& kmtrk)
{
    if(kmtrk.getNodeList().empty()) return false;

    if(kmfitter->processOneTrack(kmtrk) == 0)
    {
        return false;
    }
    kmfitter->updateTrack(kmtrk);

    return true;
}

void KalmanDSTrk::resolveLeftRight(KalmanTrack& kmtrk)
{
    bool isUpdated = false;

    std::list<int>::iterator hitID = kmtrk.getHitIndexList().begin();
    for(std::list<Node>::iterator node = kmtrk.getNodeList().begin(); node != kmtrk.getNodeList().end(); )
    {
        if(*hitID == 0)
        {
            double x_hit = node->getSmoothed().get_x();
            double y_hit = node->getSmoothed().get_y();
            double pos_hit = p_geomSvc->getUinStereoPlane(node->getHit().detectorID, x_hit, y_hit);

            int sign = 0;
            if(pos_hit > node->getHit().pos)
            {
                sign = 1;
            }
            else
            {
                sign = -1;
            }

            //update the node list
            TMatrixD m(1, 1), dm(1, 1);
            m[0][0] = node->getHit().pos + sign*node->getHit().driftDistance;
            dm[0][0] = p_geomSvc->getPlaneResolution(node->getHit().detectorID)*p_geomSvc->getPlaneResolution(node->getHit().detectorID);
            node->setMeasurement(m, dm);
            *hitID = sign*node->getHit().index;

            isUpdated = true;
        }

        ++node;
        ++hitID;
    }

    if(isUpdated) fitTrack(kmtrk);
}

void KalmanDSTrk::printTimers() {
	std::cout
		<<"KalmanDSTrk::printTimers: event: " << _event
		<< ": " << _timers["event"]->get_accumulated_time()/1000. << " sec" << std::endl;
	std::cout << "================================================================" << std::endl;
	std::cout << "Tracklet St2                "<<_timers["st2"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  Search DB                   "<<_timers["search_db_2"]->get_accumulated_time()/1000. << " sec" <<std::endl;

	std::cout << "Tracklet St3                "<<_timers["st3"]->get_accumulated_time()/1000. << " sec" <<std::endl;

	std::cout << "Tracklet St23               "<<_timers["st23"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  Search DB                   "<<_timers["search_db_23"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  st23_fit1                   "<<_timers["st23_fit1"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  st23_prop                   "<<_timers["st23_prop"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  st23_fit2                   "<<_timers["st23_fit2"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  st23_hodo                   "<<_timers["st23_hodo"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  st23_lr40                   "<<_timers["st23_lr40"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  st23_lr150                  "<<_timers["st23_lr150"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  st23_rm_hits                "<<_timers["st23_rm_hits"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  st23_acc                    "<<_timers["st23_acc"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  st23_reduce                 "<<_timers["st23_reduce"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  st23_prop                   "<<_timers["st23_prop"]->get_accumulated_time()/1000. << " sec" <<std::endl;

	std::cout << "Tracklet Global             "<<_timers["global"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  Global St1                  "<<_timers["global_st1"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  Search DB                   "<<_timers["search_db_glb"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "  Global Link                 "<<_timers["global_link"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "    Link Kalman                 "<<_timers["global_kalman"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "Tracklet Kalman             "<<_timers["kalman"]->get_accumulated_time()/1000. << " sec" <<std::endl;
	std::cout << "================================================================" << std::endl;
}

void KalmanDSTrk::chi2fit(int n, double x[], double y[], double& a, double& b)
{
    double sum = 0.;
    double sx = 0.;
    double sy = 0.;
    double sxx = 0.;
    double syy = 0.;
    double sxy = 0.;

    for(int i = 0; i < n; ++i)
    {
        ++sum;
        sx += x[i];
        sy += y[i];
        sxx += (x[i]*x[i]);
        syy += (y[i]*y[i]);
        sxy += (x[i]*y[i]);
    }

    double det = sum*sxx - sx*sx;
    if(fabs(det) < 1E-20)
    {
        a = 0.;
        b = 0.;

        return;
    }

    a = (sum*sxy - sx*sy)/det;
    b = (sy*sxx - sxy*sx)/det;
}











