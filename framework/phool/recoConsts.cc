#include "recoConsts.h"

#include <iostream>
#include <wordexp.h>

recoConsts* recoConsts::__instance = nullptr;
recoConsts* recoConsts::instance()
{
  if(__instance == nullptr)
  {
    __instance = new recoConsts();
    __instance->set_defaults();
  }

  return __instance;
}

recoConsts::recoConsts()
{}

void recoConsts::set_CharFlag(const std::string& name, const std::string& flag)
{
  std::string flag_expanded = ExpandEnvironmentals(flag);
  charflag[name] = flag_expanded;
}

std::string recoConsts::ExpandEnvironmentals(const std::string& input)
{
  wordexp_t exp_result;
  if(wordexp(input.c_str(), &exp_result, 0) != 0)
  {
    std::cout << "ExpandEnvironmentals - ERROR - Your string '" << input << "' cannot be understood!" << std::endl;
    return "";
  }
  const std::string output(exp_result.we_wordv[0]);
  return output;
}

void recoConsts::set_defaults()
{
  //Following constants are shared between simulation and reconstruction
  set_DoubleFlag("KMAGSTR", 1.0);
  set_DoubleFlag("FMAGSTR", 1.0);

  //Following flags control the running mode and must be 
  //set to appropriate values in the configuration set
  set_BoolFlag("KMAG_ON", true);
  set_BoolFlag("COARSE_MODE", false);
  set_BoolFlag("MC_MODE", false);
  set_BoolFlag("COSMIC_MODE", false);

  //Following values are fed to GeomSvc
  set_BoolFlag("OnlineAlignment", false);
  set_BoolFlag("IdealGeom", false);

  set_CharFlag("AlignmentMille", "$E1039_RESOURCE/alignment/run6/align_mille.txt");
  set_CharFlag("AlignmentHodo", "$E1039_RESOURCE/alignment/run6/alignment_hodo.txt");
  set_CharFlag("AlignmentProp", "$E1039_RESOURCE/alignment/run6/alignment_prop.txt");
  set_CharFlag("Calibration", "$E1039_RESOURCE/alignment/run6/calibration.txt");

  set_CharFlag("MySQLURL", "mysql://e906-db1.fnal.gov:3306");
  set_CharFlag("Geometry", "user_e1039_geom_plane.param_G9_run5_2");

  set_CharFlag("TRIGGER_Repo", "$TRIGGER_ROOT");
  set_CharFlag("TRIGGER_L1", "67");

  set_CharFlag("fMagFile", "$GEOMETRY_ROOT/magnetic_fields/Fmag.root");
  set_CharFlag("kMagFile", "$GEOMETRY_ROOT/magnetic_fields/Kmag.root");

  //Following flags adjust the performance/efficiency of the reconstruction,
  //thus should be included in the configuration set
  set_DoubleFlag("TX_MAX", 0.15);
  set_DoubleFlag("TY_MAX", 0.1);
  set_DoubleFlag("X0_MAX", 150.);
  set_DoubleFlag("Y0_MAX", 50.);
  set_DoubleFlag("INVP_MIN", 0.01);
  set_DoubleFlag("INVP_MAX", 0.2);
  set_DoubleFlag("PROB_LOOSE", 0.);
  set_DoubleFlag("PROB_TIGHT", 1.E-12);
  set_DoubleFlag("BAD_HIT_REJECTION", 3.);
  set_DoubleFlag("MERGE_THRESH", 0.015);
  set_DoubleFlag("RESOLUTION_FACTOR", 1.6);

  set_DoubleFlag("X_BEAM", 0.);
  set_DoubleFlag("Y_BEAM", 0.);
  set_DoubleFlag("SIGX_BEAM", 2.);
  set_DoubleFlag("SIGY_BEAM", 2.);

  set_CharFlag("EventReduceOpts", "aoc");

  set_BoolFlag("USE_V1495_HIT", true);
  set_BoolFlag("USE_TWTDC_HIT", false);

  set_IntFlag("NSTEPS_FMAG", 100);
  set_IntFlag("NSTEPS_SHIELDING", 50);
  set_IntFlag("NSTEPS_TARGET", 100);

  set_DoubleFlag("TDCTimeOffset", 0.);

  set_DoubleFlag("RejectWinDC0", 0.12);
  set_DoubleFlag("RejectWinDC1", 0.12);
  set_DoubleFlag("RejectWinDC2", 0.15);
  set_DoubleFlag("RejectWinDC3p", 0.16);
  set_DoubleFlag("RejectWinDC3m", 0.14);

  set_IntFlag("MaxHitsDC0", 100);
  set_IntFlag("MaxHitsDC1", 100);
  set_IntFlag("MaxHitsDC2", 100);
  set_IntFlag("MaxHitsDC3p", 100);
  set_IntFlag("MaxHitsDC3m", 100);

  //Following numbers are related to the geometric set up thus should not
  //change under most circumstances, unless one is studying the effects of these cuts
  //could be excluded from the configuration set
  set_DoubleFlag("SAGITTA_TARGET_CENTER", 1.85);
  set_DoubleFlag("SAGITTA_TARGET_WIDTH", 0.25);
  set_DoubleFlag("SAGITTA_DUMP_CENTER", 1.5);
  set_DoubleFlag("SAGITTA_DUMP_WIDTH", 0.3);

  set_IntFlag("MUID_MINHITS", 1);
  set_DoubleFlag("MUID_REJECTION", 4.);
  set_DoubleFlag("MUID_THE_P0", 0.11825);
  set_DoubleFlag("MUID_EMP_P0", 0.00643);
  set_DoubleFlag("MUID_EMP_P1", -0.00009);
  set_DoubleFlag("MUID_EMP_P2", 0.00000046);
  set_DoubleFlag("MUID_Z_REF", 2028.19);
  set_DoubleFlag("MUID_R_CUT", 3.0);

  set_DoubleFlag("DEDX_FE_P0", 7.18274);
  set_DoubleFlag("DEDX_FE_P1", 0.0361447);
  set_DoubleFlag("DEDX_FE_P2", -0.000718127);
  set_DoubleFlag("DEDX_FE_P3", 7.97312e-06);
  set_DoubleFlag("DEDX_FE_P4", -3.05481e-08);

  set_DoubleFlag("PT_KICK_KMAG", 0.4016);
  set_DoubleFlag("PT_KICK_FMAG", 2.909);

  set_DoubleFlag("Z_KMAG_BEND", 1064.26);
  set_DoubleFlag("Z_FMAG_BEND", 251.4);
  set_DoubleFlag("Z_KFMAG_BEND", 375.);
  set_DoubleFlag("ELOSS_KFMAG", 8.12);
  set_DoubleFlag("ELOSS_ABSORBER", 1.81);
  set_DoubleFlag("Z_ST2", 1347.36);
  set_DoubleFlag("Z_ABSORBER", 2028.19);
  set_DoubleFlag("Z_REF", 0.);
  set_DoubleFlag("Z_TARGET", -300.00);
  set_DoubleFlag("Z_DUMP", 42.);
  set_DoubleFlag("Z_ST1", 600.);
  set_DoubleFlag("Z_ST3", 1910.);
  set_DoubleFlag("FMAG_HOLE_LENGTH", 27.94);
  set_DoubleFlag("FMAG_HOLE_RADIUS", 1.27);
  set_DoubleFlag("FMAG_LENGTH", 502.92);
  set_DoubleFlag("Z_UPSTREAM", -500.);
  set_DoubleFlag("Z_DOWNSTREAM", 500.);
}

void recoConsts::init(int runNo, bool verbose)
{
  //TODO: initialization based on run range
  return;
}

void recoConsts::init(const std::string& setname, bool verbose)
{
  if(setname == "cosmic")
  {
    set_BoolFlag("KMAG_ON", false);
    set_BoolFlag("COSMIC_MODE", true);

    set_DoubleFlag("TX_MAX", 1.);
    set_DoubleFlag("TY_MAX", 1.);
    set_DoubleFlag("X0_MAX", 1000.);
    set_DoubleFlag("Y0_MAX", 1000.);
  }

  if(verbose) Print();
}

void recoConsts::initfile(const std::string& filename, bool verbose)
{
  ReadFromFile(filename, verbose);
  return;
}

void recoConsts::Print() const
{
  // methods from PHFlag
  PrintCharFlags();
  PrintFloatFlags();
  PrintDoubleFlags();
  PrintIntFlags();
  PrintBoolFlags();
}
