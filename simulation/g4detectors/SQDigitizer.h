#ifndef SQDigitizer_H
#define SQDigitizer_H

#include <fun4all/SubsysReco.h>
#include <geom_svc/GeomSvc.h>

#include <vector>
#include <string>
#include <map>

#ifndef __CINT__
#include <boost/array.hpp>
#include <Geant4/G4ThreeVector.hh>
#endif

#include <TSpline.h>

class PHG4Hit;
class SQHit;
class SQHitVector;
class PHG4HitContainer;

class SQDigitizer: public SubsysReco
{
public:
  SQDigitizer(const std::string& name = "SQDigitizer", const int verbose = 0);
  virtual ~SQDigitizer();

#ifndef __CINT__
  int Init(PHCompositeNode* topNode);
#endif

  //! module initialization
  int InitRun(PHCompositeNode* topNode);

    //! event processing
  int process_event(PHCompositeNode* topNode);

  //!main external call, fill the digi hit vector
  void digitizePlane(const std::string& detName);

  //!digitize the emcal hits
  void digitizeEMCal(const std::string& detName);

  //!Get the trigger level by detectorID
  int getTriggerLv(int detectorID) { return p_geomSvc->getTriggerLv(detectorID); }

  //!Register additional EMCal detector for digitizing
  void registerEMCal(std::string ecalName, int ecalID = 100) { detIDByName[ecalName] = ecalID; }

  //!enable/disable certain detectors
  void set_enable_st1dc(const bool en)  { enableDC1 = en; }
  void set_enable_dphodo(const bool en) { enableDPHodo = en; }

  void set_digitize_secondaries(const bool val) { digitize_secondaries = val; }

private:
  //!GeomSvc
  GeomSvc* p_geomSvc;

  //!output node - digitized hits
  SQHitVector* digits;

  //!input node - G4HitContainers
  std::map<std::string, PHG4HitContainer*> hitContainerByName;

  //!Auxillary container
  std::map<std::string, int> detIDByName;

  //!flags to toggle station-1 DC and dark photon dp
  bool enableDC1;
  bool enableDPHodo;
  bool digitize_secondaries;
};

#endif
