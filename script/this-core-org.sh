
if [ $E1039_CORE ] ; then # Clean up the old components
    function DropEleFromPath {
	local -r NAME=$1
	local -r  ELE=$2
	local -r CONT=$(eval "echo :\$$NAME:" | sed -e 's/:/::/g' -e "s%:$ELE:%%g" -e 's/:\+/:/g' -e 's/^://' -e 's/:$//'  -e 's/\ /\\ /g')
	eval "$NAME=$CONT"
    }
    DropEleFromPath PATH            $E1039_CORE/bin
    DropEleFromPath CPATH           $E1039_CORE/include
    DropEleFromPath LIBRARY_PATH    $E1039_CORE/lib
    DropEleFromPath LD_LIBRARY_PATH $E1039_CORE/lib
    unset -f DropEleFromPath
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

if [ -d $E1039_CORE/macros ] ; then
    export ROOT_INCLUDE_PATH=$(find $E1039_CORE/macros -type d -printf '%p:')$ROOT_INCLUDE_PATH
fi
