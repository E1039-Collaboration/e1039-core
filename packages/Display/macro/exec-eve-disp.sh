#!/bin/bash
DIR_MACRO=$(dirname $(readlink -f $0))

AUTO_MODE=true
test "X$1" = 'Xmanual' && AUTO_MODE=false

#source /data2/e1039/this-e1039.sh online
source /data2/e1039/this-e1039.sh online-new
cd $DIR_MACRO
root -l "EventDisp4MainDaqDst.C($AUTO_MODE)"
