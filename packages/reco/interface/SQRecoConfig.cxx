#include "SQRecoConfig.h"

#include <iostream>
#include <wordexp.h>

SQRecoConfig* SQRecoConfig::__instance = nullptr;
SQRecoConfig* SQRecoConfig::instance()
{
  if(__instance == nullptr)
  {
    __instance = new SQRecoConfig();
    __instance->setDefaults();
  }

  return __instance;
}

void SQRecoConfig::set_CharFlag(const std::string& name, const std::string& flag)
{
  std::string flag_expanded = ExpandEnvironmentals(flag);
  charflag[name] = flag_expanded;
}

void SQRecoConfig::setDefaults()
{
  //Following flags control the running mode and must be 
  //set to appropriate values in the configuration set
  set_BoolFlag("KMAG_ON", true);
  set_BoolFlag("COARSE_MODE", false);
  set_BoolFlag("ALIGNMENT_MODE", false);
  set_BoolFlag("MC_MODE", false);

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

  set_DoubleFlag("KMAGSTR", 1.0);
  set_DoubleFlag("FMAGSTR", 1.0);

  set_DoubleFlag("X_BEAM", 0.);
  set_DoubleFlag("Y_BEAM", 0.);
  set_DoubleFlag("SIGX_BEAM", 0.5);
  set_DoubleFlag("SIGY_BEAM", 0.5);

  set_CharFlag("EventReduceOpts", "aoc");

  set_BoolFlag("USE_V1495_HIT", true);
  set_BoolFlag("USE_TWTDC_HIT", false);

  set_IntFlag("NSTEPS_FMAG", 100);
  set_IntFlag("NSTEPS_SHIELDING", 50);
  set_IntFlag("NSTEPS_TARGET", 100);

  set_DoubleFlag("TDCTimeOffset", 0.);

  //Following numbers are related to the geometric set up thus should not
  //change under most circumstances, unless one is studying the effects of these cuts
  //could be excluded from the configuration set
  set_DoubleFlag("SAGITTA_TARGET_CENTER", 1.85);
  set_DoubleFlag("SAGITTA_TARGET_WIDTH", 0.25);
  set_DoubleFlag("SAGITTA_DUMP_CENTER", 1.5);
  set_DoubleFlag("SAGITTA_DUMP_WIDTH", 0.3);

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
}

void SQRecoConfig::init(int runNo)
{
  //TODO: initialization based on run range
  return;
}

std::string SQRecoConfig::ExpandEnvironmentals(const std::string& input)
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

void SQRecoConfig::Print() const 
{
  PrintCharFlags();
  PrintFloatFlags();
  PrintDoubleFlags();
  PrintIntFlags();
  PrintBoolFlags();

  return;
}