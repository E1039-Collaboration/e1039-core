#!/bin/bash

src=/home/yuhw/GitHub/SeaQuest/seaquest-offline
build=`pwd`
install=$OFFLINE_MAIN

declare -a packages=(
		framework/phool
		framework/ffaobjects
		framework/fun4all
		interface_main
		module_example
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
