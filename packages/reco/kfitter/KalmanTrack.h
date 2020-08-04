/*
KalmanTrack.h

Track used in all Kalman filter based process, i.e. KalmanFinder and KalmanFitter

Author: Kun Liu, liuk@fnal.gov
Created: 10-14-2012
*/

#ifndef _KALMANTRACK_H
#define _KALMANTRACK_H

#include <GlobalConsts.h>

#include <list>
#include <vector>
#include <algorithm>

#include <TGraph.h>

#include "KalmanUtil.h"
#include "KalmanFilter.h"
#include "SRecEvent.h"
#include "SRawEvent.h"
#include "FastTracklet.h"

class KalmanTrack
{
public:
    ///Constructor, default one is for list::resize()
    KalmanTrack();
    //KalmanTrack(Tracklet& tracklet);
    KalmanTrack(SRecTrack& _trk, SRawEvent *_rawevt, SRecEvent *_recevt);

    //Convert from tracklet to KalmanTrack
    void setTracklet(Tracklet& tracklet, bool wildseedcov = true);

    ///Flip the sign of this track
    void flipCharge();

    ///Self check to see if it is null
    bool isValid();

    ///Propagate the track to a designated position
    bool propagateTo(int detectorID);
    int getCurrentDetectorID() { return _node_next.getHit().detectorID; }

    ///Get the expected position
    double getExpPosition();
    double getExpPosError();
    double getExpPositionX() { return _node_next.getPredicted()._state_kf[3][0]; }
    double getExpPositionY() { return _node_next.getPredicted()._state_kf[4][0]; }
    double getExpPosErrorX() { return sqrt(_node_next.getPredicted()._covar_kf[3][0]); }
    double getExpPosErrorY() { return sqrt(_node_next.getPredicted()._covar_kf[4][0]); }

    Node* getNearestNodePtr(double z);
    void getExpPositionFast(double z, double& x, double& y, Node *_node = nullptr);
    void getExpPosErrorFast(double z, double& dx, double& dy, Node *_node = nullptr);

    ///Get the expected slope
    double getExpLocalSlop();
    double getExpLcSlopErr();
    double getExpLocalIntersection();
    double getExpLcIntersectionErr();

    ///Add a new hit to the hit list
    ///both in index list and node list
    bool addHit(Hit _hit);

    ///Update the track status
    void update();

    ///Update the momentum
    void updateMomentum();

    ///set the current track parameter
    void setCurrTrkpar(TrkPar& _trkpar) { _trkpar_curr = _trkpar; }

    ///Get the rough vertex momentum
    double getMomentumVertex(double z, double& px, double& py, double& pz);

    ///Get the list of hits associated
    std::list<int>& getHitIndexList() { return _hit_index; }
    std::list<Node>& getNodeList() { return _nodes; }
    std::vector<int> getMissedDetectorIDs();

    int getNHits() { return _hit_index.size(); }
    int getCharge() { return _nodes.front().getFiltered().get_charge(); }
    int getAssignedCharge() { return _nodes.back().getPredicted().get_charge(); }
    int getKickCharge();  //Get the charge decided by the sign of Kick
    double getChisq() { return _chisq; }
    double getQuality() { return _quality; }
    double getReducedChisq() { return _chisq/_hit_index.size(); }
    double getChisqVertex() { return _chisq_vertex; }

    double getXZSlopeInStation(int stationID);
    double getPositionInStation(int stationID, double& x, double& y, double& z);
    double getMomentumInStation(int stationID);
    double getMomentumUpstream() { return 1./fabs(_nodes.front().getFiltered()._state_kf[0][0]); }
    double getMomentumUpstream(double& px, double& py, double& pz);
    int getNHitsInStation(int stationID);

    void getSagittaInSuperDetector(int detectorID, double& pos_exp, double& window);

    ///Get Nodes in both upstream and downstream which are closest to KMag
    Node* getNodeDownstream();
    Node* getNodeUpstream();

    ///Overriden operators
    bool operator<(const KalmanTrack& elem) const;
    bool operator==(const KalmanTrack& elem) const;
    bool similarity(const KalmanTrack& elem) const;

    ///Debugging print
    void print();
    void printNodes();

    ///Quality monitoring methods
    int getNodeChisq(double *chisqs);
    int getAlignment(int level, int *detectorID, double* res, double* R, double* T);
    int getPositions(int level, double* x, double* y, double* z);
    int getMomentums(int level, double* px, double* py, double* pz);
    int getHitsIndex(int* index);
    TGraph getXZProjection();

    ///Output to SRecTrack
    SRecTrack getSRecTrack();

private:
    std::list<int> _hit_index;
    std::list<Node> _nodes;

    double _chisq;
    double _quality;
    double _chisq_vertex;

    Node _node_next;
    TrkPar _trkpar_curr;
};

#endif
