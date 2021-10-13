# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(g4main CXX C)

include_directories( "${PROJECT_SOURCE_DIR}/")
if(${CMAKE_VERSION} VERSION_GREATER "2.8.1") 
  message("SET CMP0015 OLD")
  cmake_policy(SET CMP0015 OLD)
endif()

# source code
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

# ROOT
find_program(ROOTCONF "root-config")
if(ROOTCONF)
  message("-- Detecting ROOT:    found at ${ROOTCONF}")
else()
  message(FATAL_ERROR "-- Detecting ROOT:    not found")
endif()
execute_process(COMMAND root-config --prefix OUTPUT_VARIABLE ROOT_PREFIX  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --evelibs   OUTPUT_VARIABLE ROOT_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Geant4
  find_program(G4CONF "geant4-config")
  if(G4CONF)
    message("-- Detecting Geant4:  found at ${G4CONF}")
  else()
    message(FATAL_ERROR "-- Detecting Geant4:  not found")
  endif()
  #find_package(Geant4 REQUIRED gdml vis_all)
  execute_process(COMMAND geant4-config --prefix OUTPUT_VARIABLE GEANT4_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND geant4-config --cflags OUTPUT_VARIABLE GEANT4_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND geant4-config --libs   OUTPUT_VARIABLE GEANT4_LINK   OUTPUT_STRIP_TRAILING_WHITESPACE)


add_custom_command (
  OUTPUT PHG4_Dict.cc
  COMMAND rootcint
  ARGS -f PHG4_Dict.cc -noIncludePaths -inlineInputHeader -c -p
  -I${PROJECT_SOURCE_DIR}/
  -I$ENV{XERCESCROOT}/include/ -I${ROOT_PREFIX}/include/ -I${GEANT4_PREFIX}/include/ -I${GEANT4_PREFIX}/include/Geant4/
  ${PROJECT_SOURCE_DIR}/HepMCNodeReader.h
  ${PROJECT_SOURCE_DIR}/PHG4ConsistencyCheck.h
  ${PROJECT_SOURCE_DIR}/PHG4HeadReco.h
  ${PROJECT_SOURCE_DIR}/PHG4InEventCompress.h
  ${PROJECT_SOURCE_DIR}/PHG4InEventReadBack.h
  ${PROJECT_SOURCE_DIR}/PHG4InputFilter.h
  ${PROJECT_SOURCE_DIR}/PHG4PileupGenerator.h
  ${PROJECT_SOURCE_DIR}/PHG4SimpleEventGenerator.h
  ${PROJECT_SOURCE_DIR}/SQCosmicGen.h
  ${PROJECT_SOURCE_DIR}/PHG4PSScanGenerator.h
  ${PROJECT_SOURCE_DIR}/PHG4ParticleGun.h
  ${PROJECT_SOURCE_DIR}/PHG4ParticleGenerator.h
  ${PROJECT_SOURCE_DIR}/PHG4ParticleGeneratorBase.h
  ${PROJECT_SOURCE_DIR}/PHG4ParticleGeneratorVectorMeson.h
  ${PROJECT_SOURCE_DIR}/PHG4ParticleGeneratorD0.h
  ${PROJECT_SOURCE_DIR}/PHG4Reco.h
  ${PROJECT_SOURCE_DIR}/PHG4Subsystem.h
  ${PROJECT_SOURCE_DIR}/PHG4TruthSubsystem.h
  ${PROJECT_SOURCE_DIR}/PHG4Utils.h
  ${PROJECT_SOURCE_DIR}/PHG4LinkDef.h
)

add_custom_command (
  OUTPUT PHG4Hit_Dict.cc
  COMMAND rootcint
  ARGS -f PHG4Hit_Dict.cc -noIncludePaths -inlineInputHeader -c -p
  -I${PROJECT_SOURCE_DIR}/
  -I$ENV{XERCESCROOT}/include/ -I${ROOT_PREFIX}/include/ -I${GEANT4_PREFIX}/include/ -I${GEANT4_PREFIX}/include/Geant4/
  ${PROJECT_SOURCE_DIR}/PHG4EventHeader.h
  ${PROJECT_SOURCE_DIR}/PHG4EventHeaderv1.h
  ${PROJECT_SOURCE_DIR}/PHG4HitReadBack.h
  ${PROJECT_SOURCE_DIR}/PHG4Hit.h
  ${PROJECT_SOURCE_DIR}/PHG4HitDefs.h
  ${PROJECT_SOURCE_DIR}/PHG4Hitv1.h
  ${PROJECT_SOURCE_DIR}/PHG4HitEval.h
  ${PROJECT_SOURCE_DIR}/PHG4HitContainer.h
  ${PROJECT_SOURCE_DIR}/PHG4InEvent.h
  ${PROJECT_SOURCE_DIR}/PHG4Shower.h
  ${PROJECT_SOURCE_DIR}/PHG4Showerv1.h
  ${PROJECT_SOURCE_DIR}/PHG4Particle.h
  ${PROJECT_SOURCE_DIR}/PHG4Particlev1.h
  ${PROJECT_SOURCE_DIR}/PHG4Particlev2.h
  ${PROJECT_SOURCE_DIR}/PHG4TruthInfoContainer.h
  ${PROJECT_SOURCE_DIR}/PHG4VtxPoint.h
  ${PROJECT_SOURCE_DIR}/PHG4VtxPointv1.h
  ${PROJECT_SOURCE_DIR}/PHG4HitLinkDef.h
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DG4INTY_USE_XT -DG4VIS_USE_OPENGL -DG4UI_USE_TCSH -DG4UI_USE_XM -DG4VIS_USE_OPENGLX")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I$ENV{XERCESCROOT}/include -I${ROOT_PREFIX}/include/ -I${GEANT4_PREFIX}/include/ -I${GEANT4_PREFIX}/include/Geant4/")

add_library(phg4hit SHARED
    ${PROJECT_SOURCE_DIR}/PHG4EventHeader.cc
    ${PROJECT_SOURCE_DIR}/PHG4EventHeaderv1.cc
    ${PROJECT_SOURCE_DIR}/PHG4HitReadBack.cc
    ${PROJECT_SOURCE_DIR}/PHG4HitDefs.cc
    ${PROJECT_SOURCE_DIR}/PHG4Hit.cc
    ${PROJECT_SOURCE_DIR}/PHG4Hitv1.cc
    ${PROJECT_SOURCE_DIR}/PHG4HitEval.cc
    ${PROJECT_SOURCE_DIR}/PHG4HitContainer.cc
    ${PROJECT_SOURCE_DIR}/PHG4InEvent.cc
    ${PROJECT_SOURCE_DIR}/PHG4Particle.cc
    ${PROJECT_SOURCE_DIR}/PHG4Particlev1.cc
    ${PROJECT_SOURCE_DIR}/PHG4Particlev2.cc
    ${PROJECT_SOURCE_DIR}/PHG4Shower.cc
    ${PROJECT_SOURCE_DIR}/PHG4Showerv1.cc
    ${PROJECT_SOURCE_DIR}/PHG4TruthInfoContainer.cc
    ${PROJECT_SOURCE_DIR}/PHG4VtxPoint.cc
    ${PROJECT_SOURCE_DIR}/PHG4VtxPointv1.cc
    PHG4Hit_Dict.cc
)

add_library(g4testbench SHARED
    ${PROJECT_SOURCE_DIR}/G4TBMagneticFieldSetup.cc
    ${PROJECT_SOURCE_DIR}/G4TBFieldMessenger.cc
    ${PROJECT_SOURCE_DIR}/PHG4MagneticField.cc
    ${PROJECT_SOURCE_DIR}/HepMCNodeReader.cc
    ${PROJECT_SOURCE_DIR}/PHG4ConsistencyCheck.cc
    ${PROJECT_SOURCE_DIR}/PHG4EtaParameterization.cc
    ${PROJECT_SOURCE_DIR}/PHG4EtaPhiParameterization.cc
    ${PROJECT_SOURCE_DIR}/PHG4HeadReco.cc
    ${PROJECT_SOURCE_DIR}/PHG4InEventCompress.cc
    ${PROJECT_SOURCE_DIR}/PHG4InEventReadBack.cc
    ${PROJECT_SOURCE_DIR}/PHG4InputFilter.cc
    ${PROJECT_SOURCE_DIR}/PHG4ParameterisationTubsEta.cc
    ${PROJECT_SOURCE_DIR}/PHG4PileupGenerator.cc
    ${PROJECT_SOURCE_DIR}/PHG4SimpleEventGenerator.cc
    ${PROJECT_SOURCE_DIR}/SQCosmicGen.cc
    ${PROJECT_SOURCE_DIR}/PHG4PSScanGenerator.cc
    ${PROJECT_SOURCE_DIR}/PHG4ParticleGun.cc
    ${PROJECT_SOURCE_DIR}/PHG4ParticleGeneratorBase.cc
    ${PROJECT_SOURCE_DIR}/PHG4ParticleGenerator.cc
    ${PROJECT_SOURCE_DIR}/PHG4ParticleGeneratorVectorMeson.cc
    ${PROJECT_SOURCE_DIR}/PHG4ParticleGeneratorD0.cc
    ${PROJECT_SOURCE_DIR}/PHG4PhenixDetector.cc
    ${PROJECT_SOURCE_DIR}/PHG4PhenixEventAction.cc
    ${PROJECT_SOURCE_DIR}/PHG4PhenixSteppingAction.cc
    ${PROJECT_SOURCE_DIR}/PHG4PhenixTrackingAction.cc
    ${PROJECT_SOURCE_DIR}/PHG4PrimaryGeneratorAction.cc
    ${PROJECT_SOURCE_DIR}/PHG4Reco.cc
    ${PROJECT_SOURCE_DIR}/PHG4RegionInformation.cc
    ${PROJECT_SOURCE_DIR}/PHG4TrackUserInfoV1.cc
    ${PROJECT_SOURCE_DIR}/PHG4TruthEventAction.cc
    ${PROJECT_SOURCE_DIR}/PHG4TruthSteppingAction.cc
    ${PROJECT_SOURCE_DIR}/PHG4TruthSubsystem.cc
    ${PROJECT_SOURCE_DIR}/PHG4TruthTrackingAction.cc
    ${PROJECT_SOURCE_DIR}/PHG4SteppingAction.cc
    ${PROJECT_SOURCE_DIR}/PHG4UIsession.cc
    ${PROJECT_SOURCE_DIR}/PHG4Utils.cc
    PHG4_Dict.cc
)

target_link_libraries(phg4hit      -lphool -lSubsysReco -lfun4all ${GEANT4_LINK})
target_link_libraries(g4testbench  -lphfield -lgsl -lgslcblas -lvararray -lHepMC -lphhepmc_io -lxerces-c -lphgeom -lphg4gdml -L./ -lphg4hit -linterface_main -lSQPrimaryVtxGen ${GEANT4_LINK})

install(TARGETS phg4hit     					DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS g4testbench 					DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  HepMCNodeReader.h
  PHBBox.h
  PHG4ColorDefs.h
  PHG4Detector.h
  PHG4EventAction.h
  PHG4EventHeader.h
  PHG4HitDefs.h
  PHG4Hit.h
  PHG4Hitv1.h
  PHG4HitEval.h
  PHG4HitContainer.h
  PHG4InEvent.h
  PHG4Particle.h
  PHG4Particlev1.h
  PHG4Particlev2.h
  PHG4ParticleGenerator.h
  PHG4ParticleGeneratorBase.h
  PHG4ParticleGeneratorVectorMeson.h
  PHG4PhenixDetector.h
  PHG4PileupGenerator.h
  PHG4Reco.h
  PHG4RegionInformation.h
  PHG4SimpleEventGenerator.h
  SQCosmicGen.h
  PHG4ParticleGun.h
  PHG4PSScanGenerator.h
  PHG4Shower.h
  PHG4Showerv1.h
  PHG4SteppingAction.h
  PHG4Subsystem.h
  PHG4TrackingAction.h
  PHG4TrackUserInfoV1.h
  PHG4TruthInfoContainer.h
  PHG4TruthSubsystem.h
  PHG4Utils.h
  PHG4VtxPoint.h
  PHG4VtxPointv1.h
)

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

# Install the pcm files in case of ROOT 6.
execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm phg4hit g4testbench)
endif()
