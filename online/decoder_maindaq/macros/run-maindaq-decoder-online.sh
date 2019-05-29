#!/bin/bash

DIR_CODA=/data2/analysis/kenichi/e1039/codadata

##
## Functions
##
function PrintHelp {
    echo "help"
}

function DecodeOneRun {
    local -r RUN=$1

}

function StartLoop {

}

function StartDaemon {
    local -r FN_LOG=
}

##
## Main
##
case $1 in
    run    ) DecodeOneRun $2 ;;
    loop   ) StartLoop ;;
    daemon ) StartDaemon ;;
    *      ) PrintHelp ;;	
esac

