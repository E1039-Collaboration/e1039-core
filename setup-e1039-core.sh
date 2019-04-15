DIR_E1039_CORE=$(dirname $(readlink -f $BASH_SOURCE))

if [ $HOSTNAME = 'seaquestdaq01.fnal.gov' ] ; then
    source /opt/e1039-share/this-share.sh
    export OFFLINE_MAIN=$DIR_E1039_SHARE/inst
    export MY_INSTALL=$(dirname $DIR_E1039_CORE)/e1039-core-build/inst # ~/tmp/e1039-core

    export            PATH=$MY_INSTALL/bin:$PATH
    export           CPATH=$MY_INSTALL/include:$CPATH
    export    LIBRARY_PATH=$MY_INSTALL/lib:$LIBRARY_PATH
    export LD_LIBRARY_PATH=$MY_INSTALL/lib:$LD_LIBRARY_PATH
else
    echo "Using the non-host-specific setting.  This might not work on your environment."
    export  CC=gcc-4.9
    export CXX=g++-4.9
    source /opt/root/root-5.34.36/bin/thisroot.sh
    source /opt/geant4/geant4.10.04.p02/build/geant4make.sh
    export OFFLINE_MAIN=~/tmp/e1039-core
    export MY_INSTALL=$OFFLINE_MAIN
    export LD_LIBRARY_PATH=$OFFLINE_MAIN/lib:$LD_LIBRARY_PATH
fi

function cmake-e1039-core {
    echo "DIR_E1039_CORE = $DIR_E1039_CORE"
    echo "MY_INSTALL     = $MY_INSTALL"

    cmake -DCMAKE_INSTALL_PREFIX=$MY_INSTALL $DIR_E1039_CORE

    #mkdir -p $build/$package
    #cd $build/$package
    #echo cmake -DCMAKE_INSTALL_PREFIX=$install $src/$package
    #cmake -DCMAKE_INSTALL_PREFIX=$install $src/$package
    #make -j4 install
    #ret=$?
    #if [ $ret -ne 0 ] ; then
    #	echo "Abort since ret = $ret."
    #	exit
    #fi
}
