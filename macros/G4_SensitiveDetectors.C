/** @file
 * @brief Macro to configure the sensitive volumes.
 *
 * You first include this macro in main macro like `Fun4All.C`;
 * @code
 * #include <top/G4_SensitiveDetectors.C>
 * @endcode
 *
 * You then call the macro function;
 * @code
 * SetupSensitiveDetectors(g4Reco);
 * @endcode
 * You can give function arguments to change the behavior.
 *
 * If you want to modify the macro function itself,
 * you can copy the macro file to your local directory, modify it and include it via;
 * @code
 * #include "G4_SensitiveDetectors.C"
 * @endcode
 */
#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <g4detectors/PHG4BlockSubsystem.h>
class SubsysReco;
#endif

#include <iostream>
#include <g4detectors/SQDigitizer.h>
typedef SQDigitizer DPDigitizer;  //so that the naming DPDigitizer is still avaiable for backward compatibility

/// Macro function to configure the sensitive volumes.
/**
 * This macro implements the all the volumes that are sensitive, which generate G4Hits.
 */
void SetupSensitiveDetectors(
  PHG4Reco*   g4Reco,
  bool        toggle_dphodo = true,
  bool        toggle_dc1    = false,
  std::string chamberGas    = "SQ_ArCO2",
  std::string hodoMat       = "SQ_Scintillator",
  const int   verbosity     = 0)
{
  GeomSvc* geom_svc = GeomSvc::instance();
  for(int i = 1; i <= nChamberPlanes+nHodoPlanes+nPropPlanes+nDarkPhotonPlanes; ++i) 
  {
    //D1 is disabled by default
    if(!toggle_dc1    && i >= 7 && i <= 12) continue;
    if(!toggle_dphodo && i > nChamberPlanes+nHodoPlanes+nPropPlanes) continue;

    std::string detectorName = geom_svc->getDetectorName(i);

    double size[3];
    double place[3];
    double rot[3];
    place[0] = geom_svc->getPlaneCenterX(i);
    place[1] = geom_svc->getPlaneCenterY(i);
    place[2] = geom_svc->getPlaneCenterZ(i);
    rot[0]   = geom_svc->getRotationInX(i);
    rot[1]   = geom_svc->getRotationInY(i);
    rot[2]   = geom_svc->getRotationInZ(i);
    size[0]  = geom_svc->getPlaneScaleX(i);
    size[1]  = geom_svc->getPlaneScaleY(i);
    size[2]  = geom_svc->getPlaneScaleZ(i);

    if(verbosity != 0)
    {
      std::cout << i << "  " << detectorName << " - "
                << place[0] << " " << place[1] << " " << place[2] << " - "
                << rot[0] << " " << rot[1] << " " << rot[2] << " - "
                << size[0] << " " << size[1] << " " << size[2] << std::endl;
    }
    
    PHG4BlockSubsystem* det = new PHG4BlockSubsystem(detectorName.c_str(), 0);
    det->SuperDetector(detectorName.c_str());
    det->set_double_param("size_x",  size[0]);
    det->set_double_param("size_y",  size[1]);
    det->set_double_param("size_z",  size[2]);
    det->set_double_param("place_x", place[0]);
    det->set_double_param("place_y", place[1]);
    det->set_double_param("place_z", place[2]);
    det->set_double_param("rot_x",   rot[0]);
    det->set_double_param("rot_y",   rot[1]);
    det->set_double_param("rot_z",   rot[2]);
    if(i <= nChamberPlanes || (i > nChamberPlanes+nHodoPlanes && i <= nChamberPlanes+nHodoPlanes+nPropPlanes))
    {
      det->set_string_param("material", chamberGas.c_str());
    }
    else
    {
      det->set_string_param("material", hodoMat.c_str());
    }
    det->SetActive(1);
    g4Reco->registerSubsystem(det);
  }

  return;
}
