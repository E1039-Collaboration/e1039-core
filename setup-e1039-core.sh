export E1039_CORE_SRC=$(dirname $(readlink -f $BASH_SOURCE))

if [ $HOSTNAME = 'seaquestdaq01.fnal.gov' -o \
    ${HOSTNAME:0:12} = 'spinquestana' ] ; then
    export E1039_RESOURCE=/data2/e1039/resource

    source /data2/e1039/share/this-share.sh
    export OFFLINE_MAIN=$E1039_SHARE
    if [ -z "$MY_INSTALL" ] ; then
	export MY_INSTALL=$(dirname $E1039_CORE_SRC)/e1039-core-build/inst
    fi

    export            PATH=$MY_INSTALL/bin:$PATH
    export           CPATH=$MY_INSTALL/include:$CPATH
    export    LIBRARY_PATH=$MY_INSTALL/lib:$LIBRARY_PATH
    export LD_LIBRARY_PATH=$MY_INSTALL/lib:$LD_LIBRARY_PATH

elif [ ${HOSTNAME:0:12} = 'seaquestgpvm' ] ; then
    export E1039_RESOURCE=/e906/app/software/osg/users/yuhw/e1039/resource

    source /e906/app/users/yuhw/setup.sh
    export OFFLINE_MAIN=$(dirname $E1039_CORE_SRC)/e1039-core-build/inst
    if [ -z "$MY_INSTALL" ] ; then
	export MY_INSTALL=$OFFLINE_MAIN
    fi

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
