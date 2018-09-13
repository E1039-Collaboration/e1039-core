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
 *	Routines to extract message text from message table
 *	
 * Author:  Chip Watson, CEBAF Data Acquisition Group
 *
 * Revision History:
 *   $Log: ceMsgLib.c,v $
 *   Revision 1.1  1998/12/07 22:11:04  saw
 *   Initial setup
 *
*	  Revision 1.1  94/03/16  07:57:20  07:57:20  heyes (Graham Heyes)
*	  Initial revision
*	  
*	  Revision 1.1  94/03/15  11:56:40  11:56:40  heyes (Graham Heyes)
*	  Initial revision
*	  
 *	  Revision 1.2  93/07/21  09:07:35  09:07:35  heyes (Graham Heyes)
 *	  Get rid of anoying pointer without cast warning
 *	  
 *	  Revision 1.1  92/07/14  18:27:48  18:27:48  watson (Chip Watson)
 *	  Initial revision
 *	  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ceMsg.h"

char *ceMsg(int,int,char *,int);
void cePmsg(char *,int);

extern CE_TBL_ENTRY ceMsgTbl[];
static char *lastmsg = (char *)0;

#ifdef NOF77extname
void cemsg
#else
void cemsg_
#endif
(int *num,int *flag,char *out,int maxout)
{
  /* fortran: call cemsg(error,flag,character_variable) */
  (void) ceMsg(*num,*flag,out,maxout);
}

char *ceMsg(num,flag,out,maxout)
     int num, flag, maxout;
     char *out;
{
  int severity,len;
  CE_TBL_ENTRY *entry;
  char *msgptr,*cp;

  if (lastmsg) free(lastmsg);	/* free last constructed message */
  for(entry = ceMsgTbl;entry->name!=NULL;entry++)
    if (entry->num==num) break;	/* assume null terminated table for now */
  if (entry->num!=num) {
    if (out) *out = '\0';
    return(NULL);
  }
  msgptr = 0;
  severity = cemsg_severity(num);
  switch (flag) {
  case (CEMSG_ALL):
    len = strlen(entry->name) + strlen(entry->msg) + 16;
    lastmsg = (char *)malloc(len);
    if (lastmsg) {
      msgptr = lastmsg;
      switch (severity) {
      case (CEMSG_INFO):
	strcpy(msgptr,"Info: ");
	break;
      case (CEMSG_WARN):
	strcpy(msgptr,"Warning: ");
	break;
      case (CEMSG_ERROR):
	strcpy(msgptr,"Error: ");
	break;
      case (CEMSG_FATAL):
	strcpy(msgptr,"Fatal error: ");
	break;
      }
      strcat(msgptr,entry->name);
      strcat(msgptr,"  ");
      strcat(msgptr,entry->msg);
    }
    break;
  case (CEMSG_NAME):
    msgptr = entry->name;
    break;
  case (CEMSG_MSG):
    msgptr = entry->msg;
    break;
  case (CEMSG_SEV):
    switch (severity) {
    case (CEMSG_INFO):
      msgptr = "I";
      break;
    case (CEMSG_WARN):
      msgptr = "W";
      break;
    case (CEMSG_ERROR):
      msgptr = "E";
      break;
    case (CEMSG_FATAL):
      msgptr = "F";
      break;
    }
    break;
  case (CEMSG_FAC):
    lastmsg = (char *)malloc(strlen(entry->name)); /* bigger than needed */
    msgptr = lastmsg;
    strcpy((char *) msgptr,(char *) entry->name[2]); /* skip over S_ */
    for (cp=msgptr;*cp;cp++) 
      if (*cp=='_') break;
    *cp = '\0';			/* truncate at next _ */
    break;
  case (CEMSG_CODE):
    lastmsg = (char *)malloc(strlen(entry->name)); /* bigger than needed */
    msgptr = lastmsg;
    for (cp= &(entry->name[2]);*cp;cp++)
      if (*cp=='_') break;
    strcpy(msgptr,cp+1);	/* copy following S_XXX_ */
    for (cp=msgptr;*cp;cp++)
      if (*cp=='_') break;
    *cp='\0';			/* terminate at next _ */
    break;
  default:
    msgptr = (char *) 0;
  }
  if (out!=NULL) strncpy(out,msgptr,maxout);
  return(msgptr);
}   

void cepmsg_(char *string,int *num,int s_len)
{
  char *t;
  t = (char *)malloc(s_len+1);
  strncpy(t,string,s_len);
  t[s_len]='\0';
  (void) cePmsg(t,*num);
  free(t);
}

void cePmsg(string,num)
     char *string;
     int num;
{
  fprintf(stderr,"%s ",string);	/* append a space */
  fprintf(stderr,"%s\n",ceMsg(num,CEMSG_ALL,NULL,0));
}

