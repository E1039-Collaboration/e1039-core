#!/bin/bash
## A script to copy the files that are not handled by cmake for now.
## This is a quite temporary solution.

if [ -z "$MY_INSTALL" ] ; then
    echo "!!ERROR!!  MY_INSTALL not set.  Abort."
    exit
fi
echo "MY_INSTALL = $MY_INSTALL"

if [ ! -e interface_main/libinterface_main.so ] ; then
    echo "!!ERROR!!  Cannot find libinterface_main.so.  Probably you are not running this scipt in your build directory.  Abort."
    exit
fi

if [ "X$1" = 'Xclean' ] ; then
    echo "Deleting '*_rdict.pcm' in MY_INSTALL/lib."
    #find $MY_INSTALL/lib -name '*_rdict.pcm'
    rm $MY_INSTALL/lib/*_rdict.pcm
else
    echo "Copying '*_rdict.pcm' to MY_INSTALL/lib."
    #find . -name '*_rdict.pcm' -exec cp -p {} $MY_INSTALL/lib \;
    find . -name '*_rdict.pcm' | while read FN_SRC ; do
	FN_BASE=$(basename $FN_SRC)
	FN_DST=$MY_INSTALL/lib/$FN_BASE
	if [ ! -e $FN_DST -o $FN_SRC -nt $FN_DST ] ; then
	    echo "  $FN_BASE"
	    cp -p $FN_SRC $MY_INSTALL/lib
	fi
    done
fi


