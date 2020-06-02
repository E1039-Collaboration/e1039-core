#!/bin/bash

EXPECTED_ARGS=1
E_BADARGS=65

if [ $# -ne $EXPECTED_ARGS ]
then
  echo "Usage: `basename $0` <pluginname>"
  exit $E_BADARGS
fi


upper=`echo $1|tr '[:lower:]' '[:upper:]'`
echo "Renaming to $1"

echo "Updateing *.cxx"
sed -e "s/TestAnalyzer/$1/g" -e "s/TestAnalyzer/${upper}/g" <TestAnalyzer.cxx > $1.cxx

echo "Updateing *.h"
sed -e "s/TestAnalyzer/$1/g" -e "s/TestAnalyzer/${upper}/g" <TestAnalyzer.h > $1.h

echo "Updateing LinkDef.h"
sed -e "s/TestAnalyzer/$1/g" -e "s/TestAnalyzer/${upper}/g" <TestAnalyzerLinkDef.h >$1\LinkDef.h



echo "Cleaning up"
rm README TestAnalyzer.cxx TestAnalyzer.h  TestAnalyzerLinkDef.h
echo "All done." 
echo "Check linked libs in CMakeLists.txt!"
rm init.sh








