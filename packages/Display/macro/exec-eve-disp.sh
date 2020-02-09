#!/bin/bash
DIR_MACRO=$(dirname $(readlink -f $0))

ONL_MODE=true
test "X$1" = 'Xoffline' && ONL_MODE=false

DIR_CORE=$DIR_MACRO
while [ "$DIR_CORE" != '/' ] ; do
    test -e $DIR_CORE/this-e1039.sh && break
    DIR_CORE=$(dirname $DIR_CORE)
done
if [ "$DIR_CORE" = '/' ] ; then
    echo "Cannot find 'this-e1039.sh'.  Abort."
    exit
fi
echo "Use $DIR_CORE/this-e1039.sh."
source $DIR_CORE/this-e1039.sh

cd $DIR_MACRO
root -l "EventDisp4MainDaqDst.C($ONL_MODE)"
