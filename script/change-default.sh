#!/bin/bash -e
## A script to change "default".  It is supposed to be placed under 'share' or 'core'.
## Usage:
##   ./change-default.sh pr.138
DIR_BASE=$(dirname $(readlink -f $0))
NAME_DEF=default

cd $DIR_BASE
DEST_NEW=$(basename $1)
echo "DIR_BASE = $DIR_BASE"
echo "NAME_DEF = $NAME_DEF"
echo "DEST_NEW = $DEST_NEW"

if [ ! -e $DEST_NEW ] ; then
    echo "ERROR: Cannot find the new link destination '$DEST_NEW'.  Abort."
    exit
fi

DEST_ORG='n/a'
test -e $NAME_DEF && DEST_ORG=$(readlink $NAME_DEF)
echo "DEST_ORG = $DEST_ORG"

echo -n "Enter 'yes' to make the change: "
read YESNO
if [ "X$YESNO" != 'Xyes' ] ; then
    echo "Abort."
    exit
fi

ln -nfs $DEST_NEW $NAME_DEF
