AUTOMAKE_OPTIONS = foreign

INCLUDES = \
  -I$(includedir) \
  -I$(OFFLINE_MAIN)/include  \
  -I`root-config --incdir`

lib_LTLIBRARIES = \
   libmodule_example.la

AM_CXXFLAGS = \
 -Wall -Werror

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib

libmodule_example_la_LIBADD = \
  -lfun4all \
  -linterface_main \
  -lphool

pkginclude_HEADERS = \
  SimpleReadMySql.h \
  ReadMySql.h \
  TestAnalyzer.h

libmodule_example_la_SOURCES = \
  SimpleReadMySql.C \
  SimpleReadMySql_Dict.C \
  ReadMySql.C \
  ReadMySql_Dict.C \
  TestAnalyzer.C \
  TestAnalyzer_Dict.C

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
  libmodule_example.la

testexternals.C:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

clean-local:
	rm -f *Dict*
