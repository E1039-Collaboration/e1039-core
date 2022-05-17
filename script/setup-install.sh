## A script to set up the environment to build and install the e1039-core
## package.  Usage:
##   /path/to/setup-install.sh /path/to/your_inst_directory
##
## This script first creates an installation directory together with a 
## script "this-core.sh".  It then tries to find proper E1039_RESOURCE and 
## E1039_SHARE directories depending on the host name.  When successful, 
## it creates a script "this-e1039.sh", which sets up all the resource,
## share and core environments at once.
##
## One need not use this script but can set up everything by oneself.  But 
## this script should be helpful in preparing for an easily-reproducible 
## environment.  Any questions/requests can be sent to Kenichi.
if [ $0 != $BASH_SOURCE ] ; then
    echo "!!ERROR!!  The usage of this script has changed recently."
    echo "Now you have to execute (not source) it.  Sorry for the inconvenience."
    return
fi
DIR_SCRIPT=$(dirname $(readlink -f $0))

if [ -z "$1" ] ; then
    echo "The 1st argument must be an installation directory,"
    echo "or 'auto' to auto-select it."
    echo "Abort."
    exit 1
elif [ "X$1" = 'Xauto' ] ; then
    DIR_INST=$(readlink -f $DIR_SCRIPT/../../core-inst)
elif [ "X$1" = 'Xosg-user' ] ; then
    DIR_INST=/e906/app/software/osg/users/$USER/e1039/core
else
    DIR_INST=$(readlink -m "$1")
fi
echo "Use this installation directory:"
echo "  $DIR_INST"

##
## Create the setup script
##
mkdir -p $DIR_INST/script
\cp $DIR_SCRIPT/this-core-org.sh $DIR_INST/this-core.sh
\cp $DIR_SCRIPT/exec-decoder.sh $DIR_INST/script/

##
## Check and set up the parent environments.
##
if   [ ${HOSTNAME:0:11} = 'seaquestdaq' -o \
       ${HOSTNAME:0:9}  = 'e1039gat1' -o \
       ${HOSTNAME:0:10} = 'e1039prod1' -o \
       ${HOSTNAME:0:12} = 'spinquestana' -o \
       ${HOSTNAME:0:13} = 'e1039trackcpu' ] ; then
    echo "Use the environment for seaquestdaq/spinquestana."
    {
	echo 'export  E1039_ROOT=/data2/e1039'
	echo 'source $E1039_ROOT/resource/this-resource.sh'
	echo 'source $E1039_ROOT/share/this-share.sh'
	echo 'source $(dirname $(readlink -f $BASH_SOURCE))/this-core.sh'
    } >$DIR_INST/this-e1039.sh
    
elif [ ${HOSTNAME:0:12} = 'seaquestgpvm' -o \
       ${HOSTNAME:0:13} = 'spinquestgpvm' ] ; then
    echo "Use the environment for seaquestgpvm/spinquestgpvm."
    {
	echo 'export E1039_ROOT=/e906/app/software/osg/software/e1039'
	echo 'if [ ! -d $E1039_ROOT ] ; then '
	echo '    E1039_ROOT=/cvmfs/seaquest.opensciencegrid.org/seaquest/software/e1039'
	echo 'fi'
	echo 'source $E1039_ROOT/resource/this-resource.sh'
	echo 'source $E1039_ROOT/share/this-share.sh'
	echo 'source $(dirname $(readlink -f $BASH_SOURCE))/this-core.sh'
    } >$DIR_INST/this-e1039.sh
    
else
    echo "Your host is not supported by this script."
    echo "You can ask the manager (Kenichi) how to proceed, or"
    echo "try to set E1039_RESOURCE and E1039_SHARE properly by yourself."
    exit 1
fi

##
## Message
##
echo
echo "A setup script was created;"
echo "  $DIR_INST/this-e1039.sh"
echo "Next you likely source this script and execute 'build.sh';"
echo "  source $DIR_INST/this-e1039.sh"
echo "  ./build.sh"
echo
echo "Note that You should source the setup script when you use a new "
echo "shell environment (i.e. text terminal) to build or execute this "
echo "e1039-core package."
