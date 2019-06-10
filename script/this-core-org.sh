export E1039_CORE=$(dirname $(readlink -f $BASH_SOURCE))

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
    export ROOT_INCLUDE_PATH=$(find $E1039_CORE/include -type d -printf '%p:')$CPATH
fi
