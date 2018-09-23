/* <@(#) ceMsgTbl.c created on Mon May 23 12:00 by makeMsgTbl> */

typedef struct ce_tbl_entry {
  char *name;
  int num;
  char *msg;
} CE_TBL_ENTRY;

CE_TBL_ENTRY ceMsgTbl[] = {

  {"S_SUCCESS",		0,		"Operation successful"},
  {"S_FAILURE",		-1,		"Operation failed"},

  /* <@(#) EVFILE 115 evfile.msg Event File I/O> */

  {"S_EVFILE",		0x00730000,	"evfile.msg Event File I/O"},
  {"S_EVFILE_TRUNC",	0x40730001,	"Event truncated on read"},
  {"S_EVFILE_BADBLOCK",	0x40730002,	"Bad block number encountered"},
  {"S_EVFILE_BADHANDLE",	0x80730001,	"Bad handle (file/stream not open)"},
  {"S_EVFILE_ALLOCFAIL",	0x80730002,	"Failed to allocate event I/O structure"},
  {"S_EVFILE_BADFILE",	0x80730003,	"File format error"},
  {"S_EVFILE_UNKOPTION",	0x80730004,	"Unknown option specified"},
  {"S_EVFILE_UNXPTDEOF",	0x80730005,	"Unexpected end of file while reading event"},
  {"S_EVFILE_BADSIZEREQ",	0x80730006,	"Invalid buffer size request to evIoct"},
  {(char *)0,0,(char *)0}
};
