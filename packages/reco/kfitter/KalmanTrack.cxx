/*
KalmanTrack.cxx

Implementation of class Seed, KalmanTrack

Author: Kun Liu, liuk@fnal.gov
Created: 10-14-2012
*/

#include <iostream>
#include <cmath>
#include <algorithm>

#include <geom_svc/GeomSvc.h>
#include "KalmanTrack.h"

///Default constructor for KalmanTrack, wiill only be used when resizing the track candidate list
KalmanTrack::KalmanTrack()
{
    _hit_index.clear();
    _nodes.clear();

    _chisq = 0.;
    update();
}

KalmanTrack::KalmanTrack(SRecTrack& _trk, SRawEvent *_rawevt, SRecEvent *_recevt)
{
    _chisq = _trk.getChisq();

    _hit_index.clear();
    _nodes.clear();

    GeomSvc *p_geomSvc = GeomSvc::instance();
    for(int i = 0; i < _trk.getNHits(); i++)
    {
        _hit_index.push_back(_trk.getHitIndex(i));

        ///Update the hit info. if needed
        Hit _hit = _rawevt->getHit(_recevt->getLocalID(_trk.getHitIndex(i)));
        _hit.pos = p_geomSvc->getMeasurement(_hit.detectorID, _hit.elementID);
        _hit.driftDistance = _hit.getSign()*p_geomSvc->getDriftDistance(_hit.detectorID, _hit.tdcTime);

        Node _node(_hit);
        _node.setZ(_trk.getZ(i));
        _node.setChisq(_trk.getChisqAtNode(i));
        _node.getPredicted()._state_kf = _trk.getStateVector(i);
        _node.getPredicted()._covar_kf = _trk.getCovariance(i);
        _node.getPredicted()._z = _trk.getZ(i);
        _node.getFiltered()._state_kf = _trk.getStateVector(i);
        _node.getFiltered()._covar_kf = _trk.getCovariance(i);
        _node.getFiltered()._z = _trk.getZ(i);
        _node.getSmoothed()._state_kf = _trk.getStateVector(i);
        _node.getSmoothed()._covar_kf = _trk.getCovariance(i);
        _node.getSmoothed()._z = _trk.getZ(i);
        _node.setSmoothDone();
        _node.setPredictionDone();
        _node.setFilterDone();
    }
}

void KalmanTrack::setTracklet(Tracklet& tracklet, bool wildseedcov)
{
    //Set the whole hit and node list
    for(auto iter = tracklet.hits.begin(); iter != tracklet.hits.end(); ++iter)
    {
        if(iter->hit.index < 0) continue;

        Node node_add(*iter);
        _nodes.push_back(node_add);
        _hit_index.push_back(iter->sign*iter->hit.index);
    }

    //Set initial state

    _trkpar_curr._z = GeomSvc::instance()->getPlanePosition(_nodes.back().getHit().detectorID);
    _trkpar_curr._state_kf[0][0] = tracklet.getCharge()*tracklet.invP/sqrt(1. + tracklet.tx*tracklet.tx + tracklet.ty*tracklet.ty);
    _trkpar_curr._state_kf[1][0] = tracklet.tx;
    _trkpar_curr._state_kf[2][0] = tracklet.ty;
    _trkpar_curr._state_kf[3][0] = tracklet.getExpPositionX(_trkpar_curr._z);
    _trkpar_curr._state_kf[4][0] = tracklet.getExpPositionY(_trkpar_curr._z);

    _trkpar_curr._covar_kf.Zero();
    if(wildseedcov)
    {
        _trkpar_curr._covar_kf[0][0] = 0.001;//1E6*tracklet.err_invP*tracklet.err_invP;
        _trkpar_curr._covar_kf[1][1] = 0.01;//1E6*tracklet.err_tx*tracklet.err_tx;
        _trkpar_curr._covar_kf[2][2] = 0.01;//1E6*tracklet.err_ty*tracklet.err_ty;
        _trkpar_curr._covar_kf[3][3] = 100;//1E6*tracklet.getExpPosErrorX(trkpar_curr._z)*tracklet.getExpPosErrorX(trkpar_curr._z);
        _trkpar_curr._covar_kf[4][4] = 100;//1E6*tracklet.getExpPosErrorY(trkpar_curr._z)*tracklet.getExpPosErrorY(trkpar_curr._z);
    }

    _nodes.back().getPredicted() = _trkpar_curr;
}

bool KalmanTrack::isValid()
{
	//FIXME 20 for now; original _chisq < 150
    if(_chisq < 0 || _chisq/_hit_index.size() > 100) return false;
    if(_nodes.empty()) return false;
    if(getMomentumUpstream() < 5. || getMomentumUpstream() > 120.) return false;

    return true;
}

void KalmanTrack::updateMomentum()
{
    double z_0 = 0.;
    double z_bend_k = 1064.26;
    double z_bend_f = 251.4;
    double kick_f = -2.911;
    double kick_k = -0.4;
    double deltaE_kf = 8.12;
    double c4 = deltaE_kf;
    double c5 = deltaE_kf/2.;

    double z_ref = _nodes.front().getZ();
    double x_ref = _nodes.front().getFiltered()._state_kf[3][0];
    double axz = _nodes.front().getFiltered()._state_kf[1][0];

    double charge;
    if(_nodes.front().getFiltered()._state_kf[0][0] > 0)
    {
        charge = 1.;
    }
    else
    {
        charge = -1.;
    }

    double c1 = (z_0 - z_bend_f)*kick_f*charge;
    double c2 = (z_0 - z_bend_k)*kick_k*charge;
    double c3 = axz*(z_ref - z_0) - x_ref;
    double b = c1/c3 + c2/c3 - c4 - c5;
    double c = c4*c5 - c1*c5/c3 - c2*c4/c3;

    double disc = b*b - 4.*c;
    if(disc < 0)
    {
        return;
    }

    double p = (-b + sqrt(disc))/2.;
    p -= deltaE_kf;

    if(p < 0. || p > 100.)
    {
        return;
    }

    //LogInfo("Momentum updated from " << 1./fabs(_trkpar_curr._state_kf[0][0]) << " to " << p);
    _trkpar_curr._state_kf[0][0] = charge/p;
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        iter->getPredicted()._state_kf[0][0] = charge/p;
        iter->getFiltered()._state_kf[0][0] = charge/p;
    }
}

void KalmanTrack::flipCharge()
{
    /*
    _trkpar_curr.flip_charge();
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
      {
        if(iter->isPredictionDone()) iter->getPredicted().flip_charge();
        if(iter->isFilterDone()) iter->getFiltered().flip_charge();
        if(iter->isSmoothDone()) iter->getSmoothed().flip_charge();
      }
    */

    TrkPar _trkpar = _nodes.back().getSmoothed();
    _trkpar.flip_charge();

    _trkpar._covar_kf.Zero();
    _trkpar._covar_kf[0][0] = 0.001;
    _trkpar._covar_kf[1][1] = 0.01;
    _trkpar._covar_kf[2][2] = 0.01;
    _trkpar._covar_kf[3][3] = 100.;
    _trkpar._covar_kf[4][4] = 100.;

    _nodes.back().getPredicted() = _trkpar;
}

bool KalmanTrack::propagateTo(int detectorID)
{
    Hit hit_dummy;
    hit_dummy.detectorID = detectorID;

    _node_next = Node(hit_dummy);

    KalmanFilter *kmfit = KalmanFilter::instance();
    kmfit->setCurrTrkpar(_trkpar_curr);
    if(kmfit->predict(_node_next))
    {
        return true;
    }
    else
    {
        return false;
    }
}

double KalmanTrack::getMomentumVertex(double z, double& px, double& py, double& pz)
{
    Node _node_vertex;
    _node_vertex.setZ(z);

    TMatrixD m(2, 1), cov(2, 2),  proj(2, 5);
    m[0][0] = 0.;
    m[1][0] = 0.;

    cov.Zero();
    cov[0][0] = 1.;
    cov[1][1] = 1.;

    proj.Zero();
    proj[0][3] = 1.;
    proj[1][4] = 1.;

    _node_vertex.getMeasurement().ResizeTo(2, 1);
    _node_vertex.getMeasurementCov().ResizeTo(2, 2);
    _node_vertex.getProjector().ResizeTo(2, 5);

    _node_vertex.getMeasurement() = m;
    _node_vertex.getMeasurementCov() = cov;
    _node_vertex.getProjector() = proj;

    KalmanFilter *kmfit = KalmanFilter::instance();
    kmfit->setCurrTrkpar(_nodes.front().getSmoothed());
    kmfit->fit_node(_node_vertex);

    _chisq_vertex = _node_vertex.getChisq();

    return _node_vertex.getFiltered().get_mom(px, py, pz);
    //return _node_vertex.getPredicted().get_mom(px, py, pz);
}

double KalmanTrack::getExpPosition()
{
    const TMatrixD& proj = _node_next.getProjector();
    const TMatrixD& state = _node_next.getPredicted()._state_kf;

#ifdef _DEBUG_ON
    LogInfo("Expected X: " << _node_next.getPredicted()._state_kf[3][0]);
    LogInfo("Expected Y: " << _node_next.getPredicted()._state_kf[4][0]);
#endif

    return (proj*state)[0][0];
}

double KalmanTrack::getExpPosError()
{
    const TMatrixD& proj = _node_next.getProjector();
    const TMatrixD& covar = _node_next.getPredicted()._covar_kf;
    //const TMatrixD& cov_m = _node_next.getMeasurementCov();

    //double err_x = sqrt(covar[3][3]);
    //double err_y = sqrt(covar[4][4]);
    //double track_err = fabs(proj[0][3]*err_x) + fabs(proj[0][4]*err_y);

    double track_err_sq = SMatrix::getABCt(proj, covar, proj)[0][0];
    double wire_spacing = GeomSvc::instance()->getPlaneSpacing(_node_next.getHit().detectorID);

    //LogInfo("measurement error: " << sqrt(cov_m[0][0]) << ", tracking error: " << track_err);
    return sqrt(wire_spacing*wire_spacing + track_err_sq);
}

Node* KalmanTrack::getNearestNodePtr(double z)
{
    Node *_node_prev = &(_nodes.front());
    double deltaZ_curr, deltaZ_prev;
    deltaZ_prev = 1E6;
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        deltaZ_curr = fabs(z - iter->getZ());
        if(deltaZ_curr > deltaZ_prev)
        {
            return _node_prev;
        }

        deltaZ_prev = deltaZ_curr;
        _node_prev = &(*iter);
    }

    return _node_prev;
}

void KalmanTrack::getExpPositionFast(double z, double& x, double& y, Node* _node)
{
    if(_node == nullptr)
    {
        _node = getNearestNodePtr(z);
    }

    TrkPar *_trkpar;
    if(_node->isSmoothDone())
    {
        _trkpar = &(_node->getSmoothed());
    }
    else if(_node->isFilterDone())
    {
        _trkpar = &(_node->getFiltered());
    }
    else
    {
        _trkpar = &(_node->getPredicted());
    }

    double z_ref = _trkpar->get_z();
    double x_ref = _trkpar->get_x();
    double y_ref = _trkpar->get_y();
    double axz = _trkpar->get_dxdz();
    double ayz = _trkpar->get_dydz();

    x = x_ref + axz*(z - z_ref);
    y = y_ref + ayz*(z - z_ref);
}

void KalmanTrack::getExpPosErrorFast(double z, double& dx, double& dy, Node* _node)
{
    if(_node == nullptr)
    {
        _node = getNearestNodePtr(z);
    }

    TrkPar *_trkpar;
    if(_node->isSmoothDone())
    {
        _trkpar = &(_node->getSmoothed());
    }
    else if(_node->isFilterDone())
    {
        _trkpar = &(_node->getFiltered());
    }
    else
    {
        _trkpar = &(_node->getPredicted());
    }

    double z_ref = _trkpar->get_z();
    double dx_ref = sqrt(fabs(_trkpar->_covar_kf[3][3]));
    double dy_ref = sqrt(fabs(_trkpar->_covar_kf[4][4]));
    double daxz = sqrt(fabs(_trkpar->_covar_kf[1][1]));
    double dayz = sqrt(fabs(_trkpar->_covar_kf[2][2]));

    dx = 2.*(dx_ref + fabs(daxz*(z - z_ref)));
    dy = 2.*(dy_ref + fabs(dayz*(z - z_ref)));
}

double KalmanTrack::getExpLocalSlop()
{
    const TMatrixD& proj = _node_next.getProjector();
    const TMatrixD& state = _node_next.getPredicted()._state_kf;


    return proj[0][3]*state[1][0] + proj[0][4]*state[2][0];
}

double KalmanTrack::getExpLcSlopErr()
{
    const TMatrixD& proj = _node_next.getProjector();
    const TMatrixD& covar = _node_next.getPredicted()._covar_kf;

    double err_x = sqrt(covar[1][1]);
    double err_y = sqrt(covar[2][2]);

    return fabs(proj[0][3]*err_x) + fabs(proj[0][4]*err_y);
}

Node *KalmanTrack::getNodeUpstream()
{
    Node* _node = nullptr;
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        int detectorID = iter->getHit().detectorID;
        if(detectorID <= 6)
        {
            _node = &(*iter);
        }
        else
        {
            break;
        }
    }

    return _node;
}

Node *KalmanTrack::getNodeDownstream()
{
    Node* _node = nullptr;
    for(std::list<Node>::reverse_iterator iter = _nodes.rbegin(); iter != _nodes.rend(); ++iter)
    {
        int detectorID = iter->getHit().detectorID;
        if(detectorID > 6)
        {
            _node = &(*iter);
        }
        else
        {
            break;
        }
    }

    return _node;
}

double KalmanTrack::getMomentumInStation(int stationID)
{
    int detectorID_begin = stationID*6 - 5;
    int detectorID_end = stationID*6;

    double p = -1.;
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        double detectorID = iter->getHit().detectorID;
        if(detectorID >= detectorID_begin && detectorID <= detectorID_end)
        {
            p = iter->getFiltered().get_mom();
            break;
        }
    }

    return p;
}

double KalmanTrack::getXZSlopeInStation(int stationID)
{
    int detectorID_begin = stationID*6 - 5;
    int detectorID_end = stationID*6;

    double axz = -100.;
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        double detectorID = iter->getHit().detectorID;
        if(detectorID >= detectorID_begin && detectorID <= detectorID_end)
        {
            axz = iter->getFiltered().get_dxdz();
            break;
        }
    }

    return axz;
}

double KalmanTrack::getPositionInStation(int stationID, double& x, double& y, double& z)
{
    int detectorID_begin = stationID*6 - 5;
    int detectorID_end = stationID*6;

    x = 9999;
    y = 9999;
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        double detectorID = iter->getHit().detectorID;
        if(detectorID >= detectorID_begin && detectorID <= detectorID_end)
        {
            z = iter->getZ();
            if(iter->isSmoothDone())
            {
                x = iter->getSmoothed().get_x();
                y = iter->getSmoothed().get_y();
            }
            else if(iter->isFilterDone())
            {
                x = iter->getFiltered().get_x();
                y = iter->getFiltered().get_y();
            }
            break;
        }
    }

    return sqrt(x*x + y*y);

}

int KalmanTrack::getKickCharge()
{
    double axz_st1 = getXZSlopeInStation(1);
    double axz_st2 = getXZSlopeInStation(2);

    return axz_st1 < axz_st2 ? -1 : 1;
}

double KalmanTrack::getMomentumUpstream(double& px, double& py, double& pz)
{
    if(_nodes.front().isSmoothDone())
    {
        return _nodes.front().getSmoothed().get_mom(px, py, pz);
    }
    else if(_nodes.front().isFilterDone())
    {
        return _nodes.front().getFiltered().get_mom(px, py, pz);
    }
    else
    {
        return _nodes.front().getPredicted().get_mom(px, py, pz);
    }
}

int KalmanTrack::getNHitsInStation(int stationID)
{
    int detectorID_begin = stationID*6 - 5;
    int detectorID_end = stationID*6;

    int nHits = 0;
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        int detectorID = iter->getHit().detectorID;
        if(detectorID >= detectorID_begin && detectorID <= detectorID_end)
        {
            nHits++;
        }
    }

    return nHits;
}

void KalmanTrack::update()
{
    double nHits = _hit_index.size();

    _chisq = 0.;
    for(std::list<Node>::iterator node = _nodes.begin(); node != _nodes.end(); ++node)
    {
        _chisq += node->getChisq();
    }

    _quality = nHits - 0.4*_chisq;
}

bool KalmanTrack::operator<(const KalmanTrack& elem) const
{
    return _quality > elem._quality;
}

bool KalmanTrack::operator==(const KalmanTrack& elem) const
{
    if(_hit_index.size() != elem._hit_index.size()) return false;
    if(fabs(_quality - elem._quality) > 1E-6) return false;

    return true;
}

bool KalmanTrack::similarity(const KalmanTrack& elem) const
{
    std::list<int> _hit_index1, _hit_index2;

    _hit_index1.clear();
    for(std::list<int>::const_iterator iter = _hit_index.begin(); iter != _hit_index.end(); ++iter)
    {
        _hit_index1.push_back(abs(*iter));
    }
    _hit_index1.sort();

    _hit_index2.clear();
    for(std::list<int>::const_iterator iter = elem._hit_index.begin(); iter != elem._hit_index.end(); ++iter)
    {
        _hit_index2.push_back(abs(*iter));
    }
    _hit_index2.sort();

    std::list<int> commonHits;
    commonHits.clear();
    set_intersection(_hit_index1.begin(), _hit_index1.end(), _hit_index2.begin(), _hit_index2.end(), back_inserter(commonHits));

    double nHits_original = double(_hit_index1.size());
    double nHits_common = double(commonHits.size());

    if(nHits_common/nHits_original > 0.2) return true;
    return false;
}


bool KalmanTrack::addHit(Hit _hit)
{
    _hit_index.push_front(_hit.index);

    Node _node(_hit);
    KalmanFilter *_kmfit = KalmanFilter::instance();

    _node.getPredicted() = _node_next.getPredicted();
    _node.setPredictionDone();
    if(_kmfit->filter(_node))
    {
        _nodes.push_front(_node);
        update();

        _trkpar_curr = _node.getFiltered();

        return true;
    }
    else
    {
        return false;
    }
}

int KalmanTrack::getAlignment(int level, int *detectorID, double *res, double *R, double *T)
{
    if(level != 1 && level != 2 && level != 3)
    {
        std::cerr << "Wrong output level selection! " << std::endl;
        return 0;
    }

    int nHits = 0;
    double x, y;

    GeomSvc *p_geomSvc = GeomSvc::instance();
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        if(level == 3)
        {
            x = iter->getSmoothed().get_x();
            y = iter->getSmoothed().get_y();
            res[nHits] = iter->getSmoothedResidual()[0][0];
        }
        else if(level == 2)
        {
            x = iter->getFiltered().get_x();
            y = iter->getFiltered().get_y();
            res[nHits] = iter->getFilteredResidual()[0][0];
        }
        else if(level == 1)
        {
            x = iter->getPredicted().get_x();
            y = iter->getPredicted().get_y();
            res[nHits] = iter->getPredictedResidual()[0][0];
        }

        detectorID[nHits] = iter->getHit().detectorID;
        R[nHits] = x*p_geomSvc->getCostheta(detectorID[nHits]) + y*p_geomSvc->getSintheta(detectorID[nHits]) - iter->getHit().pos;
        T[nHits] = iter->getHit().tdcTime;

        nHits++;
    }

    return nHits;
}

int KalmanTrack::getPositions(int level, double *x, double *y, double *z)
{
    if(level != 1 && level != 2 && level != 3)
    {
        std::cerr << "Wrong output level selection! " << std::endl;
        return 0;
    }

    int nHits = 0;
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        z[nHits] = iter->getZ();
        if(level == 3)
        {
            x[nHits] = iter->getSmoothed().get_x();
            y[nHits] = iter->getSmoothed().get_y();
        }
        else if(level == 2)
        {
            x[nHits] = iter->getFiltered().get_x();
            y[nHits] = iter->getFiltered().get_y();
        }
        else if(level == 1)
        {
            x[nHits] = iter->getPredicted().get_x();
            y[nHits] = iter->getPredicted().get_y();
        }

        nHits++;
    }

    return nHits;
}

int KalmanTrack::getMomentums(int level, double *px, double *py, double *pz)
{
    if(level != 1 && level != 2 && level != 3)
    {
        std::cerr << "Wrong output level selection! " << std::endl;
        return 0;
    }

    int nHits = 0;
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        double m_px, m_py, m_pz;
        if(level == 3)
        {
            iter->getSmoothed().get_mom(m_px, m_py, m_pz);
        }
        else if(level == 2)
        {
            iter->getFiltered().get_mom(m_px, m_py, m_pz);
        }
        else
        {
            iter->getPredicted().get_mom(m_px, m_py, m_pz);
        }

        px[nHits] = m_px;
        py[nHits] = m_py;
        pz[nHits] = m_pz;

        nHits++;
    }

    return nHits;
}

int KalmanTrack::getHitsIndex(int *index)
{
    int nHits = 0;
    for(std::list<int>::iterator iter = _hit_index.begin(); iter != _hit_index.end(); ++iter)
    {
        index[nHits] = *iter;
        nHits++;
    }

    return nHits;
}

std::vector<int> KalmanTrack::getMissedDetectorIDs()
{
    std::vector<int> detectorIDs_all;
    for(int i = 1; i <= 12; ++i) detectorIDs_all.push_back(i);
    if(_nodes.back().getHit().detectorID < 19)
    {
        for(int i = 13; i <= 18; ++i) detectorIDs_all.push_back(i);
    }
    else
    {
        for(int i = 19; i <= 24; ++i) detectorIDs_all.push_back(i);
    }

    std::vector<int> detectorIDs_now;
    detectorIDs_now.clear();
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        detectorIDs_now.push_back(iter->getHit().detectorID);
    }
    std::sort(detectorIDs_now.begin(), detectorIDs_now.end());

    std::vector<int> detectorIDs_miss(18);
    std::vector<int>::iterator iter = std::set_difference(detectorIDs_all.begin(), detectorIDs_all.end(), detectorIDs_now.begin(), detectorIDs_now.end(), detectorIDs_miss.begin());
    detectorIDs_miss.resize(iter - detectorIDs_miss.begin());

    return detectorIDs_miss;
}

void KalmanTrack::getSagittaInSuperDetector(int detectorID, double& pos_exp, double& window)
{
    if(detectorID > 3 || detectorID < 1) return;
    GeomSvc *p_geomSvc = GeomSvc::instance();

    ///DetectorID of the relevant view of D1U, D1X and D1V
    int detectorID_st2[3] = {12, 10, 8};
    double ratio[3] = {1.9, 1.80, 1.7};
    double sigma[3] = {0.2, 0.2, 0.2};

    double x_st3, y_st3, z_st3;
    z_st3 = _nodes.back().getZ();
    if(_nodes.back().isSmoothDone())
    {
        x_st3 = _nodes.back().getSmoothed().get_x();
        y_st3 = _nodes.back().getSmoothed().get_y();
    }
    else if(_nodes.back().isFilterDone())
    {
        x_st3 = _nodes.back().getFiltered().get_x();
        y_st3 = _nodes.back().getFiltered().get_y();
    }
    else
    {
        x_st3 = _nodes.back().getPredicted().get_x();
        y_st3 = _nodes.back().getPredicted().get_y();
    }
    double pos_st3 = x_st3*p_geomSvc->getCostheta(_nodes.back().getHit().detectorID) + y_st3*p_geomSvc->getSintheta(_nodes.back().getHit().detectorID);

    double x_st2, y_st2, z_st2;
    z_st2 = p_geomSvc->getPlanePosition(detectorID_st2[detectorID-1]);
    getExpPositionFast(z_st2, x_st2, y_st2);
    double pos_st2 = x_st2*p_geomSvc->getCostheta(detectorID_st2[detectorID-1]) + y_st2*p_geomSvc->getSintheta(detectorID_st2[detectorID-1]);
    double sagitta_st2 = pos_st2 - pos_st3/z_st3*z_st2;

    double z_st1 = p_geomSvc->getPlanePosition(2*detectorID);
    pos_exp =  sagitta_st2*ratio[detectorID-1] + pos_st3/z_st3*z_st1;
    window = fabs(6.*sagitta_st2*sigma[detectorID-1]);
}

TGraph KalmanTrack::getXZProjection()
{
    double x[20], y[20], z[20];
    getPositions(3, x, y, z);

    TGraph gr(getNHits(), z, x);
    return gr;
}

SRecTrack KalmanTrack::getSRecTrack()
{
    SRecTrack _strack;

    _strack.setChisq(_chisq);
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        _strack.insertHitIndex(iter->getHit().index);
        _strack.insertStateVector(iter->getSmoothed()._state_kf);
        _strack.insertCovariance(iter->getSmoothed()._covar_kf);
        _strack.insertZ(iter->getZ());
        _strack.insertChisq(iter->getChisq());
    }

    _strack.swimToVertex();
    _strack.setKalmanStatus(1);
    return _strack;
}

void KalmanTrack::print()
{
    std::cout << "=============== Kalman finder candidate track ==================" << std::endl;
    std::cout << "This candidate has " << _hit_index.size() << " hits!" << std::endl;

    for(std::list<int>::iterator iter = _hit_index.begin(); iter != _hit_index.end(); ++iter)
    {
        std::cout << *iter << " : ";
    }
    std::cout << std::endl;

    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        std::cout << iter->getHit().detectorID << " : ";
    }
    std::cout << std::endl;

    std::cout << "Most upstream momentum is: " << 1./fabs(_trkpar_curr._state_kf[0][0]) << std::endl;
    std::cout << "Most upstream position is: " << _trkpar_curr.get_z() << std::endl;
    std::cout << "Current slope:    X-Z = " << _trkpar_curr._state_kf[1][0] << ", Y-Z = " << _trkpar_curr._state_kf[2][0] << std::endl;
    std::cout << "Current position: X = " << _trkpar_curr._state_kf[3][0] << ", Y = " << _trkpar_curr._state_kf[4][0] << std::endl;
    std::cout << "Quality is: " << _quality << std::endl;
    std::cout << "Chisq/d.o.f = " << _chisq/_hit_index.size() << std::endl;
    if(!_hit_index.empty()) std::cout << "Charge      = " << getCharge() << std::endl;
    if(!_hit_index.empty()) std::cout << "Assigned Charge  = " << getAssignedCharge() << std::endl;
    if(!_hit_index.empty()) std::cout << "Kick Charge      = " << getKickCharge() << std::endl;

    //std::cout << "Using this seed as start: " << std::endl;
    //_seed.print();
}

void KalmanTrack::printNodes()
{
    std::cout << "The content of all Nodes of this track ... " << std::endl;
    for(std::list<Node>::reverse_iterator node = _nodes.rbegin(); node != _nodes.rend(); ++node)
    {
        node->print(true);
    }
}
