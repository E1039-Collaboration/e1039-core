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
 *	Event I/O test program
 *	
 * Author:  Chip Watson, CEBAF Data Acquisition Group
 *
 * Revision History:
 *   $Log: evtest.c,v $
 *   Revision 1.1  1998/12/07 22:11:05  saw
 *   Initial setup
 *
*	  Revision 1.1  94/03/15  11:57:27  11:57:27  heyes (Graham Heyes)
*	  Initial revision
*	  
 *	  Revision 1.2  1992/07/14  19:14:18  watson
 *	  Make test event more complex
 *
 *	  Revision 1.1  1992/07/08  18:28:48  watson
 *	  Initial revision
 *
 */

int *makeEvent();
#define MIN(a,b) (a<b)? a : b

main()
{
  int handle,status,nevents,nlong,handle2;
  int buffer[2048],i;
  int *ip;

  printf("Event I/O tests...\n");
  status = evOpen("single.dat","w",&handle);
  cePmsg("Opening single.dat",status);
  ip = makeEvent();
  status = evWrite(handle,ip);
  cePmsg("Writing single.dat",status);
  status = evClose(handle);
  cePmsg("Closing single.dat",status);
  status = evOpen("single.dat","r",&handle);
  nevents=0;
  while ((status=evRead(handle,buffer,16384))==0) {
    nevents++;
    printf("  nevent %d len %d\n",nevents,buffer[0]);
    if (nevents<=4) 
      nlong = buffer[0]+1;
      for(ip=buffer;nlong>0;nlong-=8) { 
	for (i=MIN(nlong,8);i>0;i--) printf("  %8x",*ip++);
	printf("\n");
      }
    if (nevents==1) {
      status = evOpen("multiple.dat","w",&handle2);
      evWrite(handle2,buffer);
      evWrite(handle2,buffer);
      evWrite(handle2,buffer);
      evClose(handle2);
    }
  }
  printf("last read status %x\n",status);
  evClose(handle);
}


int *makeEvent()
{
  int *bank;
  int *segment, *longword;
  short *word;
  short *packet;
  float data;

  bank = (int *) malloc(80);
  bank[0] = 22;			/* event length = 18 */
  bank[1] = 1<<16 | 0x20<<8;	/* bank 1 contains segments */
  {
    segment = &(bank[2]);
    segment[0] = 1<<24 | 0x20<<16 | 6; /* segment 1 has 2 segments of len 3 */
    {
      segment = &(segment[1]);
      segment[0] = 2<<24 |  1<<16 | 2; /* segment 2 has 2 longwords */
      segment[1] = 0x11111111;
      segment[2] = 0x22222222;
      segment += 3;
      segment[0] = 3<<24 |  4<<16 | 2; /* segment 3 has 2 longwords of shorts */
      {
	word = (short *) &(segment[1]);
	word[0] = 0x0000;
	word[1] = 0x1111;
	word[2] = 0x2222;
	word[3] = 0x3333;
      }
    }
    segment = &(bank[2]) + 7;	/* point past segment 1 */
    segment[0] = 4<<24 | 0x34<<16 | 3; /* seg 4 has I*2 packets */
    {
      packet = (short *) &(segment[1]);
      packet[0] = 1<<8 | 2;	/* packet 1 */
      packet[1] = 0x1111;
      packet[2] = 0x2222;
      packet += 3;
      packet[0] = 2<<8 | 2;	/* packet 2 */
      packet[1] = 0x1111;
      packet[2] = 0x2222;
    }
    segment += 4;
    segment[0] = 5<<24 | 0xF<<16 | 8; /* seg 5 contains repeating structures */
    {
      word = (short *) &(segment[1]);
      word[0] = 2;
      word[1] = 2<<8 | 2;	/* 2(a,b) */
      word[2] = 0x8000 | 2<<4 | 1; /* 2I */
      word[3] = 0x8000 | 1<<4 | 2; /* 1F */
      longword = &(segment[3]);
      data = 123.456;
      longword[0] = 0x1111;
      longword[1] = 0x2222;
      longword[2] = *(int *)&data;
      longword[3] = 0x11111111;
      longword[4] = 0x22222222;
      longword[5] = *(int *)&data;
    }
  }
  return(bank);
}
