#include <cstdlib>
#include <string>
#include <iostream>
#include <iomanip>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <phool/phool.h>
#include <phool/getClass.h>
#include <phool/PHNode.h>
#include <phool/PHNodeIOManager.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQRun.h>
#include <ktracker/SRecEvent.h>
#include <interface_main/SQSpillMap.h>
#include <interface_main/SQHitVector.h>
#include <interface_main/SQTrackVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include "Fun4AllRUSOutputManager.h"
#include <fun4all/Fun4AllOutputManager.h>

using namespace std;

Fun4AllRUSOutputManager::Fun4AllRUSOutputManager(const std::string &myname)
    : Fun4AllOutputManager(myname),
    m_file(0),
    m_tree(0),
    true_mode(true),
    exp_mode(false),
    m_tree_name("tree"),
    m_file_name("output.root"),
    m_evt(0),
    m_vec_trk(0),
    m_sp_map(0),
    sq_run(0),
    m_compression_level(5),
    m_basket_size(64000),
    m_auto_flush(2500),
    process_id(14),
    source_flag(1),
    m_hit_vec(0)
{
    ;
}

Fun4AllRUSOutputManager::~Fun4AllRUSOutputManager() {
    CloseFile();
}

int Fun4AllRUSOutputManager::OpenFile(PHCompositeNode* startNode) {
    std::cout << "Fun4AllRUSOutputManager::OpenFile(): Attempting to open file: " << m_file_name << " with tree: " << m_tree_name << std::endl;
    m_file = new TFile(m_file_name.c_str(), "RECREATE");
    m_file->SetCompressionAlgorithm(ROOT::kLZMA);
    m_file->SetCompressionLevel(m_compression_level);

    if (!m_file || m_file->IsZombie()) {
        std::cerr << "Error: Could not create file " << m_file_name << std::endl;
        exit(1);
    } else {
        std::cout << "File " << m_file->GetName() << " opened successfully." << std::endl;
    }

    m_tree = new TTree(m_tree_name.c_str(), "Tree for storing events");
    if (!m_tree) {
        std::cerr << "Error: Could not create tree " << m_tree_name << std::endl;
        exit(1);
    } else {
        std::cout << "Tree " << m_tree->GetName() << " created successfully." << std::endl;
    }

    m_tree->Branch("runID", &runID, "runID/I");
    m_tree->Branch("eventID", &eventID, "eventID/I");

    if (sqevent_flag){
        m_tree->Branch("spillID", &spillID, "spillID/I");
        m_tree->Branch("rfID", &rfID, "rfID/I");
        m_tree->Branch("turnID", &turnID, "turnID/I");
        m_tree->Branch("rfIntensity", rfIntensity, "rfIntensity[33]/I");
        m_tree->Branch("fpgaTrigger", fpgaTrigger, "fpgaTrigger[5]/I");
        m_tree->Branch("nimTrigger", nimTrigger, "nimTrigger[5]/I");
    }
    m_tree->Branch("hitID", &hitID);
    m_tree->Branch("hit_trackID", &hit_trackID);
    m_tree->Branch("detectorID", &detectorID);
    m_tree->Branch("elementID", &elementID);
    m_tree->Branch("tdcTime", &tdcTime);
    m_tree->Branch("driftDistance", &driftDistance);


    if (true_mode) {
        m_tree->Branch("processID", &processID); //Hit level
        m_tree->Branch("gCharge", &gCharge);
        m_tree->Branch("trackID", &trackID);
        m_tree->Branch("gvx", &gvx);
        m_tree->Branch("gvy", &gvy);
        m_tree->Branch("gvz", &gvz);
        m_tree->Branch("gpx", &gpx);
        m_tree->Branch("gpy", &gpy);
        m_tree->Branch("gpz", &gpz);
        m_tree->Branch("gx_st1", &gx_st1);
        m_tree->Branch("gy_st1", &gy_st1);
        m_tree->Branch("gz_st1", &gz_st1);
        m_tree->Branch("gpx_st1", &gpx_st1);
        m_tree->Branch("gpy_st1", &gpy_st1);
        m_tree->Branch("gpz_st1", &gpz_st1);
        m_tree->Branch("gx_st3", &gx_st3);
        m_tree->Branch("gy_st3", &gy_st3);
        m_tree->Branch("gz_st3", &gz_st3);
        m_tree->Branch("gpx_st3", &gpx_st3);
        m_tree->Branch("gpy_st3", &gpy_st3);
        m_tree->Branch("gpz_st3", &gpz_st3);
    }

    m_tree->SetAutoFlush(m_auto_flush);
    m_tree->SetBasketSize("*", m_basket_size);

    m_evt = findNode::getClass<SQEvent>(startNode, "SQEvent");
    m_hit_vec = findNode::getClass<SQHitVector>(startNode, "SQHitVector");

    if (true_mode) {
        m_vec_trk = findNode::getClass<SQTrackVector>(startNode, "SQTruthTrackVector");
        if (!m_vec_trk) {
            return Fun4AllReturnCodes::ABORTEVENT;
        }
    }

    if (!m_evt || !m_hit_vec) {
        return Fun4AllReturnCodes::ABORTEVENT;
    }

    return Fun4AllReturnCodes::EVENT_OK;
}
int Fun4AllRUSOutputManager::Write(PHCompositeNode* startNode) {
    if (!m_file || !m_tree) {
        OpenFile(startNode);
    }

    if (sqevent_flag){
        runID = m_evt->get_run_id();
        spillID = m_evt->get_spill_id();
        eventID = m_evt->get_event_id();
        turnID =  m_evt->get_qie_turn_id();
        rfID = m_evt->get_qie_rf_id ();

        fpgaTrigger[0] = m_evt->get_trigger(SQEvent::MATRIX1);
        fpgaTrigger[1] = m_evt->get_trigger(SQEvent::MATRIX2);
        fpgaTrigger[2] = m_evt->get_trigger(SQEvent::MATRIX3);
        fpgaTrigger[3] = m_evt->get_trigger(SQEvent::MATRIX4);
        fpgaTrigger[4] = m_evt->get_trigger(SQEvent::MATRIX5);

        nimTrigger[0] = m_evt->get_trigger(SQEvent::NIM1);
        nimTrigger[1] = m_evt->get_trigger(SQEvent::NIM2);
        nimTrigger[2] = m_evt->get_trigger(SQEvent::NIM3);
        nimTrigger[3] = m_evt->get_trigger(SQEvent::NIM4);
        nimTrigger[4] = m_evt->get_trigger(SQEvent::NIM5);
        for (int i = -16; i <= 16; ++i) {
            rfIntensity[i+ 16] = m_evt->get_qie_rf_intensity(i);
        }
    }
    if (m_hit_vec && exp_mode==true) {
        ResetHitBranches();
        for (int ihit = 0; ihit < m_hit_vec->size(); ++ihit) {
            SQHit* hit = m_hit_vec->at(ihit);
            hit_trackID.push_back(hit->get_track_id());
            hitID.push_back(hit->get_hit_id());
            detectorID.push_back(hit->get_detector_id());
            elementID.push_back(hit->get_element_id());
            tdcTime.push_back(hit->get_tdc_time());
            driftDistance.push_back(hit->get_drift_distance());
        }
    }
    if(true_mode == true){
        ResetTrueTrackBranches();
        ResetHitBranches();
        for (unsigned int ii = 0; ii < m_vec_trk->size(); ii++) {
            SQTrack* trk = m_vec_trk->at(ii);

            gCharge.push_back(trk->get_charge());
            trackID.push_back(trk->get_track_id());
            gvx.push_back(trk->get_pos_vtx().X());
            gvy.push_back(trk->get_pos_vtx().Y());
            gvz.push_back(trk->get_pos_vtx().Z());
            gpx.push_back(trk->get_mom_vtx().Px());
            gpy.push_back(trk->get_mom_vtx().Py());
            gpz.push_back(trk->get_mom_vtx().Pz());

            gx_st1.push_back(trk->get_pos_st1().X());
            gy_st1.push_back(trk->get_pos_st1().Y());
            gz_st1.push_back(trk->get_pos_st1().Z());
            gpx_st1.push_back(trk->get_mom_st1().Px());
            gpy_st1.push_back(trk->get_mom_st1().Py());
            gpz_st1.push_back(trk->get_mom_st1().Pz());

            gx_st3.push_back(trk->get_pos_st3().X());
            gy_st3.push_back(trk->get_pos_st3().Y());
            gz_st3.push_back(trk->get_pos_st3().Z());
            gpx_st3.push_back(trk->get_mom_st3().Px());
            gpy_st3.push_back(trk->get_mom_st3().Py());
            gpz_st3.push_back(trk->get_mom_st3().Pz());

            //This assignment will later need to be done using a DST node, for example, a Geant4 geometry data node to set the limits.
            double z = trk->get_pos_vtx().Z();
            if (z <= -296. && z >= -304.) source_flag = 1; // target
            else if (z >= 0. && z < 500) source_flag = 2;   // dump
            else if (z > -296. && z < 0.) source_flag = 3;  // gap
            else if (z < -304) source_flag = 0;              // upstream

            if (m_hit_vec) {
                for (int ihit = 0; ihit < m_hit_vec->size(); ++ihit) {
                    SQHit* hit = m_hit_vec->at(ihit);
                    if(hit->get_track_id() != trk->get_track_id()) continue;
                    int process_id_ = (trk->get_charge() > 0) ? process_id : process_id + 10;
                    unsigned int encodedValue = EncodeProcess(process_id_, source_flag);
                    //cout << "charge: "<< trk->get_charge() <<endl;
                    //cout << "encodedValue: "<< encodedValue<< endl;
                    //cout << "DecodeSourceFlag: "<< DecodeSourceFlag(encodedValue) << endl;
                    //cout << "DecodeProcessID: "<< DecodeProcessID(encodedValue) << endl;
                    processID.push_back(encodedValue);
                    hitID.push_back(hit->get_hit_id());
                    hit_trackID.push_back(hit->get_track_id());
                    detectorID.push_back(hit->get_detector_id());
                    elementID.push_back(hit->get_element_id());
                    tdcTime.push_back(hit->get_tdc_time());
                    driftDistance.push_back(hit->get_drift_distance());
                }   
            }
        }
    }
    m_tree->Fill();
    return 0;
}

void Fun4AllRUSOutputManager::CloseFile() {
    if (!m_file) return;
    std::cout << "Fun4AllRUSOutputManager::CloseFile(): Closing file: " << m_file_name << std::endl;
    m_file->Write();
    m_file->Close();
    delete m_file;
    m_file = nullptr;
}

void Fun4AllRUSOutputManager::ResetHitBranches() {
    hitID.clear();
    hit_trackID.clear();
    detectorID.clear();
    elementID.clear();
    tdcTime.clear();
    driftDistance.clear();
}

void Fun4AllRUSOutputManager::ResetTrueTrackBranches() {
    gCharge.clear();
    trackID.clear();
    gvx.clear(); gvy.clear(); gvz.clear();
    gpx.clear(); gpy.clear(); gpz.clear();
    gx_st1.clear(); gy_st1.clear(); gz_st1.clear();
    gpx_st1.clear(); gpy_st1.clear(); gpz_st1.clear();
    gx_st3.clear(); gy_st3.clear(); gz_st3.clear();
    gpx_st3.clear(); gpy_st3.clear(); gpz_st3.clear();
    processID.clear();
}


unsigned int Fun4AllRUSOutputManager::EncodeProcess(int processID, int sourceFlag) {
    return ((sourceFlag & 0x3) << 5) |  // 2 bits for sourceFlag (0-3)
        (processID & 0x1F);          // 5 bits for processID (0-31)
}
