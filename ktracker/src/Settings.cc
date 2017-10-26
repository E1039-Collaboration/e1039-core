#include "Settings.hh"
#include "JobOptsSvc.h"
#include "GlobalConsts.h"

Settings::Settings()
{
  //  These are the settings that will be used by the monte carlo if parameters are not specified.

  JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();

  seed = 0;
  beamMomentum = 120*GeV;
  beamCurrent = 2e12;
  asciiFieldMap = true;
  generator = "gun";
  energyCut = 1.0*GeV;
  recordMethod = "hits";
  eventPos = "both";
  dimuonSource = "both";
  login = "seaguest";
  outputFileName = "test_default";
  password = "qqbar2mu+mu-";
  fMagName = p_jobOptsSvc->m_fMagFile;
  kMagName = p_jobOptsSvc->m_kMagFile;
  sqlServer = p_jobOptsSvc->m_mySQLInputServer;
  sqlPort = p_jobOptsSvc->m_mySQLInputPort;
  dimuonRepeat = 1;
  ironOn = true;
  trackingZCut = 400*cm;
  trackingEnergyCut = 1.0*GeV;
  if(p_jobOptsSvc->m_alignmentMode)
    {
      kMagMultiplier = 0.;
      fMagMultiplier = 0.;
    }
  else if(p_jobOptsSvc->m_mcMode)
    {
      kMagMultiplier = 1.;
      fMagMultiplier = 1.;
    }
  else
    {
      kMagMultiplier = KMAGSTR;
      fMagMultiplier = FMAGSTR;
    }

  geometrySchema = p_jobOptsSvc->m_geomVersion;
  magnetSchema = "geometry_R996_magneticFields";
  target = 1;
  pythia_shower = true;
  bucket_size = 40000;
}
