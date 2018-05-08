AUTOMAKE_OPTIONS = foreign

# List of shared libraries to produce
lib_LTLIBRARIES = \
    libg4detectors_io.la \
    libg4detectors.la

AM_CPPFLAGS = \
    -I$(includedir) \
    -I$(OFFLINE_MAIN)/include \
    -I$(ROOTSYS)/include \
    -I$(G4_MAIN)/include \
    -I$(OPT_SPHENIX)/include

AM_CXXFLAGS = -std=c++11

AM_LDFLAGS = \
    -L$(libdir) \
    -L$(OFFLINE_MAIN)/lib

libg4detectors_io_la_LIBADD = \
  -L$(ROOTSYS)/lib \
  -lG4clhep \
  -lphool \
  -lboost_filesystem \
  -lboost_system \
  -lpdbcalBase \
  -lXMLIO \
  -lphparameter

libg4detectors_la_LIBADD = \
  libg4detectors_io.la \
  -lphg4gdml \
  -lphool  \
  -lSubsysReco \
  -lg4testbench 

pkginclude_HEADERS = \
  PHG4BlockCellGeom.h \
  PHG4BlockCellGeomContainer.h \
  PHG4BlockGeom.h \
  PHG4BlockGeomContainer.h \
  PHG4Cell.h \
  PHG4Cellv1.h \
  PHG4Cellv2.h \
  PHG4CellContainer.h \
  PHG4CellDefs.h \
  PHG4CylinderCell.h \
  PHG4CylinderCellv1.h \
  PHG4CylinderCell_MAPS.h \
  PHG4CylinderCellv2.h \
  PHG4CylinderCellv3.h \
  PHG4CylinderCellContainer.h \
  PHG4CylinderGeom.h \
  PHG4CylinderGeomv1.h \
  PHG4CylinderGeomv2.h \
  PHG4CylinderGeomv3.h \
  PHG4CylinderGeomv4.h \
  PHG4CylinderGeom_MAPS.h \
  PHG4CylinderGeom_Siladders.h \
  PHG4CylinderGeomContainer.h \
  PHG4CylinderCellDefs.h \
  PHG4CylinderCellGeom.h \
  PHG4CylinderCellGeomContainer.h \
  PHG4CylinderSubsystem.h \
  PHG4PolyconSubsystem.h \
  PHG4TargetCoilSubsystem.h \
  PHG4BNLTargetCoilSubsystem.h \
  PHG4CollimatorSubsystem.h \
  PHG4DetectorSubsystem.h \
  PHG4DetectorGroupSubsystem.h \
  PHG4ParameterContainerInterface.h \
  PHG4ParameterInterface.h \
  PHG4Parameters.h \
  PHG4ParametersContainer.h \
  PHG4ScintillatorSlat.h \
  PHG4ScintillatorSlatContainer.h \
  PHG4ScintillatorSlatDefs.h \
  PHG4StepStatusDecode.h

libg4detectors_io_la_SOURCES = \
  PHG4Parameters.cc \
  PHG4Parameters_Dict.cc \
  PHG4ParametersContainer.cc \
  PHG4ParametersContainer_Dict.cc \
  PHG4ParameterContainerInterface.cc \
  PHG4ParameterContainerInterface_Dict.cc \
  PHG4ParameterInterface.cc \
  PHG4ParameterInterface_Dict.cc \
  PHG4BlockCellGeom.cc \
  PHG4BlockCellGeom_Dict.cc \
  PHG4BlockCellGeomContainer.cc \
  PHG4BlockCellGeomContainer_Dict.cc \
  PHG4BlockGeom.cc \
  PHG4BlockGeom_Dict.cc \
  PHG4BlockGeomv1.cc \
  PHG4BlockGeomv1_Dict.cc \
  PHG4BlockGeomContainer.cc \
  PHG4BlockGeomContainer_Dict.cc \
  PHG4Cell.cc \
  PHG4Cell_Dict.cc \
  PHG4Cellv1.cc \
  PHG4Cellv1_Dict.cc \
  PHG4Cellv2.cc \
  PHG4Cellv2_Dict.cc \
  PHG4CellContainer.cc \
  PHG4CellContainer_Dict.cc \
  PHG4CellDefs.cc \
  PHG4CellDefs_Dict.cc \
  PHG4CylinderGeom.cc \
  PHG4CylinderGeom_Dict.cc \
  PHG4CylinderGeomv1.cc \
  PHG4CylinderGeomv1_Dict.cc \
  PHG4CylinderGeomv2.cc \
  PHG4CylinderGeomv2_Dict.cc \
  PHG4CylinderGeomv3.cc \
  PHG4CylinderGeomv3_Dict.cc \
  PHG4CylinderGeomv4.cc \
  PHG4CylinderGeomv4_Dict.cc \
  PHG4CylinderGeom_Siladders.cc \
  PHG4CylinderGeom_Siladders_Dict.cc \
  PHG4CylinderGeom_MAPS.cc \
  PHG4CylinderGeom_MAPS_Dict.cc \
  PHG4CylinderGeomContainer.cc \
  PHG4CylinderGeomContainer_Dict.cc \
  PHG4CylinderCell_Dict.cc \
  PHG4CylinderCellv1.cc \
  PHG4CylinderCellv1_Dict.cc \
  PHG4CylinderCell_MAPS.cc \
  PHG4CylinderCell_MAPS_Dict.cc \
  PHG4CylinderCellv2.cc \
  PHG4CylinderCellv2_Dict.cc \
  PHG4CylinderCellv3.cc \
  PHG4CylinderCellv3_Dict.cc \
  PHG4CylinderCellContainer.cc \
  PHG4CylinderCellContainer_Dict.cc \
  PHG4CylinderCellGeom.cc \
  PHG4CylinderCellGeom_Dict.cc \
  PHG4CylinderCellGeomContainer.cc \
  PHG4CylinderCellGeomContainer_Dict.cc \
  PHG4ScintillatorSlat_Dict.cc \
  PHG4ScintillatorSlatDefs.cc \
  PHG4ScintillatorSlatDefs_Dict.cc \
  PHG4ScintillatorSlatv1.cc \
  PHG4ScintillatorSlatv1_Dict.cc \
  PHG4ScintillatorSlatContainer.cc \
  PHG4ScintillatorSlatContainer_Dict.cc

libg4detectors_la_SOURCES = \
  PHG4BeamlineMagnetDetector.cc \
  PHG4BeamlineMagnetSubsystem.cc \
  PHG4BeamlineMagnetSubsystem_Dict.cc \
  PHG4BlockCellReco.cc \
  PHG4BlockCellReco_Dict.cc \
  PHG4BlockDetector.cc \
  PHG4BlockSteppingAction.cc \
  PHG4BlockSubsystem.cc \
  PHG4BlockSubsystem_Dict.cc \
  PHG4CEmcTestBeamDetector.cc \
  PHG4CEmcTestBeamSteppingAction.cc \
  PHG4CEmcTestBeamSubsystem.cc \
  PHG4CEmcTestBeamSubsystem_Dict.cc \
  PHG4ConeDetector.cc \
  PHG4ConeRegionSteppingAction.cc \
  PHG4ConeSteppingAction.cc \
  PHG4ConeSubsystem.cc \
  PHG4ConeSubsystem_Dict.cc \
  PHG4CrystalCalorimeterDetector.cc \
  PHG4CrystalCalorimeterSteppingAction.cc \
  PHG4CrystalCalorimeterSubsystem.cc \
  PHG4CrystalCalorimeterSubsystem_Dict.cc \
  PHG4CylinderDetector.cc \
  PHG4CylinderSubsystem.cc \
  PHG4CylinderSubsystem_Dict.cc \
  PHG4CylinderCellReco.cc \
  PHG4CylinderCellReco_Dict.cc \
  PHG4CylinderSteppingAction.cc \
  PHG4PolyconDetector.cc \
  PHG4PolyconSubsystem.cc \
  PHG4PolyconSubsystem_Dict.cc \
  PHG4PolyconSteppingAction.cc \
  PHG4TargetCoilDetector.cc \
  PHG4TargetCoilSubsystem.cc \
  PHG4TargetCoilSubsystem_Dict.cc \
  PHG4TargetCoilSteppingAction.cc \
  PHG4BNLTargetCoilDetector.cc \
  PHG4BNLTargetCoilSubsystem.cc \
  PHG4BNLTargetCoilSubsystem_Dict.cc \
  PHG4BNLTargetCoilSteppingAction.cc \
  PHG4CollimatorDetector.cc \
  PHG4CollimatorSubsystem.cc \
  PHG4CollimatorSubsystem_Dict.cc \
  PHG4CollimatorSteppingAction.cc \
  PHG4DetectorGroupSubsystem.cc \
  PHG4DetectorGroupSubsystem_Dict.cc \
  PHG4DetectorSubsystem.cc \
  PHG4DetectorSubsystem_Dict.cc \
  PHG4EnvelopeDetector.cc \
  PHG4EnvelopeSteppingAction.cc \
  PHG4EnvelopeSubsystem.cc \
  PHG4EnvelopeSubsystem_Dict.cc \
  PHG4EventActionClearZeroEdep.cc \
  PHG4FCalDetector.cc \
  PHG4FCalSteppingAction.cc \
  PHG4FCalSubsystem.cc \
  PHG4FCalSubsystem_Dict.cc \
  PHG4ForwardEcalDetector.cc \
  PHG4EICForwardEcalDetector.cc \
  PHG4ForwardEcalSteppingAction.cc \
  PHG4ForwardEcalSubsystem.cc \
  PHG4ForwardEcalSubsystem_Dict.cc \
  PHG4ForwardCalCellReco.cc \
  PHG4ForwardCalCellReco_Dict.cc \
  PHG4FPbScDetector.cc \
  PHG4FPbScSteppingAction.cc \
  PHG4FPbScRegionSteppingAction.cc \
  PHG4FPbScSubsystem.cc \
  PHG4FPbScSubsystem_Dict.cc \
  PHG4GenHit.cc \
  PHG4GenHit_Dict.cc \
  PHG4ProjCrystalCalorimeterDetector.cc \
  PHG4SectorConstructor.cc \
  PHG4SectorConstructor_Dict.cc \
  PHG4SectorDetector.cc \
  PHG4SectorSteppingAction.cc \
  PHG4SectorSubsystem.cc \
  PHG4SectorSubsystem_Dict.cc \
  PHG4StepStatusDecode.cc \
  PHG4TrackStatusDecode.cc

# Rule for generating table CINT dictionaries.
%_Dict.cc: %.h %LinkDef.h
	rootcint -f $@ @CINTDEFS@ -c $(DEFAULT_INCLUDES) $(AM_CPPFLAGS) $^

################################################
# linking tests

BUILT_SOURCES = testexternals.cc

noinst_PROGRAMS = \
  testexternals_g4detectors \
  testexternals_g4detectors_io

testexternals_g4detectors_SOURCES = testexternals.cc
testexternals_g4detectors_LDADD = libg4detectors.la

testexternals_g4detectors_io_SOURCES = testexternals.cc
testexternals_g4detectors_io_LDADD = libg4detectors_io.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

##############################################
# please add new classes in alphabetical order

clean-local:
	rm -f *Dict* $(BUILT_SOURCES)
