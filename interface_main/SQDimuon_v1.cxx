#include "SQDimuon_v1.h"
#include <GlobalConsts.h>

using namespace std;

SQDimuon_v1::SQDimuon_v1()
  : _id(-1)
  , _rec_id(-1)
  , _pdg_id(0)
  , _track_id_pos(0)
  , _track_id_neg(0)
  , _pos(0,0,0)
  , _mom(0,0,0,0)
  , _mom_pos(0,0,0,0)
  , _mom_neg(0,0,0,0)
{
  ;
}

SQDimuon_v1::~SQDimuon_v1()
{
  ;
}

void SQDimuon_v1::identify(std::ostream& os) const
{
  ;
}

void SQDimuon_v1::Reset()
{
  ;
}

double SQDimuon_v1::get_x1() const
{
  TLorentzVector p_beam(0., 0., sqrt(E_BEAM*E_BEAM - M_P*M_P), E_BEAM);
  TLorentzVector p_target(0., 0., 0., M_P);

  TLorentzVector p_cms = p_beam + p_target;
  TLorentzVector p_sum = _mom_pos + _mom_neg;

  return (p_target*p_sum)/(p_target*p_cms);
}

double SQDimuon_v1::get_x2() const
{
  TLorentzVector p_beam(0., 0., sqrt(E_BEAM*E_BEAM - M_P*M_P), E_BEAM);
  TLorentzVector p_target(0., 0., 0., M_P);

  TLorentzVector p_cms = p_beam + p_target;
  TLorentzVector p_sum = _mom_pos + _mom_neg;

  return (p_beam*p_sum)/(p_beam*p_cms);
}

double SQDimuon_v1::get_xf() const
{
  TLorentzVector p_beam(0., 0., sqrt(E_BEAM*E_BEAM - M_P*M_P), E_BEAM);
  TLorentzVector p_target(0., 0., 0., M_P);

  TLorentzVector p_cms = p_beam + p_target;
  TLorentzVector p_sum = _mom_pos + _mom_neg;

  double mass = p_sum.M();
  double sqrts = p_cms.M();

  return 2.*p_sum.Pz()/sqrts/(1. - mass*mass/sqrts/sqrts);
}

