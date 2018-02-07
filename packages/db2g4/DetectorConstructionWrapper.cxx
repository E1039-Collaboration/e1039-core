/*
 * DetectorConstructionWrapper.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */


#include "DetectorConstruction.hh"

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/PHTFileServer.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

//GEANT4
#include <G4GDMLParser.hh>

#include <TFile.h>

#include <cstring>
#include <cmath>
#include <cfloat>
#include <stdexcept>
#include <limits>

#define LogDebug(exp)		    std::cout<<"DEBUG: "  <<__FUNCTION__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogError(exp)		    std::cout<<"ERROR: "  <<__FUNCTION__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogWarning(exp)	    std::cout<<"WARNING: "<<__FUNCTION__<<": "<<__LINE__<<": "<< exp << std::endl

DetectorConstructionWrapper::DetectorConstructionWrapper(
		const std::string& name):
		_geometrySchema("geometry_G17_run3"),
		_fMagStr(FMAGSTR),
		_kMagStr(KMAGSTR),
		_out_name("geom.gdml"),
		_jobOptsSvc(nullptr)
{}

int DetectorConstructionWrapper::Init(PHCompositeNode* topNode) {

  Settings *mySettings = new Settings();
  mySettings->geometrySchema = _geometrySchema.c_str();

  if(fabs(_fMagStr) < 10.) mySettings->fMagMultiplier = _fMagStr;
  if(fabs(_kMagStr) < 10.) mySettings->kMagMultiplier = _kMagStr;

  auto detector = new DetectorConstruction(mySettings);

	G4GDMLParser parser;

	parser.Write(_out_name.c_str(), detector->Construct());

	return Fun4AllReturnCodes::EVENT_OK;
}

int DetectorConstructionWrapper::End(PHCompositeNode* topNode) {

	return Fun4AllReturnCodes::EVENT_OK;
}
