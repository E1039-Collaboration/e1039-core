export DIR_TOP=$(dirname $(readlink -f $BASH_SOURCE))

source /data2/e1039/this-e1039.sh
#source /data2/users/kenichi/e1039/online/core-inst/this-e1039.sh

export DIR_ONLINE=$E1039_ROOT/online
export STATUS_DB=$DIR_ONLINE/data_status.db
export STATUS_TBL=data_status

export DB_VAL_UNDEF=0
export DB_VAL_START=1
export DB_VAL_UPDATE=2
export DB_VAL_END=3

## Note:
## Not only the DB file but also its directory have to be writable.
