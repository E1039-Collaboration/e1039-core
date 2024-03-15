#include "GFFitter.h"

#include <GenFit/DAF.h>
#include <GenFit/EventDisplay.h>
#include <GenFit/FieldManager.h>
#include <GenFit/FitStatus.h>
#include <GenFit/KalmanFitter.h>
#include <GenFit/KalmanFitterRefTrack.h>
#include <GenFit/MaterialEffects.h>
#include <GenFit/TGeoMaterialInterface.h>

namespace SQGenFit
{
GFFitter::GFFitter(): _verbosity(0), _kmfitter(nullptr), _display(nullptr)
{}

GFFitter::~GFFitter()
{
  if(_kmfitter != nullptr) delete _kmfitter;
  if(_display != nullptr) delete _display;
}

void GFFitter::setVerbosity(unsigned int v)
{
  _verbosity = v;
  if(_kmfitter != nullptr) _kmfitter->setDebugLvl(_verbosity);
}

void GFFitter::init(GFField* field, const TString& fitter_choice)
{
  genfit::FieldManager::getInstance()->init(field);
  genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());

  _fitterTy = fitter_choice;
  if(fitter_choice == "KalmanFitterRefTrack")
    _kmfitter = new genfit::KalmanFitterRefTrack();
  else if(fitter_choice == "KalmanFitter")
    _kmfitter = new genfit::KalmanFitter();
  else if(fitter_choice == "DafSimple")
    _kmfitter = new genfit::DAF(false);
  else if(fitter_choice == "DafRef")
    _kmfitter = new genfit::DAF(true);
  else
    _kmfitter = new genfit::KalmanFitter();

  _kmfitter->setMaxIterations(10);
  _kmfitter->setDebugLvl(_verbosity);
}

int GFFitter::processTrack(GFTrack& track, bool display)
{
  //check before fit
  try
  {
    track.checkConsistency();
  }
  catch(genfit::Exception& e)
  {
    std::cerr << "Track consistency error before fit: " << e.what() << std::endl;
    return -1;
  }

  //Do the fit
  genfit::Track* gftrack = track.getGenFitTrack();
  try
  {
    _kmfitter->processTrack(gftrack);
  }
  catch(genfit::Exception& e)
  {
    std::cerr << "Track fitting failed: " << e.what() << std::endl;
    return -2;
  }
  
  //check after fit
  try
  {
    track.checkConsistency();
  }
  catch(genfit::Exception& e)
  {
    std::cerr << "Track consistency error after fit: " << e.what() << std::endl;
    return -3;
  }

  genfit::AbsTrackRep* rep = gftrack->getCardinalRep();
  if(!gftrack->getFitStatus(rep)->isFitConverged())
  {
    std::cerr << "Fit failed to converge." << std::endl;
    return -4;
  }

  if(display)
  {
    if(_display == nullptr) _display = genfit::EventDisplay::getInstance();
    _display->addEvent(gftrack);
  }

  return 0;
}

void GFFitter::displayEvent()
{
  if(_display != nullptr) _display->open();
}

}