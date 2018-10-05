/* <@(#) evfile_msg.h created on Mon May 23 12:00 by makeMsgTbl> */
/* <@(#) Return Codes for EVFILE 115 evfile.msg Event File I/O> */
#ifndef EVFILE_header
#ifndef S_SUCCESS
#define S_SUCCESS 0
#define S_FAILURE -1
#endif

#define S_EVFILE    	0x00730000	/* evfile.msg Event File I/O */
#define S_EVFILE_TRUNC	0x40730001	/* Event truncated on read */
#define S_EVFILE_BADBLOCK	0x40730002	/* Bad block number encountered */
#define S_EVFILE_BADHANDLE	0x80730001	/* Bad handle (file/stream not open) */
#define S_EVFILE_ALLOCFAIL	0x80730002	/* Failed to allocate event I/O structure */
#define S_EVFILE_BADFILE	0x80730003	/* File format error */
#define S_EVFILE_UNKOPTION	0x80730004	/* Unknown option specified */
#define S_EVFILE_UNXPTDEOF	0x80730005	/* Unexpected end of file while reading event */
#define S_EVFILE_BADSIZEREQ	0x80730006	/* Invalid buffer size request to evIoct */
#endif
