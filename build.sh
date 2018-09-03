#!/bin/bash

src=/e906/app/users/yuhw/seaquest-offline
build=`pwd`
install=$MY_INSTALL

if [ $# -eq 1 ]; then
  echo $1
  declare -a packages=(
    $1
  )
else
  declare -a packages=(
		framework/phool
		framework/ffaobjects
		framework/fun4all
		interface_main
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
		packages/global_consts
		packages/jobopts_svc
		packages/geom_svc
		packages/db2g4
		packages/PHGenFitPkg/GenFitExp
		packages/PHGenFitPkg/PHGenFit
		packages/ktracker
		module_example
	)
fi

for package in "${packages[@]}"
do
	echo $src/$package
	mkdir -p $build/$package
	cd $build/$package
	echo cmake -DCMAKE_INSTALL_PREFIX=$install $src/$package
	cmake -DCMAKE_INSTALL_PREFIX=$install $src/$package
	make -j4 install
done

cd $build
