/*-----------------------------------------------------------------------------
 * Copyright (c) 1991,1992 Southeastern Universities Research Association,
 *                         Continuous Electron Beam Accelerator Facility
 *
 * This software was developed under a United States Government license
 * described in the NOTICE file included as part of this distribution.
 *
 * CEBAF Data Acquisition Group, 12000 Jefferson Ave., Newport News, VA 23606
 * Email: coda@cebaf.gov  Tel: (804) 249-7101  Fax: (804) 249-7363
 *-----------------------------------------------------------------------------
 * 
 * Description:
 *	Include file to define error message field values, structure
 *	
 * Author:  Chip Watson
 *
 * Revision History:
 *   $Log: ceMsg.h,v $
 *   Revision 1.1  1998/12/07 22:11:04  saw
 *   Initial setup
 *
*	  Revision 1.1  94/03/16  07:57:22  07:57:22  heyes (Graham Heyes)
*	  Initial revision
*	  
*	  Revision 1.1  94/03/15  11:56:42  11:56:42  heyes (Graham Heyes)
*	  Initial revision
*	  
 *	  Revision 1.1  92/07/14  18:27:34  18:27:34  watson (Chip Watson)
 *	  Initial revision
 *	  
 */

#define CEMSG_ALL 0
#define CEMSG_NAME 1
#define CEMSG_MSG 2
#define CEMSG_SEV 3
#define CEMSG_FAC 4
#define CEMSG_CODE 5

#define CEMSG_INFO 0
#define CEMSG_WARN 1
#define CEMSG_ERROR 2
#define CEMSG_FATAL 3

#define cemsg_severity(code) (((code)>>30)&3)

typedef struct coda_error_table_entry {
  char *name;
  int  num;
  char *msg;
} CE_TBL_ENTRY;


