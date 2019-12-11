
if [ $E1039_CORE ] ; then # Clean up the old components
    PATH=${PATH//"$E1039_CORE/bin:"}
    CPATH=${CPATH//"$E1039_CORE/include:"}
    LIBRARY_PATH=${LIBRARY_PATH//"$E1039_CORE/lib:"}
    LD_LIBRARY_PATH=${LD_LIBRARY_PATH//"$E1039_CORE/lib:"}
fi

export E1039_CORE=$(dirname $(readlink -f $BASH_SOURCE))
export OFFLINE_MAIN=$E1039_CORE
export   MY_INSTALL=$E1039_CORE

if [ -z "$E1039_RESOURCE" ] ; then
    echo "E1039_RESOURCE is not defined.  Probably this program won't work."
fi
if [ -z "$E1039_SHARE" ] ; then
    echo "E1039_SHARE is not defined.  Probably this program won't work."
fi

export            PATH=$E1039_CORE/bin:$PATH
export           CPATH=$E1039_CORE/include:$CPATH
export    LIBRARY_PATH=$E1039_CORE/lib:$LIBRARY_PATH
export LD_LIBRARY_PATH=$E1039_CORE/lib:$LD_LIBRARY_PATH

if [ -d $E1039_CORE/include ] ; then
    export ROOT_INCLUDE_PATH=$(find $E1039_CORE/include -type d -printf '%p:')$ROOT_INCLUDE_PATH_E1039_SHARE
fi
