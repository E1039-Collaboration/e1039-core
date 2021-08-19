#ifndef _VERTEXFIT_H
#define _VERTEXFIT_H

/*
VertexFit.h

Definition of the Vertex fit of dimuon events, this package is aimed at:
1. Use closest distance method to find z0 for two muon tracks
2. Use vertex fit to fit the z0 --- will be implemented in next version

Reference: CBM-SOFT-note-2006-001, by S. Gorbunov and I. Kisel, with some minor
modifications

Author: Kun Liu, liuk@fnal.gov
Created: 2-8-2012
*/

// Fun4All includes
#include <fun4all/SubsysReco.h>
#include <GlobalConsts.h>

#include <iostream>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TMatrixD.h>

#include "KalmanUtil.h"
#include "SRecEvent.h"
#include "FastTracklet.h"
#include "GenFitExtrapolator.h"

class KalmanTrack;
class KalmanFilter;
class PHField;
class TGeoManager;
class JobOptsSvc;

class VtxPar
{
public:
    VtxPar()
    {
        _r.ResizeTo(3, 1);
        _cov.ResizeTo(3, 3);

        _r.Zero();
        _cov.Zero();
    }

    void print()
    {
        SMatrix::printMatrix(_r, "Vertex position:");
        SMatrix::printMatrix(_cov, "Vertex covariance:");
    }

    TMatrixD _r;
    TMatrixD _cov;
};

class VertexFit : public SubsysReco
{
public:
    VertexFit(const std::string& name = "VertexFit");
    ~VertexFit();

    int Init(PHCompositeNode *topNode);
    int InitRun(PHCompositeNode *topNode);
    int process_event(PHCompositeNode *topNode);
    int End(PHCompositeNode *topNode);

    ///Enable the optimization of final dimuon vertex z position
    void enableOptimization() { optimize = true; }
 //Fitting in the target center (Abi)
    void enable_fit_target_center() {fit_target_center = true;}

    ///Set the convergence control parameters
    void setControlParameter(int nMaxIteration, double tolerance)
    {
        _max_iteration = nMaxIteration;
        _tolerance = tolerance;
    }

    ///Set the SRecEvent, main external call the use vertex fit
    int setRecEvent(SRecEvent* recEvent, int sign1 = 1, int sign2 = -1);

    ///Initialize and reset
    void init();
    void addHypothesis(double x, double y, double z, double sigx = 2., double sigy = 2., double sigz = 50.0);
    void setStartingVertex(double x_start, double sigx_start, double y_start, double sigy_start, double z_start, double sigz_start);

    ///Add one track parameter set into the fit
    void addTrack(int index, SRecTrack& _track);
    void addTrack(int index, KalmanTrack& _track);
    void addTrack(int index, TrkPar& _trkpar);

    ///After setting both tracks and hypothesis, start the iteration
    int processOnePair();

    ///Find the primary vertex
    int findVertex();
	TVector3 findVertexDumpSt1(SRecTrack& track1, SRecTrack& track2);
    TVector3 findDimuonVertexFast(SRecTrack& track1, SRecTrack& track2);
    double findSingleMuonVertex(SRecTrack& _track);
    double findSingleMuonVertex(Node& _node_start);
    double findSingleMuonVertex(TrkPar& _trkpar_start);

    ///Gets
    double getVertexZ0() { return _vtxpar_curr._r[2][0]; }
    double getVXChisq() { return _chisq_vertex; }
    double getKFChisq() { return _chisq_kalman; }
    int getNTracks() { return _trkpar_curr.size(); }

    ///Core function, update the vertex prediction according to the track info.
    void updateVertex();

    ///Evaluation
    void bookEvaluation(std::string evalFileName = "vtx_eval.root");
    void fillEvaluation();

    ///Debugging output
    void print();

  const std::string& get_eval_file_name() const {
    return evalFileName;
  }

  void set_eval_file_name(const std::string& evalFileName) {
    this->evalFileName = evalFileName;
  }

private:

    int InitField(PHCompositeNode *topNode);

    int InitGeom(PHCompositeNode *topNode);

    int MakeNodes(PHCompositeNode *topNode);

    int GetNodes(PHCompositeNode *topNode);

    ///storage of the input track parameters
    std::vector<TrkPar> _trkpar_curr;

    ///vertex parameter
    VtxPar _vtxpar_curr;

    ///Kalman node at the vertex
    Node _node_vertex;

    ///pointer to external Kalman filter
    KalmanFilter* _kmfit;

    ///chi squares
    double _chisq_vertex;
    double _chisq_kalman;

    ///Starting points
    std::vector<double> z_start;
    std::vector<double> sig_z_start;	
    std::vector<double> x_start;
    std::vector<double> sig_x_start;
    std::vector<double> y_start;
    std::vector<double> sig_y_start;

    ///Temporary results
    std::vector<double> z_vertex;
	std::vector<double> x_vertex;
	std::vector<double> y_vertex;
    std::vector<double> r_vertex;
    std::vector<double> chisq_km;
    std::vector<double> chisq_vx;

    ///convergence control parameter
    int _max_iteration;
    double _tolerance;

    ///Track extrapolator
    GenFitExtrapolator _extrapolator;

    ///Flag to enable/disable optimization of final position
    bool optimize;
    bool fit_target_center;

    TGeoManager * _t_geo_manager;

    SRecEvent* _recEvent;

    ///Evaluation file and tree
    std::string evalFileName;
    TFile* evalFile;
    TTree* evalTree;

    int runID;
    int eventID;
    int targetPos;
    int nPos;
    int nNeg;
    int p_idx_eval;
    int m_idx_eval;
    int choice_eval;
    int choice_by_kf_eval;
    int choice_by_vx_eval;

    int nStart;
    double z_start_eval[50];
    int nIter_eval[50];
    double chisq_kf_eval[50];
    double chisq_vx_eval[50];
    double z_vertex_eval[50];
    double r_vertex_eval[50];

    double m_chisq_kf_eval;
    double s_chisq_kf_eval;
    double m_z_vertex_eval;
    double s_z_vertex_eval;
};

#endif
