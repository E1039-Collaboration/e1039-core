#include <phool/phool.h>
#include "SQMCEvent_v1.h"
using namespace std;

SQMCEvent_v1::SQMCEvent_v1()
  : _proc_id(0)
  , _xsec(.0)
  , _weight(1.0)
{
  for (int ii = 0; ii < 4; ii++) {
    _par_id [ii] = 0;
    _par_mom[ii].SetXYZT(0,0,0,0);
  }
}

SQMCEvent_v1::~SQMCEvent_v1()
{
  ;
}

void SQMCEvent_v1::identify(std::ostream& os) const
{
  ;
}

void SQMCEvent_v1::Reset()
{
  ;
}

int SQMCEvent_v1::get_particle_id(const int i) const
{
  if (i < 0 || i >= _N_PAR) {
    cerr << PHWHERE << " Invalid index:" << i << endl;
    return 0;
  }
  return _par_id[i];
}

void SQMCEvent_v1::set_particle_id(const int i, const int a)
{
  if (i < 0 || i >= _N_PAR) {
    cerr << PHWHERE << " Invalid index:" << i << endl;
    return;
  }
  _par_id[i] = a;
}

TLorentzVector SQMCEvent_v1::get_particle_momentum(const int i) const
{
  if (i < 0 || i >= _N_PAR) {
    cerr << PHWHERE << " Invalid index:" << i << endl;
    return TLorentzVector(0,0,0,0);
  }
  return _par_mom[i];
}

void SQMCEvent_v1::set_particle_momentum(const int i, const TLorentzVector a)
{
  if (i < 0 || i >= _N_PAR) {
    cerr << PHWHERE << " Invalid index:" << i << endl;
    return;
  }
  _par_mom[i] = a;
}
