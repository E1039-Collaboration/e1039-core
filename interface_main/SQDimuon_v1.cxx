#include "SQDimuon_v1.h"
using namespace std;

SQDimuon_v1::SQDimuon_v1()
  : _id(-1)
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

