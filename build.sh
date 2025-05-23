#!/bin/bash
# Script to build all (or a part of) packages in e1039-core.
# Usage:
#   ./build.sh        ... Build all packages.
#   ./build.sh pkg    ... Build a single package "pkg".
#   ./build.sh -s pkg ... Build a single package "pkg".
#   ./build.sh -r pkg ... Resume building all packages from "pkg".
#   ./build.sh -i pkg ... Incrementally build "pkg" without clean up previous build.
#   ./build.sh [-c cmake_args] Optionally pass addtional cmake args to the build
#
# The 2nd usage is not recommended but kept available for backward compatibility for now.
#
# The "-b" option is useful in choosing "/dev/shm/$USER/core-build" for example to speed up the build process.
test -z "$OFFLINE_MAIN" && echo "Need set 'OFFLINE_MAIN'.  Abort." && exit
test -z "$MY_INSTALL"   && echo   "Need set 'MY_INSTALL'.  Abort." && exit

src=$(dirname $(readlink -f $0))
build=$src/../core-build
install=$MY_INSTALL

mode=all
OPTIND=1
cmake_args=""
while getopts ":s:r:i:c:b:" OPT ; do
    case $OPT in
        s ) mode='single'
            package=$OPTARG
            echo "Single mode: package = $package"
            ;;
        r ) mode='resume'
            package=$OPTARG
            echo "Resuming mode: package = $package"
            ;;
        i ) mode='increment'
            package=$OPTARG
            echo "Incremental mode: package = $package"
            ;;
        c ) cmake_args=$OPTARG
            echo " - pass additional args $cmake_args to cmake"
            ;;
        b ) build=$(readlink -m $OPTARG)
            echo "Build directory = $build"
            ;;
        * ) echo 'Unsupported option.  Abort.'
            exit
            ;;
    esac
done
shift $((OPTIND - 1))

if [ $# -eq 1 ]; then # backward compatilibity
    mode='single'
    package="$1"
fi

if [ $mode = 'single' ] || [ $mode = 'increment' ]; then
    declare -a packages=( $package )
else # 'all' or 'resume'
  declare -a packages=(
    packages/global_consts
    framework/phool
    packages/db_svc
    packages/geom_svc
    framework/ffaobjects
    framework/fun4all
    interface_main
    packages/UtilAna
    online/decoder_maindaq
    packages/Half
    packages/vararray
    database/pdbcal/base
    database/PHParameter
    packages/PHGeometry
    packages/PHField 
    generators/E906LegacyVtxGen
    generators/phhepmc
    generators/PHPythia8
    simulation/g4gdml
    simulation/g4main
    simulation/g4detectors
    simulation/g4eval
    generators/E906LegacyGen
    generators/SingleMuonGen
    packages/calibrator
    packages/evt_filter
    packages/dptrigger
    #packages/db2g4
    packages/reco/interface
    packages/reco/SQGenFit
    packages/reco/kfitter
    packages/reco/ktracker
    packages/embedding
    packages/rs_Reader
    packages/RUS
    simulation/g4dst
    _macro_
	)
  if [ $mode = 'resume' ] ; then
      NN=${#packages[@]}
      for (( II = 0 ; II < $NN ; II++ )) ; do
	  test ${packages[$II]} = $package && break
	  unset packages[$II]
      done
  fi
fi

## Clean up $build when $mode = 'all'.  Otherwise the file deletion via
## 'install_manifest.txt' causes a problem when you changed 
## "$install" (via "setup-install.sh") but didn't clean up "$install".
if [ $mode = 'all' -a -e $build ] ; then
    echo "Clean up the build directory."
    rm -rf $build
fi

for package in "${packages[@]}" ; do
  echo "================================================================"
  if [ $package = '_macro_' ] ; then
      echo "Install all macros to $install/macros/."
      ( 
	  cd $src
	  find . -type d -regex '.*/macros*' | while read DIR_SRC ; do
	      echo "  $DIR_SRC"
	      DIR_DEST=$install/macros/$(dirname $DIR_SRC)
	      test $DIR_SRC = './macros' && DIR_DEST=$DIR_DEST/top
	      mkdir -p $DIR_DEST
	      cp -p $DIR_SRC/* $DIR_DEST
	  done
      )
      continue
  fi

  echo "Package: $package"
  if [ -f $build/$package/install_manifest.txt ]; then
    cat $build/$package/install_manifest.txt | xargs rm -f
  fi
  if [ $mode = 'increment' ] && [ -d $build/$package ]; then
    echo "Previous build exists, will build incrementally."
  else
    if [ -d $build/$package ]; then
      echo "Previous build exists, will clean up."
      rm -rf $build/$package
    fi

    mkdir -p $build/$package
    cd $build/$package
    echo cmake -DCMAKE_INSTALL_PREFIX=$install $src/$package $cmake_args
    cmake -DCMAKE_INSTALL_PREFIX=$install $src/$package $cmake_args
  fi
  
  cd $build/$package
  make -j4 install

  ret=$?
  if [ $ret -ne 0 ] ; then
    echo "Abort since ret = $ret."
    exit
  fi
done

cd $build
