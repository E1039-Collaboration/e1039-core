AUTOMAKE_OPTIONS = foreign

INCLUDES = \
  -I$(includedir) \
  -I$(OFFLINE_MAIN)/include  \
  -I`root-config --incdir`

lib_LTLIBRARIES = \
   libinterface_main.la

AM_CXXFLAGS = \
 -Wall -Werror

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib

libinterface_main_la_LIBADD = \
  -lphool

pkginclude_HEADERS = \
  SQHit.h \
  SQHit_v1.h \
  SQMCHit_v1.h \
  SQHitMap.h \
  SQHitMap_v1.h \
  SQHitVector.h \
  SQHitVector_v1.h \
  SQEvent.h \
  SQEvent_v1.h \
  SQRun.h \
  SQRun_v1.h \
  SQSpill.h \
  SQSpill_v1.h \
  SQSpillMap.h \
  SQSpillMap_v1.h

libinterface_main_la_SOURCES = \
  SQHit.C \
  SQHit_Dict.C \
  SQHit_v1.C \
  SQHit_v1_Dict.C \
  SQMCHit_v1.C \
  SQMCHit_v1_Dict.C \
  SQHitMap.C \
  SQHitMap_Dict.C \
  SQHitMap_v1.C \
  SQHitMap_v1_Dict.C \
  SQHitVector.C \
  SQHitVector_Dict.C \
  SQHitVector_v1.C \
  SQHitVector_v1_Dict.C \
  SQEvent.C \
  SQEvent_Dict.C \
  SQEvent_v1.C \
  SQEvent_v1_Dict.C \
  SQRun.C \
  SQRun_Dict.C \
  SQRun_v1.C \
  SQRun_v1_Dict.C \
  SQSpill.C \
  SQSpill_Dict.C \
  SQSpill_v1.C \
  SQSpill_v1_Dict.C \
  SQSpillMap.C \
  SQSpillMap_Dict.C \
  SQSpillMap_v1.C \
  SQSpillMap_v1_Dict.C

# Rule for generating table CINT dictionaries.
%_Dict.C: %.h %LinkDef.h
	rootcint -f $@ -c $(DEFAULT_INCLUDES) $(INCLUDES) $^

%_Dict.cpp: %.h %LinkDef.h
	rootcint -f $@ -c $(DEFAULT_INCLUDES) $(INCLUDES) $^



################################################
# linking tests

noinst_PROGRAMS = testexternals

BUILT_SOURCES = \
  testexternals.C

testexternals_LDADD = \
  libinterface_main.la

testexternals.C:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

clean-local:
	rm -f *Dict*
