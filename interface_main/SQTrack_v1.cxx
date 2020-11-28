#include "SQTrack_v1.h"
using namespace std;

SQTrack_v1::SQTrack_v1() 
  : _id(0)
  , _rec_id(-1)
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
  os << "---SQTrack_v1--------------------" << endl;
  os << "track_id: " << get_track_id() << endl;
  os << "rec_track_id: " << get_rec_track_id() << endl;
  os << "num_hits: " << get_num_hits() << endl;
  os << "vtx_pos: (" << _pos_vtx.X() << ", " << _pos_vtx.Y() << ", " << _pos_vtx.Z() << ")" << endl;
  os << "vtx_mom: (" << _mom_vtx.Px() << ", " << _mom_vtx.Py() << ", " << _mom_vtx.Pz() << ")" << endl;
  os << "st1_pos: (" << _pos_st1.X() << ", " << _pos_st1.Y() << ", " << _pos_st1.Z() << ")" << endl;
  os << "st1_mom: (" << _mom_st1.Px() << ", " << _mom_st1.Py() << ", " << _mom_st1.Pz() << ")" << endl;
  os << "st3_pos: (" << _pos_st3.X() << ", " << _pos_st3.Y() << ", " << _pos_st3.Z() << ")" << endl;
  os << "st3_mom: (" << _mom_st3.Px() << ", " << _mom_st3.Py() << ", " << _mom_st3.Pz() << ")" << endl;

  return;
}

void SQTrack_v1::Reset()
{
  ;
}
