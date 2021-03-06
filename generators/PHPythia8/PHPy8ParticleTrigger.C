#include "PHPy8ParticleTrigger.h"

#include <Pythia8/Pythia.h>

using namespace std;

//__________________________________________________________
PHPy8ParticleTrigger::PHPy8ParticleTrigger(const std::string &name):
  PHPy8GenTrigger(name),
  _theEtaHigh(-999.9),
  _theEtaLow(-999.9),
  _thePtHigh(999.9),
  _thePtLow(-999.9),
  _thePHigh(999.9),
  _thePLow(-999.9),
  _thePxHigh(999.9),
  _thePxLow(-999.9),
  _thePyHigh(999.9),
  _thePyLow(-999.9),
  _thePzHigh(999.9),
  _thePzLow(-999.9),

  _doEtaHighCut(false),
  _doEtaLowCut(false),
  _doBothEtaCut(false),

  _doAbsEtaHighCut(false),
  _doAbsEtaLowCut(false),
  _doBothAbsEtaCut(false),

  _doPtHighCut(false),
  _doPtLowCut(false),
  _doBothPtCut(false),

  _doPHighCut(false),
  _doPLowCut(false),
  _doBothPCut(false),

  _doPxHighCut(false),
  _doPxLowCut(false),
  _doBothPxCut(false),
  _doPyHighCut(false),
  _doPyLowCut(false),
  _doBothPyCut(false),
  _doPzHighCut(false),
  _doPzLowCut(false),
  _doBothPzCut(false)
{}

PHPy8ParticleTrigger::~PHPy8ParticleTrigger() {
  if (_verbosity > 0) PrintConfig();
}

bool PHPy8ParticleTrigger::Apply(Pythia8::Pythia *pythia) {

  if (_verbosity > 2) {
    cout << "PHPy8ParticleTrigger::Apply - pythia event size: "
  	 << pythia->event.size() << endl;
  }
  
  // Loop over all particles in the event
  for (int i = 0; i < pythia->event.size(); ++i) {

    // loop over all the trigger particle criteria
    for (int j = 0; j < int(_theParticles.size()); j++) {
	
      bool passedParents = false;
      if (pythia->event[i].id() == _theParticles[j] &&
  	  pythia->event[i].status() > 0) { //only stable particles
	
  	if (_doBothEtaCut && (pythia->event[i].eta() < _theEtaLow ||
  			      pythia->event[i].eta() > _theEtaHigh)) continue;
  	if (_doEtaLowCut && pythia->event[i].eta() < _theEtaLow) continue;
  	if (_doEtaHighCut && pythia->event[i].eta() > _theEtaHigh) continue;

  	if (_doBothAbsEtaCut && (abs(pythia->event[i].eta()) < _theEtaLow ||
  				 abs(pythia->event[i].eta()) > _theEtaHigh)) continue;
  	if (_doAbsEtaLowCut && abs(pythia->event[i].eta()) < _theEtaLow) continue;
  	if (_doAbsEtaHighCut && abs(pythia->event[i].eta()) > _theEtaHigh) continue;

  	if (_doBothPtCut && (pythia->event[i].pT() < _thePtLow ||
  			     pythia->event[i].pT() > _thePtHigh)) continue;
  	if (_doPtHighCut && pythia->event[i].pT() > _thePtHigh ) continue;
  	if (_doPtLowCut && pythia->event[i].pT() < _thePtLow) continue;

  	if (_doBothPCut && (pythia->event[i].pAbs() < _thePLow ||
  			    pythia->event[i].pAbs() > _thePHigh)) continue;
  	if (_doPHighCut && pythia->event[i].pAbs() > _thePHigh ) continue;
  	if (_doPLowCut && pythia->event[i].pAbs() < _thePLow) continue;

  	if (_doBothPxCut && (pythia->event[i].px() < _thePxLow ||
  			     pythia->event[i].px() > _thePxHigh)) continue;
  	if (_doPxHighCut && pythia->event[i].px() > _thePxHigh ) continue;
  	if (_doPxLowCut && pythia->event[i].px() < _thePxLow) continue;

  	if (_doBothPyCut && (pythia->event[i].py() < _thePyLow ||
  			     pythia->event[i].py() > _thePyHigh)) continue;
  	if (_doPyHighCut && pythia->event[i].py() > _thePyHigh ) continue;
  	if (_doPyLowCut && pythia->event[i].py() < _thePyLow) continue;

  	if (_doBothPzCut && (pythia->event[i].pz() < _thePzLow ||
  			     pythia->event[i].pz() > _thePzHigh)) continue;
  	if (_doPzHighCut && pythia->event[i].pz() > _thePzHigh ) continue;
  	if (_doPzLowCut && pythia->event[i].pz() < _thePzLow) continue;

  	if (_verbosity > 5) {
  	  cout << "stable " << pythia->event[i].id()
  	       << "  pt: " << pythia->event[i].pT()
  	       << " pz: " << pythia->event[i].pz()
  	       << " p: " << pythia->event[i].pAbs()
  	       << " eta: " << pythia->event[i].eta() << endl; 
  	}
	
  	// loop over all partents to this particle
  	for (int k = 0; k < int(_theParents.size()); k++) {
  	  // check Mothers
  	  std::vector<int> moms = pythia->event[i].motherList();
  	  for (int m = 0; m < int(moms.size()) ; m++) {
  	    if (abs(pythia->event[ moms[m] ].id()) == abs(_theParents[k])) {
  	      passedParents = true;
  	      if (_verbosity > 5) cout << "found parent!" << endl;
  	      break;
  	    }
  	  }//moms for loop
  	  if (passedParents) break;
  	}//parents for loop

  	//If we made it here and it passes parents, success!
  	if (_theParents.size() == 0 || passedParents) return true; 

      }//if _theParticles
    }//_theParticles for loop
    
  }//pythia event for loop

  return false;
}

void PHPy8ParticleTrigger::AddParticles(std::string particles) {
  std::vector<int> addedParts = convertToInts(particles);
  _theParticles.insert(_theParticles.end(),addedParts.begin(),addedParts.end());
}

void PHPy8ParticleTrigger::AddParticles(int particle) {
  _theParticles.push_back(particle);
}

void PHPy8ParticleTrigger::AddParticles(std::vector<int> particles) {
  _theParticles.insert(_theParticles.end(),particles.begin(),particles.end());
}

void PHPy8ParticleTrigger::AddParents(std::string parents) {
  std::vector<int> addedParents = convertToInts(parents);
  _theParents.insert(_theParents.end(),addedParents.begin(),addedParents.end());
}

void PHPy8ParticleTrigger::AddParents(int parent) {
  _theParents.push_back(parent);
}

void PHPy8ParticleTrigger::AddParents(std::vector<int> parents) {
  _theParents.insert(_theParents.end(),parents.begin(),parents.end());
}

void PHPy8ParticleTrigger::SetPtHigh(double pt) {
  _thePtHigh = pt;
  if (_doPtLowCut) _doBothPtCut = true;
  else _doPtHighCut = true;
}

void PHPy8ParticleTrigger::SetPtLow(double pt) {
  _thePtLow = pt;
  if (_doPtHighCut) _doBothPtCut = true;
  else _doPtLowCut = true;
}

void PHPy8ParticleTrigger::SetPtHighLow(double ptHigh, double ptLow) {
  if (ptHigh < ptLow) {
    _thePtHigh = ptLow;
    _thePtLow = ptHigh;
  } else {
    _thePtHigh = ptHigh;
    _thePtLow = ptLow;
  }
  _doBothPtCut = true;
  _doPtLowCut = false;
  _doPtHighCut = false;
}

void PHPy8ParticleTrigger::SetPHigh(double p) {
  _thePHigh = p;
  if (_doPLowCut) {
    _doBothPCut = true;
    _doPLowCut = false;
  } else {
    _doPHighCut = true;
  }
}

void PHPy8ParticleTrigger::SetPLow(double p) {
  _thePLow = p;
  if (_doPHighCut) {
    _doBothPCut = true;
    _doPHighCut = false;
  } else {
    _doPLowCut = true;
  }
}

void PHPy8ParticleTrigger::SetPHighLow(double pHigh, double pLow) {
  if (pHigh < pLow) {
    _thePHigh = pLow;
    _thePLow= pHigh;
  } else {
    _thePHigh = pHigh;
    _thePLow = pLow;
  }
  _doBothPCut = true;
  _doPLowCut = false;
  _doPHighCut = false;
}

void PHPy8ParticleTrigger::SetEtaHigh(double eta) {
  _theEtaHigh = eta;
  if (_doEtaLowCut) {
    _doBothEtaCut = true;
    _doEtaLowCut = false;
  } else {
    _doEtaHighCut = true;
  }
}
  
void PHPy8ParticleTrigger::SetEtaLow(double eta) {
  _theEtaLow = eta;
  if (_doEtaHighCut) {
    _doBothEtaCut = true;
    _doEtaHighCut = false;
  } else {
    _doEtaLowCut = true;
  }
}
  
void PHPy8ParticleTrigger::SetEtaHighLow(double etaHigh, double etaLow) {
  _theEtaHigh = etaHigh;
  _theEtaLow = etaLow;
  _doBothEtaCut = true;
  _doEtaHighCut = false;
  _doEtaLowCut = false;
}

void PHPy8ParticleTrigger::SetAbsEtaHigh(double eta) {
  _theEtaHigh = eta;
  if (_doAbsEtaLowCut) {
    _doBothAbsEtaCut = true;
    _doAbsEtaLowCut = false;
  } else {
    _doAbsEtaHighCut = true;
  }
}

void PHPy8ParticleTrigger::SetAbsEtaLow(double eta) {
  _theEtaLow = eta;
  if (_doAbsEtaHighCut) {
    _doBothAbsEtaCut = true;
    _doAbsEtaHighCut = false;
  } else {
    _doAbsEtaLowCut = true;
  }
}

void PHPy8ParticleTrigger::SetAbsEtaHighLow(double etaHigh, double etaLow) {
  _theEtaHigh = etaHigh;
  _theEtaLow = etaLow;
  _doBothAbsEtaCut = true;
  _doAbsEtaLowCut = false;
  _doAbsEtaHighCut = false;
}

void PHPy8ParticleTrigger::SetPxHigh(double px) {
  _thePxHigh = px;
  if (_doPxLowCut) {
    _doBothPxCut = true;
    _doPxLowCut = false;
  } else {
    _doPxHighCut = true;
  }
}

void PHPy8ParticleTrigger::SetPxLow(double px) {
  _thePxLow = px;
  if (_doPxHighCut) {
    _doBothPxCut = true;
    _doPxHighCut = false;
  } else {
    _doPxLowCut = true;
  }
}

void PHPy8ParticleTrigger::SetPxHighLow(double pxHigh, double pxLow) {
  if (pxHigh < pxLow) {
    _thePxHigh = pxLow;
    _thePxLow= pxHigh;
  } else {
    _thePxHigh = pxHigh;
    _thePxLow = pxLow;
  }
  _doBothPxCut = true;
  _doPxLowCut = false;
  _doPxHighCut = false;
}

void PHPy8ParticleTrigger::SetPyHigh(double py) {
  _thePyHigh = py;
  if (_doPyLowCut) {
    _doBothPyCut = true;
    _doPyLowCut = false;
  } else {
    _doPyHighCut = true;
  }
}

void PHPy8ParticleTrigger::SetPyLow(double py) {
  _thePyLow = py;
  if (_doPyHighCut) {
    _doBothPyCut = true;
    _doPyHighCut = false;
  } else {
    _doPyLowCut = true;
  }
}

void PHPy8ParticleTrigger::SetPyHighLow(double pyHigh, double pyLow) {
  if (pyHigh < pyLow) {
    _thePyHigh = pyLow;
    _thePyLow= pyHigh;
  } else {
    _thePyHigh = pyHigh;
    _thePyLow = pyLow;
  }
  _doBothPyCut = true;
  _doPyLowCut = false;
  _doPyHighCut = false;
}

void PHPy8ParticleTrigger::SetPzHigh(double pz) {
  _thePzHigh = pz;
  if (_doPzLowCut) {
    _doBothPzCut = true;
    _doPzLowCut = false;
  } else {
    _doPzHighCut = true;
  }
}

void PHPy8ParticleTrigger::SetPzLow(double pz) {
  _thePzLow = pz;
  if (_doPzHighCut) {
    _doBothPzCut = true;
    _doPzHighCut = false;
  } else {
    _doPzLowCut = true;
  }
}

void PHPy8ParticleTrigger::SetPzHighLow(double pzHigh, double pzLow) {
  if (pzHigh < pzLow) {
    _thePzHigh = pzLow;
    _thePzLow= pzHigh;
  } else {
    _thePzHigh = pzHigh;
    _thePzLow = pzLow;
  }
  _doBothPzCut = true;
  _doPzLowCut = false;
  _doPzHighCut = false;
}

void PHPy8ParticleTrigger::PrintConfig() {
  cout << "---------------- PHPy8ParticleTrigger::PrintConfig --------------------" << endl;
  cout << "   Particles: ";
  for (int i = 0; i < int(_theParticles.size()); i++) cout << _theParticles[i] << "  ";
  cout << endl;

  cout << "   Parents: ";
  for (int i = 0; i < int(_theParents.size()); i++) cout << _theParents[i] << "  ";
  cout << endl;

  if (_doEtaHighCut||_doEtaLowCut||_doBothEtaCut)
    cout << "   doEtaCut:  " << _theEtaLow << " < eta < " << _theEtaHigh << endl; 
  if (_doAbsEtaHighCut||_doAbsEtaLowCut||_doBothAbsEtaCut)
    cout << "   doAbsEtaCut:  " << _theEtaLow << " < |eta| < " << _theEtaHigh << endl; 
  if (_doPtHighCut||_doPtLowCut||_doBothPtCut)
    cout << "   doPtCut:  " << _thePtLow << " < pT < " << _thePtHigh << endl; 
  if (_doPHighCut||_doPLowCut||_doBothPCut)
    cout << "   doPCut:  " << _thePLow << " < p < " << _thePHigh << endl;
  if (_doPxHighCut||_doPxLowCut||_doBothPxCut)
    cout << "   doPxCut:  " << _thePxLow << " < px < " << _thePxHigh << endl;
  if (_doPyHighCut||_doPyLowCut||_doBothPyCut)
    cout << "   doPyCut:  " << _thePyLow << " < py < " << _thePyHigh << endl;
  if (_doPzHighCut||_doPzLowCut||_doBothPzCut)
    cout << "   doPzCut:  " << _thePzLow << " < pz < " << _thePzHigh << endl; 
  cout << "-----------------------------------------------------------------------" << endl;
}
