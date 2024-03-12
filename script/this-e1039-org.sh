# An all-in-one script to make use of "resource", "share" and "core" under
# a directory where this script is placed.  User can simply execute
#   source /path/to/this-e1039.sh
# or write it in ~/.bashrc.  
#
# User can change the "share" and/or "core" versions by setting 
# "E1039_SHARE_VERSION" and/or "E1039_CORE_VERSION" beforehand;
#   E1039_SHARE_VERSION=20240312
#   E1039_CORE_VERSION=daily
#   source /path/to/this-e1039.sh
export E1039_ROOT=$(dirname $(readlink -f $BASH_SOURCE))

if    [ -e $E1039_ROOT/resource/this-resource.sh ] ; then
    source $E1039_ROOT/resource/this-resource.sh
else
    echo "!!WARNING!!  Cannot find the resource directory: '$E1039_ROOT/resource'.  Your program might not work."
fi

test -z "$E1039_SHARE_VERSION" && E1039_SHARE_VERSION=default
if    [ -e $E1039_ROOT/share/$E1039_SHARE_VERSION/this-share.sh ] ; then
    source $E1039_ROOT/share/$E1039_SHARE_VERSION/this-share.sh
else
    echo "!!WARNING!!  Cannot find the share directory: '$E1039_ROOT/share/$E1039_SHARE_VERSION'.  Your program might not work."
fi

test -z "$E1039_CORE_VERSION" && E1039_CORE_VERSION=default
if    [ -e $E1039_ROOT/core/$E1039_CORE_VERSION/this-core.sh ] ; then
    source $E1039_ROOT/core/$E1039_CORE_VERSION/this-core.sh
else
    echo "!!WARNING!!  Cannot find the core directory: '$E1039_ROOT/core/$E1039_CORE_VERSION'.  Your program might not work."
fi
