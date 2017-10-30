AUTOMAKE_OPTIONS = foreign

BUILT_SOURCES = \
  testexternals.cc

AM_CPPFLAGS = \
  -I$(includedir) \
  -I$(OFFLINE_MAIN)/include \
  -I`root-config --incdir`

pkginclude_HEADERS = \
  Fun4AllBase.h \
  Fun4AllDstInputManager.h \
  Fun4AllDstOutputManager.h \
  Fun4AllDummyInputManager.h \
  Fun4AllHistoBinDefs.h \
  Fun4AllHistoManager.h \
  Fun4AllInputManager.h \
  Fun4AllNoSyncDstInputManager.h \
  Fun4AllOutputManager.h \
  Fun4AllReturnCodes.h \
  Fun4AllServer.h \
  Fun4AllSyncManager.h \
  Fun4AllUtils.h \
  PHTFileServer.h \
  SubsysReco.h \
  TDirectoryHelper.h

noinst_HEADERS = \
  Fun4AllLinkDef.h \
  SubsysRecoLinkDef.h

lib_LTLIBRARIES = \
  libSubsysReco.la \
  libTDirectoryHelper.la \
  libfun4all.la

libTDirectoryHelper_la_SOURCES = \
  TDirectoryHelper.cc

libfun4all_la_SOURCES = \
  Fun4AllDstInputManager.cc \
  Fun4AllDstOutputManager.cc \
  Fun4AllDummyInputManager.cc \
  Fun4AllHistoManager.cc \
  Fun4AllInputManager.cc \
  Fun4AllSyncManager.cc \
  Fun4AllNoSyncDstInputManager.cc \
  Fun4AllOutputManager.cc \
  Fun4AllServer.cc \
  Fun4AllUtils.cc \
  PHTFileServer.cxx

nodist_libfun4all_la_SOURCES = Fun4All_Dict.cc

libfun4all_la_LIBADD = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib \
  libSubsysReco.la \
  libTDirectoryHelper.la \
  -lboost_filesystem \
  -lffaobjects \
  -lphool

libSubsysReco_la_SOURCES = \
  Fun4AllBase.cc \
  SubsysReco.cc
  
libTDirectoryHelper_la_LDFLAGS = \
  `root-config --libs`

nodist_libSubsysReco_la_SOURCES = SubsysReco_Dict.cc

noinst_PROGRAMS = \
  testexternals

testexternals_SOURCES = testexternals.cc
testexternals_LDADD   = libfun4all.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

Fun4All_Dict.cc: \
  Fun4AllDstInputManager.h \
  Fun4AllDstOutputManager.h \
  Fun4AllDummyInputManager.h \
  Fun4AllHistoManager.h \
  Fun4AllInputManager.h \
  Fun4AllSyncManager.h \
  Fun4AllNoSyncDstInputManager.h \
  Fun4AllOutputManager.h \
  Fun4AllServer.h \
  Fun4AllLinkDef.h
	rootcint -f $@ @CINTDEFS@ -c $(DEFAULT_INCLUDES) $(AM_CPPFLAGS) $^

SubsysReco_Dict.cc: \
  Fun4AllBase.h \
  SubsysReco.h \
  SubsysRecoLinkDef.h
	rootcint -f $@ @CINTDEFS@ -c $(DEFAULT_INCLUDES) $(AM_CPPFLAGS) $^

clean-local:
	rm -f *Dict*
