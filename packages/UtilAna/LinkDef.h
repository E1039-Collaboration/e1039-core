#ifdef __CINT__
#pragma link off all class;
#pragma link off all function;
#pragma link off all global;

#pragma link C++ namespace UtilBeam;
#pragma link C++ namespace UtilDimuon;
#pragma link C++ namespace UtilHist;
#pragma link C++ namespace UtilHodo;
#pragma link C++ class     UtilOnline-!;
#pragma link C++ namespace UtilSQHit;
#pragma link C++ namespace UtilTrack;
#pragma link C++ namespace UtilTrigger;

// As of 2025-11-19:
// The functions in the namespaces above cannot be completed in CINT
// just after the library is loaded (i.e. `R__LOAD_LIBRARY(UtilAna)`).
// They become available in the CINT completion once the member functions
// of UtilOnline are completed (i.e. `UtilOnline::[Tab][Tab]`).

#endif
