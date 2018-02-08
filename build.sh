#!/bin/bash

src=/home/yuhw/GitHub/SeaQuest/seaquest-offline
build=`pwd`
install=$OFFLINE_MAIN

declare -a packages=(
		framework/phool
		framework/ffaobjects
		framework/fun4all
		interface_main
		packages/global_consts
		packages/jobopts_svc
		packages/geom_svc
		module_example
		packages/db2g4
		packages/PHField
		packages/PHGeometry
		packages/PHGenFitPkg/GenFitExp
		packages/PHGenFitPkg/PHGenFit
		packages/ktracker
		)

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
