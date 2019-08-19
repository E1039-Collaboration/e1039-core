DIR_SCRIPT=$(dirname $(readlink -f $BASH_SOURCE))

if [ -z "$1" ] ; then
    echo "The 1st argument must be an installation directory,"
    echo "or 'auto' to auto-select it."
    echo "Abort."
    return
elif [ "X$1" = 'Xauto' ] ; then
    DIR_INST=$(readlink -f $DIR_SCRIPT/../../e1039-core-inst)
else
    DIR_INST=$(readlink -m "$1")
fi
echo "Use this installation directory:"
echo "  $DIR_INST"
echo

##
## Check and set up the parent environments.
##
if [ "$E1039_RESOURCE" -a "$E1039_SHARE" ] ; then
    echo "Use the pre-defined environment:"
elif [ ${HOSTNAME:0:11} = 'seaquestdaq' -o \
       ${HOSTNAME:0:12} = 'spinquestana' ] ; then
    echo "Use the environment for seaquestdaq/spinquestana."
    export E1039_RESOURCE=/data2/e1039/resource
    source /data2/e1039/share/this-share.sh
elif [ ${HOSTNAME:0:12} = 'seaquestgpvm' -o \
       ${HOSTNAME:0:13} = 'spinquestgpvm'] ; then
    echo "Use the environment for seaquestgpvm/spinquestgpvm:"
    export E1039_RESOURCE=/e906/app/software/osg/users/yuhw/e1039/resource
    source /e906/app/users/yuhw/setup.sh
else
    echo "Your host is not supported by this script."
    echo "You need manually set E1039_RESOURCE and E1039_SHARE to proceed."
    echo "Abort."
    return
fi
echo "  E1039_RESOURCE = $E1039_RESOURCE"
echo "  E1039_SHARE    = $E1039_SHARE"
echo

##
## Set up the setup script
##
mkdir -p $DIR_INST
\cp $DIR_SCRIPT/this-core-org.sh $DIR_INST/this-core.sh
source $DIR_INST/this-core.sh
#export OFFLINE_MAIN=$DIR_INST
echo "The setup script was installed and sourced:"
echo "  $DIR_INST/this-core.sh"
echo 

echo "Next you move to a build directory and execute 'build.sh'."
