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
#include <fun4all/Fun4AllReturnCodes.h>
#include "Fun4AllRUSOutputManager.h"
#include <fun4all/Fun4AllOutputManager.h>

using namespace std;

Fun4AllRUSOutputManager::Fun4AllRUSOutputManager(const std::string &myname)
  : Fun4AllOutputManager(myname),
    m_file(0),
    m_tree(0),
    m_tree_name("tree"),
    m_file_name("output.root"),
    m_evt(0),
    m_sp_map(0),
    sq_run(0),
   m_compression_level(5),
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
	m_tree->Branch("spillID", &spillID, "spillID/I");
	m_tree->Branch("eventID", &eventID, "eventID/I");
	m_tree->Branch("rfID", &rfID, "rfID/I");
	m_tree->Branch("turnID", &rfID, "turnID/I");
	m_tree->Branch("rfIntensity", rfIntensity, "rfIntensity[33]/I");
	m_tree->Branch("fpgaTrigger", fpgaTrigger, "fpgaTrigger[5]/I");
	m_tree->Branch("nimTrigger", nimTrigger, "nimTrigger[5]/I");
	m_tree->Branch("hitID", &hitID);
	m_tree->Branch("detectorID", &detectorID);
	m_tree->Branch("elementID", &elementID);
	m_tree->Branch("tdcTime", &tdcTime);
	m_tree->Branch("driftDistance", &driftDistance);

	m_evt = findNode::getClass<SQEvent>(startNode, "SQEvent");
	m_hit_vec = findNode::getClass<SQHitVector>(startNode, "SQHitVector");

	if (!m_evt || !m_hit_vec) {
		return Fun4AllReturnCodes::ABORTEVENT;
	}
	
	return Fun4AllReturnCodes::EVENT_OK;
}
int Fun4AllRUSOutputManager::Write(PHCompositeNode* startNode) {
	if (!m_file || !m_tree) {
		OpenFile(startNode);
	}

	ResetBranches();

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

if (m_hit_vec) {
    for (int ihit = 0; ihit < m_hit_vec->size(); ++ihit) {
        SQHit* hit = m_hit_vec->at(ihit);
        hitID.push_back(hit->get_hit_id());
        detectorID.push_back(hit->get_detector_id());
        elementID.push_back(hit->get_element_id());
        tdcTime.push_back(hit->get_tdc_time());
        driftDistance.push_back(hit->get_drift_distance());
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

void Fun4AllRUSOutputManager::ResetBranches() {
	hitID.clear();
	detectorID.clear();
	elementID.clear();
	tdcTime.clear();
	driftDistance.clear();
}
