#!/bin/bash
# Wrapper script to upload a parameter set from TSV file to MySQL DB.
# Usage: 
#   https://github.com/E1039-Collaboration/e1039-wiki/wiki/channel-mapping
DIR_SCRIPT=$(dirname $(readlink -f $0))

CATEGORY=$1
PARAM_TYPE=$2
PARAM_ID=$3
MACRO=

case $CATEGORY in
    geom  ) MACRO=UploadGeomParam.C  ;;
    chan  ) MACRO=UploadChanMap.C    ;;
    calib ) MACRO=UploadCalibParam.C ;;
    * )
        echo "!!ERROR!!  Bad category.  Abort."
        exit
        ;;
esac

source /data2/e1039/this-e1039.sh
root.exe -b -q "$DIR_SCRIPT/$MACRO(\"$PARAM_TYPE\", \"$PARAM_ID\")"
