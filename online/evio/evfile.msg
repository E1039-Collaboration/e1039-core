!-----------------------------------------------------------------------------
!  Copyright (c) 1991,1992 Southeastern Universities Research Association,
!                          Continuous Electron Beam Accelerator Facility
! 
!  This software was developed under a United States Government license
!  described in the NOTICE file included as part of this distribution.
! 
!  CEBAF Data Acquisition Group, 12000 Jefferson Ave., Newport News, VA 23606
!  Email: coda@cebaf.gov  Tel: (804) 249-7101  Fax: (804) 249-7363
! -----------------------------------------------------------------------------
!  
!  Description:
! 	Error message definitions for the Slow Controls package.
! 	
!  Author:  Chip Watson, CEBAF Data Acquisition Group
! 
!  Revision History:
!    $Log: evfile.msg,v $
!    Revision 1.1  1998/12/07 22:11:05  saw
!    Initial setup
!
!    Revision 1.1  94/03/15  11:57:29  11:57:29  heyes (Graham Heyes)
!    Initial revision
!    
! Revision 1.1  1992/07/08  18:28:20  watson
! Initial revision
!
! 

.warn
TRUNC		1   	Event truncated on read
BADBLOCK    	2   	Bad block number encountered

.error
BADHANDLE	1	Bad handle (file/stream not open)
ALLOCFAIL	2	Failed to allocate event I/O structure
BADFILE	    	3	File format error
UNKOPTION	4	Unknown option specified
UNXPTDEOF	5	Unexpected end of file while reading event
BADSIZEREQ	6	Invalid buffer size request to evIoct
