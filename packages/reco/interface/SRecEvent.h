/*
SRecEvent.h

Definition of the class SRecEvent and SRecTrack. SRecTrack is the  final track structure of recontructed tracks.
Contains nothing but ROOT classes, light-weighted and can be used as input for physics analysis. SRecEvent serves
as a container of SRecTrack

Added SRecDimuon, containing the dimuon info

Author: Kun Liu, liuk@fnal.gov
Created: 01-21-2013
*/

#ifndef _SRECTRACK_H
#define _SRECTRACK_H

#include <GlobalConsts.h>

#include <phool/PHObject.h>
#include <interface_main/SQTrack.h>
#include <interface_main/SQDimuon.h>

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <map>
#include <cmath>
#include <stdexcept>

#include <TObject.h>
#include <TROOT.h>
#include <TMatrixD.h>
#include <TVector3.h>
#include <TLorentzVector.h>

#include <GenFit/MeasuredStateOnPlane.h>

#include "SRawEvent.h"

class SRecTrack: public SQTrack
{
public:
    SRecTrack();

    /// PHObject virtual overloads
    void         identify(std::ostream& os = std::cout) const { print(os); }
    void         Reset() { *this = SRecTrack(); }
    int          isValid() const;
    SRecTrack*   Clone() const  {return (new SRecTrack(*this)); }

    /// SQTrack virtual overloads
    virtual int  get_track_id() const      { return 0; }
    virtual void set_track_id(const int a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual int  get_rec_track_id() const      { return 0; }
    virtual void set_rec_track_id(const int a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual int  get_charge() const      { return getCharge(); }
    virtual void set_charge(const int a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual int  get_num_hits() const      { return getNHits(); }
    virtual void set_num_hits(const int a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual TVector3 get_pos_vtx() const           { return fVertexPos; }
    virtual void     set_pos_vtx(const TVector3 a) { fVertexPos = a;    } 

    virtual TVector3 get_pos_st1() const           { return getPositionVecSt1(); }
    virtual void     set_pos_st1(const TVector3 a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual TVector3 get_pos_st3() const           { return getPositionVecSt3(); }
    virtual void     set_pos_st3(const TVector3 a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual TLorentzVector get_mom_vtx() const                 { return getlvec(fVertexMom); }
    virtual void           set_mom_vtx(const TLorentzVector a) { fVertexMom = a.Vect(); }

    virtual TLorentzVector get_mom_st1() const                 { return getlvec(getMomentumVecSt1()); }
    virtual void           set_mom_st1(const TLorentzVector a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual TLorentzVector get_mom_st3() const                 { return getlvec(getMomentumVecSt3()); }
    virtual void           set_mom_st3(const TLorentzVector a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual double get_chisq() const          { return fChisq; }
    virtual double get_chisq_target() const   { return fChisqVertex; }
    virtual double get_chisq_dump() const     { return fChisqDump; } 
    virtual double get_chsiq_upstream() const { return fChisqUpstream; } 

    virtual TVector3 get_pos_target() const   { return fTargetPos; }
    virtual TVector3 get_pos_dump() const     { return fDumpPos; }

    virtual TLorentzVector get_mom_target() const { return getlvec(fTargetMom); }
    virtual TLorentzVector get_mom_dump() const   { return getlvec(fDumpMom); }

    virtual int get_hit_id(const int i) const { return fHitIndex[i]; }

    inline TLorentzVector getlvec(const TVector3& vec) const { TLorentzVector lvec; lvec.SetVectM(vec, M_MU); return lvec; }

    ///Gets
    Int_t getCharge() const { return (fState[0])[0][0] > 0 ? 1 : -1; }
    Int_t getNHits() const { return fHitIndex.size(); }
    Int_t getNHitsInStation(Int_t stationID);
    Double_t getChisq() const { return fChisq; }
    Double_t getProb() const;
    Double_t getQuality() const { return (Double_t)getNHits() - 0.4*getChisq(); }

    Int_t getHitIndex(Int_t i) { return fHitIndex[i]; }
    TMatrixD getStateVector(Int_t i) { return fState[i]; }
    TMatrixD getCovariance(Int_t i) { return fCovar[i]; }
    Double_t getZ(Int_t i) { return fZ[i]; }
    Double_t getChisqAtNode(Int_t i) { return fChisqAtNode[i]; }

    TVector3 getGFPlaneO(Int_t i)  { return fGFDetPlaneVec[0][i]; }
    TVector3 getGFPlaneU(Int_t i)  { return fGFDetPlaneVec[1][i]; }
    TVector3 getGFPlaneV(Int_t i)  { return fGFDetPlaneVec[2][i]; }
    TVectorD getGFAuxInfo(Int_t i) { return fGFAuxInfo[i]; }
    TVectorD getGFState(Int_t i)   { return fGFStateVec[i]; }
    TMatrixDSym getGFCov(Int_t i)  { return fGFCov[i]; }

    Int_t getNearestNode(Double_t z);
    void getExpPositionFast(Double_t z, Double_t& x, Double_t& y, Int_t iNode = -1);
    void getExpPosErrorFast(Double_t z, Double_t& dx, Double_t& dy, Int_t iNode = -1);
    Double_t getExpMomentumFast(Double_t z, Double_t& px, Double_t& py, Double_t& pz, Int_t iNode = -1);
    Double_t getExpMomentumFast(Double_t z, Int_t iNode = -1);

    Double_t getMomentumSt1(Double_t& px, Double_t& py, Double_t& pz) const { return getMomentum(fState.front(), px, py, pz); }
    Double_t getMomentumSt1() const { Double_t px, py, pz; return getMomentumSt1(px, py, pz); }
    TVector3 getMomentumVecSt1() const { Double_t px, py, pz; getMomentumSt1(px, py, pz); return TVector3(px, py, pz); }

    Double_t getMomentumSt3(Double_t& px, Double_t& py, Double_t& pz) const { return getMomentum(fState.back(), px, py, pz); }
    Double_t getMomentumSt3() const { Double_t px, py, pz; return getMomentumSt3(px, py, pz); }
    TVector3 getMomentumVecSt3() const { Double_t px, py, pz; getMomentumSt3(px, py, pz); return TVector3(px, py, pz); }

    Double_t getPositionSt1(Double_t& x, Double_t& y) const { return getPosition(fState.front(), x, y); }
    Double_t getPositionSt1() const { Double_t x, y; return getPositionSt1(x, y); }
    TVector3 getPositionVecSt1() const { Double_t x, y; getPositionSt1(x, y); return TVector3(x, y, fZ.front()); }

    Double_t getPositionSt3(Double_t& x, Double_t& y) const { return getPosition(fState.back(), x, y); }
    Double_t getPositionSt3() const { Double_t x, y; return getPositionSt3(x, y); }
    TVector3 getPositionVecSt3() const { Double_t x, y; getPositionSt3(x, y); return TVector3(x, y, fZ.back()); }

    Double_t getMomentum(const TMatrixD& state, Double_t& px, Double_t& py, Double_t& pz) const;
    Double_t getPosition(const TMatrixD& state, Double_t& x, Double_t& y) const;

    ///Fit status
    Bool_t isKalmanFitted() { return fKalmanStatus > 0; }
    void setKalmanStatus(Int_t status) { fKalmanStatus = status; }

    ///Comparitor
    bool operator<(const SRecTrack& elem) const;

    ///Sets
    void setChisq(Double_t chisq) { fChisq = chisq; }
    void insertHitIndex(Int_t index) { fHitIndex.push_back(index); }
    void insertStateVector(TMatrixD state) { fState.push_back(state); }
    void insertCovariance(TMatrixD covar) { fCovar.push_back(covar); }
    void insertZ(Double_t z) { fZ.push_back(z); }
    void insertChisq(Double_t chisq) { fChisqAtNode.push_back(chisq); }
    void insertGFState(const genfit::MeasuredStateOnPlane& msop);

    ///Fast-adjust of kmag
    void adjustKMag(double kmagStr);

    ///Vertex stuff
    bool isVertexValid() const;
    void setZVertex(Double_t z, bool update = true);
    void updateVtxHypothesis();

    ///Plain setting, no KF-related stuff
    void setVertexFast(TVector3 mom, TVector3 pos);

    ///Simple swim to vertex
    void swimToVertex(TVector3* pos = nullptr, TVector3* mom = nullptr, bool hyptest = true);

    ///Get the vertex info
    TLorentzVector getMomentumVertex();
    Double_t getMomentumVertex(Double_t& px, Double_t& py, Double_t& pz) { return getMomentum(fStateVertex, px, py, pz); }
    Double_t getZVertex() { return fVertexPos[2]; }
    Double_t getRVertex() { return fVertexPos.Perp(); }
    TVector3 getVertex() { return fVertexPos; }
    Double_t getVtxPar(Int_t i) { return fVertexPos[i]; }
    Double_t getChisqVertex() { return fChisqVertex; }

    //Get mom/pos at a given location
    TVector3 getDumpPos() { return fDumpPos; }
    TVector3 getDumpFacePos() { return fDumpFacePos; }
    TVector3 getTargetPos() { return fTargetPos; }
    TVector3 getXVertexPos() { return fXVertexPos; }
    TVector3 getYVertexPos() { return fYVertexPos; }
    TVector3 getDumpMom() { return fDumpMom; }
    TVector3 getDumpFaceMom() { return fDumpFaceMom; }
    TVector3 getTargetMom() { return fTargetMom; }
    TVector3 getXVertexMom() { return fXVertexMom; }
    TVector3 getYVertexMom() { return fYVertexMom; }
    TVector3 getVertexPos() { return fVertexPos; }
    TVector3 getVertexMom() { return fVertexMom; }
    Double_t getChisqDump() { return fChisqDump; }
    Double_t getChisqTarget() { return fChisqTarget; }
    Double_t getChisqUpstream() { return fChisqUpstream; }

    //Set mom/pos at a given location
    void setDumpPos(TVector3 pos) { fDumpPos = pos; }
    void setDumpFacePos(TVector3 pos) { fDumpFacePos = pos; }
    void setTargetPos(TVector3 pos) { fTargetPos = pos; }
    void setXVertexPos(TVector3 pos) { fXVertexPos = pos; }
    void setYVertexPos(TVector3 pos) { fYVertexPos = pos; }
    void setVertexPos(TVector3 pos) { fVertexPos = pos; }
    void setDumpMom(TVector3 mom) { fDumpMom = mom; }
    void setDumpFaceMom(TVector3 mom) { fDumpFaceMom = mom; }
    void setTargetMom(TVector3 mom) { fTargetMom = mom; }
    void setXVertexMom(TVector3 mom) { fXVertexMom = mom; }
    void setYVertexMom(TVector3 mom) { fYVertexMom = mom; }
    void setVertexMom(TVector3 mom) { fVertexMom = mom; }
    void setChisqDump(Double_t chisq) { fChisqDump = chisq; }
    void setChisqTarget(Double_t chisq) { fChisqTarget = chisq; }
    void setChisqUpstream(Double_t chisq) { fChisqUpstream = chisq; }
    void setChisqVertex(Double_t chisq) { fChisqVertex = chisq; }

    //Trigger road info
    void setTriggerRoad(Int_t roadID) { fTriggerID = roadID; }
    Int_t getTriggerRoad() { return fTriggerID; }

    //Prop. tube muon ID info
    void setPTSlope(Double_t slopeX, Double_t slopeY) { fPropSlopeX = slopeX; fPropSlopeY = slopeY; }
    void setNHitsInPT(Int_t nHitsX, Int_t nHitsY) { fNPropHitsX = nHitsX; fNPropHitsY = nHitsY; }
    Double_t getPTSlopeX() { return fPropSlopeX; }
    Double_t getPTSlopeY() { return fPropSlopeY; }
    Double_t getDeflectionX() { return fState.back()[1][0] - fPropSlopeX; }
    Double_t getDeflectionY() { return fState.back()[2][0] - fPropSlopeY; }
    Int_t getNHitsInPTX() { return fNPropHitsX; }
    Int_t getNHitsInPTY() { return fNPropHitsY; }

    //Overall track quality cut
    //bool isValid();
    bool isTarget();
    bool isDump();

    ///Debugging output
    void print(std::ostream& os = std::cout) const;

private:
    ///Total Chisq
    Double_t fChisq;

    ///Hit list and associated track parameters
    std::vector<Int_t> fHitIndex;
    std::vector<TMatrixD> fState;
    std::vector<TMatrixD> fCovar;
    std::vector<Double_t> fZ;
    std::vector<Double_t> fChisqAtNode;

    ///Momentum/Position at a given z
    TVector3 fDumpFacePos;
    TVector3 fDumpPos;
    TVector3 fTargetPos;
    TVector3 fXVertexPos;
    TVector3 fYVertexPos;

    TVector3 fDumpFaceMom;
    TVector3 fDumpMom;
    TVector3 fTargetMom;
    TVector3 fXVertexMom;
    TVector3 fYVertexMom;

    ///Vertex infomation
    TVector3 fVertexMom;      //duplicate information as fStateVertex already contains all the info., just keep it for now
    TVector3 fVertexPos;
    Double_t fChisqVertex;
    TMatrixD fStateVertex;
    TMatrixD fCovarVertex;

    ///Kalman Fitted
    Int_t fKalmanStatus;

    ///Corresponding trigger road
    Int_t fTriggerID;

    ///Prop. tube. slope
    Int_t fNPropHitsX;
    Int_t fNPropHitsY;
    Double_t fPropSlopeX;
    Double_t fPropSlopeY;

    //Chisq of three test position
    Double_t fChisqTarget;
    Double_t fChisqDump;
    Double_t fChisqUpstream;

    //GenFit track info - only available if the track comes from GF fitter
    std::vector<TVector3> fGFDetPlaneVec[3];
    std::vector<TVectorD> fGFAuxInfo;
    std::vector<TVectorD> fGFStateVec;
    std::vector<TMatrixDSym> fGFCov;

    ClassDef(SRecTrack, 11)
};

class SRecDimuon: public SQDimuon
{
public:

    /// PHObject virtual overloads
    void         identify(std::ostream& os = std::cout) const { os << "SRecDimuon: TODO: NOT IMPLEMENTED!" << std::endl; }
    void         Reset() { *this = SRecDimuon(); }
    int          isValid() const;
    SRecDimuon*  Clone() const { return (new SRecDimuon(*this)); }

    //SQDimuon virtual functions
    virtual int  get_dimuon_id() const      { return 0; }
    virtual void set_dimuon_id(const int a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual int  get_rec_dimuon_id() const      { return 0; }
    virtual void set_rec_dimuon_id(const int a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual int  get_pdg_id() const      { return 0; }
    virtual void set_pdg_id(const int a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual int  get_track_id_pos() const      { return trackID_pos; }
    virtual void set_track_id_pos(const int a) { trackID_pos = a; }

    virtual int  get_track_id_neg() const      { return trackID_neg; }
    virtual void set_track_id_neg(const int a) { trackID_neg = a; }

    virtual TVector3 get_pos() const           { return vtx; }
    virtual void     set_pos(const TVector3 a) { vtx = a; }

    virtual TLorentzVector get_mom() const                 { return p_pos + p_neg; }
    virtual void           set_mom(const TLorentzVector a) { throw std::logic_error(__PRETTY_FUNCTION__); }

    virtual TLorentzVector get_mom_pos() const                 { return p_pos; }
    virtual void           set_mom_pos(const TLorentzVector a) { p_pos = a; }

    virtual TLorentzVector get_mom_neg() const                 { return p_neg; }
    virtual void           set_mom_neg(const TLorentzVector a) { p_neg = a; }

    virtual double get_mass() const { return mass; }
    virtual double get_x1()   const { return x1; }
    virtual double get_x2()   const { return x2; }
    virtual double get_xf()   const { return xF; }

    virtual double get_chisq() const { return chisq_kf; }

    //Get the total momentum of the virtual photon
    TLorentzVector getVPhoton() { return p_pos + p_neg; }

    //Calculate the kinematic vairables
    void calcVariables();

    //Dimuon quality cut
    //bool isValid();

    //Target dimuon
    bool isTarget();

    //Dump dimuon
    bool isDump();

    //Index of muon track used in host SRecEvent
    Int_t trackID_pos;
    Int_t trackID_neg;

    //4-momentum of the muon tracks after vertex fit
    TLorentzVector p_pos;
    TLorentzVector p_neg;

    //4-momentum of the muon tracks before vertex fit
    TLorentzVector p_pos_single;
    TLorentzVector p_neg_single;

    //3-vector vertex position
    TVector3 vtx;
    TVector3 vtx_pos;
    TVector3 vtx_neg;

    //Track projections at different location
    TVector3 proj_target_pos;
    TVector3 proj_dump_pos;
    TVector3 proj_target_neg;
    TVector3 proj_dump_neg;

    //Kinematic variables
    Double_t mass;
    Double_t pT;
    Double_t xF;
    Double_t x1;
    Double_t x2;
    Double_t costh;
    Double_t phi;
    Double_t mass_single;
    Double_t chisq_single;

    //Vertex fit chisqs
    Double_t chisq_kf;
    Double_t chisq_vx;

    //Chisq of three test position
    Double_t chisq_target;
    Double_t chisq_dump;
    Double_t chisq_upstream;

    ClassDef(SRecDimuon, 6)
};

class SRecEvent: public PHObject
{
public:
    SRecEvent();

    /// PHObject virtual overloads
    void         identify(std::ostream& os = std::cout) const {
      os
      << " SRecEvent: { " << fRunID << ", " << fSpillID << ", " << fEventID << " } "
      << " NTracks: " << fAllTracks.size()
      << std::endl;
    }
    void         Reset() {*this = SRecEvent();}
    int          isValid() const {return true;}
    SRecEvent*   Clone() const {return (new SRecEvent(*this));}

    ///Set/Get event info
    void setEventInfo(SRawEvent* rawEvent);
    void setEventInfo(int runID, int spillID, int eventID) { fRunID = runID; fSpillID = spillID; fEventID = eventID; }
    void setTargetPos(int targetPos) { fTargetPos = targetPos; }
    void setRecStatus(int status) { fRecStatus += status; }

    ///directly setup everything by raw event
    void setRawEvent(SRawEvent* rawEvent);

    ///Trigger util
    bool isTriggeredBy(Int_t trigger) { return (fTriggerBits & trigger) != 0; }

    Int_t getRunID() { return fRunID; }
    Int_t getSpillID() { return fSpillID; }
    Int_t getEventID() { return fEventID; }
    Int_t getTargetPos() { return fTargetPos; }
    Int_t getTriggerBits() { return fTriggerBits; }
    Int_t getRecStatus() { return fRecStatus; }

    Int_t getLocalID(Int_t hitID) { return fLocalID[hitID]; }

    //Event source set/get
    void setEventSource(Int_t id1, Int_t id2) { fSource1 = id1; fSource2 = id2; }
    Int_t getSourceID1() { return fSource1; }
    Int_t getSourceID2() { return fSource2; }

    ///Get tracks
    Int_t getNTracks() { return fAllTracks.size(); }
    SRecTrack& getTrack(Int_t i) { return fAllTracks[i]; }

    ///Get track IDs
    std::vector<Int_t> getChargedTrackIDs(Int_t charge);

    ///Get dimuons
    Int_t getNDimuons() { return fDimuons.size(); }
    SRecDimuon& getDimuon(Int_t i) { return fDimuons[i]; }

    ///Insert tracks
    void insertTrack(SRecTrack trk) { fAllTracks.push_back(trk); }
    void reIndex() { sort(fAllTracks.begin(), fAllTracks.end()); }

    ///Insert dimuon
    void insertDimuon(SRecDimuon dimuon) { fDimuons.push_back(dimuon); }

    ///Clear everything
    void clear();
    void clearTracks();
    void clearDimuons();

private:
    ///Reconstruction status
    Short_t fRecStatus;

    ///Basic event info.
    Int_t fRunID;
    Int_t fSpillID;
    Int_t fEventID;

    ///Target position
    Int_t fTargetPos;

    ///Trigger bit
    Int_t fTriggerBits;

    ///Container of SRecTrack
    std::vector<SRecTrack> fAllTracks;

    ///Dimuons reconstructed
    std::vector<SRecDimuon> fDimuons;

    ///Mapping of hitID to local container ID in SRawEvent
    std::map<Int_t, Int_t> fLocalID;

    //Event source
    Int_t fSource1;
    Int_t fSource2;

    ClassDef(SRecEvent, 5)
};

#endif
