#!/bin/bash
DIR_MACRO=$(dirname $(readlink -f $0))
source /data2/e1039/this-e1039.sh
cd $DIR_MACRO
root -l EventDisp4MainDaqDst.C
