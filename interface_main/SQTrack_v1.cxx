#include "SQTrack_v1.h"
using namespace std;

SQTrack_v1::SQTrack_v1() 
  : _id(0)
  , _charge(0)
  , _n_hits(0)
  , _pos_vtx(0,0,0)
  , _pos_st1(0,0,0)
  , _pos_st3(0,0,0)
  , _mom_vtx(0,0,0,0)
  , _mom_st1(0,0,0,0)
  , _mom_st3(0,0,0,0)
{
  ;
}

SQTrack_v1::~SQTrack_v1()
{
  ;
}

void SQTrack_v1::identify(std::ostream& os) const
{
  ;
}

void SQTrack_v1::Reset()
{
  ;
}
