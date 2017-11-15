AUTOMAKE_OPTIONS = foreign

INCLUDES = \
  -I$(includedir) \
  -I$(OFFLINE_MAIN)/include  \
  -I`root-config --incdir`

lib_LTLIBRARIES = \
   libinterfaces.la

AM_CXXFLAGS = \
 -Wall -Werror

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib

libinterfaces_la_LIBADD = \
  -lphool

pkginclude_HEADERS = \
  SQHit.h \
  SQHit_v1.h \
  SQHitMap.h \
  SQHitMap_v1.h \
  SQEvent.h \
  SQEvent_v1.h

libinterfaces_la_SOURCES = \
  SQHit.C \
  SQHit_Dict.C \
  SQHit_v1.C \
  SQHit_v1_Dict.C \
  SQHitMap.C \
  SQHitMap_Dict.C \
  SQHitMap_v1.C \
  SQHitMap_v1_Dict.C \
  SQEvent.C \
  SQEvent_Dict.C \
  SQEvent_v1.C \
  SQEvent_v1_Dict.C

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
  libinterfaces.la

testexternals.C:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

clean-local:
	rm -f *Dict*
