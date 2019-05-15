#!/bin/bash

test -z "$OFFLINE_MAIN" && echo "Need set 'OFFLINE_MAIN'.  Abort." && exit
test -z "$MY_INSTALL"   && echo   "Need set 'MY_INSTALL'.  Abort." && exit

src=$(dirname $(readlink -f $0))
build=`pwd`
install=$MY_INSTALL

if [ $# -eq 1 ]; then
  echo $1
  declare -a packages=(
    $1
  )
else
  declare -a packages=(
    packages/global_consts
    packages/jobopts_svc
    packages/geom_svc
    packages/db_svc
    framework/phool
    framework/ffaobjects
    framework/fun4all
    interface_main
    online/chan_map
    online/onlmonserver
    online/decoder_maindaq
    packages/Half
    packages/vararray
    database/pdbcal/base
    database/PHParameter
    packages/PHGeometry
    packages/PHField
    generators/phhepmc
    generators/PHPythia8
    simulation/g4decayer
    simulation/g4gdml
    simulation/g4main
    simulation/g4detectors
    simulation/g4dst
    simulation/g4eval
    packages/dptrigger
    #packages/db2g4
    packages/PHGenFitPkg/GenFitExp
    packages/PHGenFitPkg/PHGenFit
    packages/ktracker
    packages/embedding
    module_example
	)
fi

for package in "${packages[@]}"
do
	echo "================================================================"
	echo $src/$package
	mkdir -p $build/$package
	cd $build/$package
	echo cmake -DCMAKE_INSTALL_PREFIX=$install $src/$package
	cmake -DCMAKE_INSTALL_PREFIX=$install $src/$package
	make -j4 install
	ret=$?
	if [ $ret -ne 0 ] ; then
	    echo "Abort since ret = $ret."
	    exit
	fi
done

cd $build
