/**
 * \class KalmanDSTrk
 * \brief Pattern Dictionary Filter built based on Kun Liu's ktracker
 * \author Haiwang Yu, yuhw@nmsu.edu
 *
 * Created: 08-27-2018
 */

#ifndef _KalmanDSTrk_H
#define _KalmanDSTrk_H

#include <GlobalConsts.h>
#include <jobopts_svc/JobOptsSvc.h>
#include <geom_svc/GeomSvc.h>

#include "SRawEvent.h"
#include "KalmanTrack.h"
#include "KalmanFitter.h"
#include "FastTracklet.h"
#include "PatternDB.h"

#include <list>
#include <vector>
#include <map>
#include <set>
//#include <tuple>

#include <Math/Factory.h>
#include <Math/Minimizer.h>
#include <Math/Functor.h>

class TGeoManager;

class PHField;
class PHTimer;

class TFile;
class TNtuple;

class KalmanDSTrk
{
public:

  //enum DS_LEVEL {NO_DS, ST23_DS, ST123_DS, IN_ST_DS};
  enum DS_LEVEL {NO_DS, IN_ST_DS, ST23_DS, ST123_DS};

	explicit KalmanDSTrk(
			const PHField* field,
			const TGeoManager *geom,
			bool enable_KF = true,
			int DS_level = KalmanDSTrk::NO_DS,
			const std::string sim_db_name = "",
      const std::string pattern_db_name = ""
			);

	~KalmanDSTrk();

	//
	void Verbosity(const int a) {verbosity = a;}
	int Verbosity() const {return verbosity;}
	void printTimers();

	//Set the input event
	int setRawEvent(SRawEvent* event_input);
	int setRawEventWorker(SRawEvent* event_input);
	void setRawEventDebug(SRawEvent* event_input);

	//Event quality cut
	bool acceptEvent(SRawEvent* rawEvent);

	///Tracklet finding stuff
	//Build tracklets in a station
	void buildTrackletsInStation(int stationID, int listID, double* pos_exp = NULL, double* window = NULL);

	//Build back partial tracks using tracklets in station 2 & 3
	void buildBackPartialTracks();

	//Build global tracks by connecting station 23 tracklets and station 1 tracklets
	void buildGlobalTracks();

	//Fit tracklets
	int fitTracklet(Tracklet& tracklet);

	//Check the quality of tracklet, number of hits
	bool acceptTracklet(Tracklet& tracklet);
	bool hodoMask(Tracklet& tracklet);
	bool muonID_comp(Tracklet& tracklet);
	bool muonID_search(Tracklet& tracklet);
	bool muonID_hodoAid(Tracklet& tracklet);

	void buildPropSegments();

	//Resolve left-right when possible
	void resolveLeftRight(SRawEvent::hit_pair hpair, int& LR1, int& LR2);
	void resolveLeftRight(Tracklet& tracklet, double threshold);
	void resolveSingleLeftRight(Tracklet& tracklet);

	//Remove bad hit if needed
	void removeBadHits(Tracklet& tracklet);

	//Reduce the list of tracklets, returns the number of elements reduced
	int reduceTrackletList(std::list<Tracklet>& tracklets);

	//Get exp postion and window using sagitta method in station 1
	void getSagittaWindowsInSt1(Tracklet& tracklet, double* pos_exp, double* window, int st1ID);
	void getExtrapoWindowsInSt1(Tracklet& tracklet, double* pos_exp, double* window, int st1ID);

	//Print the distribution of tracklets at detector back/front
	void printAtDetectorBack(int stationID, std::string outputFileName);

	///Track fitting stuff
	//Convert Tracklet to KalmanTrack and solve left-right problem, and eventually to a SRecTrack
	SRecTrack processOneTracklet(Tracklet& tracklet);

	//Use Kalman fitter to fit a track
	bool fitTrack(KalmanTrack& kmtrk);

	//Resolve left right by Kalman fitting results
	void resolveLeftRight(KalmanTrack& kmtrk);

	///Final output
	std::list<Tracklet>& getFinalTracklets() { return trackletsInSt[4]; }
	std::list<Tracklet>& getBackPartials() { return trackletsInSt[3]; }
	std::list<Tracklet>& getTrackletList(int i) { return trackletsInSt[i]; }
	std::list<SRecTrack>& getSRecTracks() { return stracks; }
	std::list<PropSegment>& getPropSegments(int i) { return propSegs[i]; }

	/// Tool, a simple-minded chi square fit
	/// Y = a*X + b
	void chi2fit(int n, double x[], double y[], double& a, double& b);

  const std::string& get_pattern_db_name() const {
    return _pattern_db_name;
  }

  void set_pattern_db_name(const std::string& patternDbName) {
    _pattern_db_name = patternDbName;
  }

  const std::string& get_sim_db_name() const {
    return _sim_db_name;
  }

  void set_sim_db_name(const std::string& simDbName) {
    _sim_db_name = simDbName;
  }

private:

    int verbosity;

    //Raw event input
    SRawEvent* rawEvent;
    std::vector<Hit> hitAll;

    //Tracklets in one event, id = 0, 1, 2 for station 0/1, 2, 3+/-, id = 3 for station 2&3 combined, id = 4 for global tracks
    //Likewise for the next part
    std::list<Tracklet> trackletsInSt[5];

    //Final SRecTrack list
    std::list<SRecTrack> stracks;

    //Prop. tube segments for muon id purposes
    // 0 for X-Z, 1 for Y-Z
    std::list<PropSegment> propSegs[2];

    ///Configurations of tracklet finding
    //Hodo. IDs for masking, 4 means we have 4 hodo stations
    std::vector<int> detectorIDs_mask[4];
    std::vector<int> detectorIDs_maskX[4];
    std::vector<int> detectorIDs_maskY[4];
    std::list<int>   hitIDs_mask[4];              //hits in T/B, L/R are combined
    std::vector<int> detectorIDs_muidHodoAid[2];  //Aux-hodoscope masking for muon ID

    //register difference hodo masking stations for different chamber detectors
    std::vector<int> stationIDs_mask[nStations];

    //prop. tube IDs for MUID -- 0 for x-z, 1 for y-z
    int detectorIDs_muid[2][4];
    double z_ref_muid[2][4];
    std::list<int> hitIDs_muid[2][4];
    std::list<int> hitIDs_muidHodoAid[2];

    //Masking window sizes, index is the uniqueID defined by nElement*detectorID + elementID
    double z_mask[nHodoPlanes+nPropPlanes];
    double x_mask_min[nHodoPlanes+nPropPlanes][72];
    double x_mask_max[nHodoPlanes+nPropPlanes][72];
    double y_mask_min[nHodoPlanes+nPropPlanes][72];
    double y_mask_max[nHodoPlanes+nPropPlanes][72];

    ///For following part, id = 0, 1, 2, 3, 4, 5, 6 stand for station 0, 1, 2, 3+, 3-, and prop tubes X-Z and Y-Z
    //Super plane IDs for DCs
    std::vector<int> superIDs[nChamberPlanes/6+2];

    //Window sizes for X-U combination
    double u_win[nChamberPlanes/6];
    double u_costheta[nChamberPlanes/6];
    double u_sintheta[nChamberPlanes/6];
    double z_plane_x[nChamberPlanes/6];
    double z_plane_u[nChamberPlanes/6];
    double z_plane_v[nChamberPlanes/6];

    //Plane angles for all planes
    double costheta_plane[nChamberPlanes+1];
    double sintheta_plane[nChamberPlanes+1];

    //Z positions
    double z_plane[nChamberPlanes+1];

    //Maximum slope and intersection in each view
    double slope_max[nChamberPlanes+1];
    double intersection_max[nChamberPlanes+1];

    //Resolutions of all planes
    double resol_plane[nChamberPlanes+1];

    //Cell width of all planes
    double spacing_plane[nChamberPlanes+1];

    //Sagitta ratio in station 1, index 0, 1, 2 are for X/U/V
    int s_detectorID[3];

    //Current tracklets being processed
    Tracklet tracklet_curr;

    //Least chi square fitter and functor
    ROOT::Math::Minimizer* minimizer[2];
    ROOT::Math::Functor fcn;

    //Kalman fitter
    KalmanFitter* kmfitter;

    //Geometry service
    GeomSvc* p_geomSvc;

    //Job option service
    JobOptsSvc* p_jobOptsSvc;

    //Flag for enable Kalman fitting
    const bool _enable_KF;

    // Dictionary search
    int _DS_level;
    std::string _sim_db_name;
    std::string _pattern_db_name;

    /*
    //typedef std::tuple<unsigned char, unsigned char, unsigned char, unsigned char> TrackletKey;
    typedef std::tuple<unsigned int, unsigned int> TrackletKey;
    const TrackletKey _error_key;

    typedef std::tuple<TrackletKey, TrackletKey> PartTrackKey;

    void print(TrackletKey key);
    void print(PartTrackKey key);

    enum STATION { DC1, DC2, DC3p, DC3m};

    std::set<TrackletKey> _db_st2;
    std::set<TrackletKey> _db_st3;
    std::set<PartTrackKey> _db_st23;

    int LoadPatternDB (const std::string fname);

    TrackletKey EncodeTrackletKey (
    		STATION,
    		const unsigned int X, const unsigned int Xp,
				const unsigned int U, const unsigned int Up,
				const unsigned int V, const unsigned int Vp);


    TrackletKey GetTrackletKey (const Tracklet tracklet, const STATION station);

  	std::map<unsigned int, unsigned int> _detid_view;
  	*/

  	PatternDB* _pattern_db;

    std::map< std::string, PHTimer* > _timers;

    /// Analysis mode
    bool _ana_mode;
    TFile *_fana;
    TNtuple *_tana_Event;
    TNtuple *_tana_St1;
    TNtuple *_tana_St2;
    TNtuple *_tana_St3;
    TNtuple *_tana_St23;
    TNtuple *_tana_St123;


    int _event;
};

#endif /*_KalmanDSTrk_H*/
