/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/stat.c,v 1.7 2000-10-22 14:11:27 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.2a.

  Low level system statistics source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2000 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "procmeter.h"

#define CPU         0
#define CPU_USER    1
#define CPU_NICE    2
#define CPU_SYS     3
#define CPU_IDLE    4
#define DISK        5
#define DISK_READ   6
#define DISK_WRITE  7
#define SWAP        8
#define SWAP_IN     9
#define SWAP_OUT   10
#define PAGE       11
#define PAGE_IN    12
#define PAGE_OUT   13
#define CONTEXT    14
#define INTR       15
#define N_OUTPUTS  16

/*+ The length of the buffer for reading in lines. +*/
#define BUFFLEN 2048

/* The interface information.  */

/*+ The statistics +*/
ProcMeterOutput _outputs[N_OUTPUTS]=
{
 /*+ The total cpu output +*/
 {
  /* char  name[16];         */ "CPU",
  /* char *description;      */ "The total fraction of the time that the CPU is busy.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 %",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 20,
  /* char  graph_units[8];   */ "(%d%%)"
 },
 /*+ The user cpu output +*/
 {
  /* char  name[16];         */ "CPU_User",
  /* char *description;      */ "The fraction of the time that the CPU is processing user level code (applications).",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 %",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 20,
  /* char  graph_units[8];   */ "(%d%%)"
 },
 /*+ The nice cpu output +*/
 {
  /* char  name[16];         */ "CPU_Nice",
  /* char *description;      */ "The fraction of the time that the CPU is running processes that run at a lowered priority.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 %",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 20,
  /* char  graph_units[8];   */ "(%d%%)"
 },
 /*+ The system cpu output +*/
 {
  /* char  name[16];         */ "CPU_System",
  /* char *description;      */ "The fraction of the time that the CPU is processing system level code (kernel).",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 %",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 20,
  /* char  graph_units[8];   */ "(%d%%)"
 },
 /*+ The idle cpu output +*/
 {
  /* char  name[16];         */ "CPU_Idle",
  /* char *description;      */ "The fraction of the time that the CPU is idle.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 %",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 20,
  /* char  graph_units[8];   */ "(%d%%)"
 },
 /*+ The disk blocks accessed per second +*/
 {
  /* char  name[16];         */ "Disk",
  /* char *description;      */ "The total number of disk blocks accessed per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks read per second +*/
 {
  /* char  name[16];         */ "Disk_Read",
  /* char *description;      */ "The total number of disk blocks that are read per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks write per second +*/
 {
  /* char  name[16];         */ "Disk_Write",
  /* char *description;      */ "The total number of disk blocks that are written per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The swap blocks accessed per second +*/
 {
  /* char  name[16];         */ "Swap",
  /* char *description;      */ "The total number of swap space blocks accessed per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The swap blocks in per second +*/
 {
  /* char  name[16];         */ "Swap_In",
  /* char *description;      */ "The number of swap space blocks that are read in per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The swap blocks out per second +*/
 {
  /* char  name[16];         */ "Swap_Out",
  /* char *description;      */ "The number of swap space blocks that are written out per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The page blocks accessed per second +*/
 {
  /* char  name[16];         */ "Page",
  /* char *description;      */ "The number of paged blocks accessed per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The page blocks in per second +*/
 {
  /* char  name[16];         */ "Page_In",
  /* char *description;      */ "The number of paged blocks that are read in per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The page blocks out per second +*/
 {
  /* char  name[16];         */ "Page_Out",
  /* char *description;      */ "The number of paged blocks that are dumped out per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The number of context switches per second +*/
 {
  /* char  name[16];         */ "Context",
  /* char *description;      */ "The number of context switches between running processes per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 100,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The number of interrupts per second +*/
 {
  /* char  name[16];         */ "Interrupts",
  /* char *description;      */ "The number of hardware interrupts per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 100,
  /* char  graph_units[8];   */ "(%d/s)"
 }
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[N_OUTPUTS+1];

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];           */ "Statistics",
 /* char *description;       */ "Low level system statistics. [From /proc/stat]",
};


static int available[N_OUTPUTS];
static unsigned long *current,*previous,values[2][N_OUTPUTS];

static int kernel_version_240=0;


/*++++++++++++++++++++++++++++++++++++++
  Load the module.

  ProcMeterModule *Load Returns the module information.
  ++++++++++++++++++++++++++++++++++++++*/

ProcMeterModule *Load(void)
{
 return(&module);
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise the module, creating the outputs as required.

  ProcMeterOutput **Initialise Returns a NULL terminated list of outputs.

  char *options The options string for the module from the .procmeterrc file.
  ++++++++++++++++++++++++++++++++++++++*/

ProcMeterOutput **Initialise(char *options)
{
 FILE *f;
 char line[BUFFLEN+1];
 int n;

 outputs[0]=NULL;
 for(n=0;n<N_OUTPUTS;n++)
    available[n]=0;
 n=0;

 current=values[0];
 previous=values[1];

 /* Verify the statistics from /proc/stat */

 f=fopen("/proc/stat","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/stat'.\n",__FILE__);
 else
   {
    if(!fgets(line,BUFFLEN,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/stat'.\n",__FILE__);
    else
      {
       int i;

       if(sscanf(line,"cpu %lu %lu %lu %lu",&current[CPU_USER],&current[CPU_NICE],&current[CPU_SYS],&current[CPU_IDLE])==4)
          available[CPU]=available[CPU_USER]=available[CPU_NICE]=available[CPU_SYS]=available[CPU_IDLE]=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'cpu' line in '/proc/stat'.\n",__FILE__);

       fgets(line,BUFFLEN,f);
       while(line[0]=='c')      /* kernel version > ~2.1.84 */
          fgets(line,BUFFLEN,f);

       if(!strncmp(line,"disk ",5)) /* kernel version < ~2.4.0-test4 */
         {
          unsigned long d0,d1,d2,d3;

          if(sscanf(line,"disk %lu %lu %lu %lu",&d0,&d1,&d2,&d3)==4)
            {available[DISK]=1;current[DISK]=d0+d1+d2+d3;}
          else
             fprintf(stderr,"ProcMeter(%s): Unexpected 'disk' line in '/proc/stat'.\n",__FILE__);

          fgets(line,BUFFLEN,f);
          while(line[0]=='d')      /* kernel version > ~1.3.0 */
            {
             if(sscanf(line,"disk_rblk %lu %lu %lu %lu",&d0,&d1,&d2,&d3)==4)
               {available[DISK_READ]=1;current[DISK_READ]=d0+d1+d2+d3;}
             if(sscanf(line,"disk_wblk %lu %lu %lu %lu",&d0,&d1,&d2,&d3)==4)
               {available[DISK_WRITE]=1;current[DISK_WRITE]=d0+d1+d2+d3;}
             fgets(line,BUFFLEN,f);
            }
         }

       if(sscanf(line,"page %lu %lu",&current[PAGE_IN],&current[PAGE_OUT])==2)
          available[PAGE]=available[PAGE_IN]=available[PAGE_OUT]=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'page' line in '/proc/stat'.\n",__FILE__);

       fgets(line,BUFFLEN,f);
       if(sscanf(line,"swap %lu %lu",&current[SWAP_IN],&current[SWAP_OUT])==2)
          available[SWAP]=available[SWAP_IN]=available[SWAP_OUT]=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'swap' line in '/proc/stat'.\n",__FILE__);

       fgets(line,BUFFLEN,f);
       if(sscanf(line,"intr %lu",&current[INTR])==1)
          available[INTR]=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'intr' line in '/proc/stat'.\n",__FILE__);

       fgets(line,BUFFLEN,f);
       if(!strncmp(line,"disk_io: ",9)) /* kernel version > ~2.4.0-test4 */
         {
          int maj,min,num=8,nm;
          unsigned long d0,d1,d2,d3;

          kernel_version_240=1;

          current[DISK]=0;
          current[DISK_READ]=0;
          current[DISK_WRITE]=0;

          while(sscanf(line+num," (%d,%d):(%lu,%lu,%lu,%lu)%n",&maj,&min,&d0,&d1,&d2,&d3,&nm)==6)
            {
             available[DISK]=1;      current[DISK]      +=d1+d3;
             available[DISK_READ]=1; current[DISK_READ] +=d1;
             available[DISK_WRITE]=1;current[DISK_WRITE]+=d3;

             num+=nm;
            }

          fgets(line,BUFFLEN,f);
         }

       if(sscanf(line,"ctxt %lu",&current[CONTEXT])==1)
          available[CONTEXT]=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'ctxt' line in '/proc/stat'.\n",__FILE__);

       if(available[CPU])
          current[CPU]=current[CPU_USER]+current[CPU_NICE]+current[CPU_SYS];

       if(available[PAGE])
          current[PAGE]=current[PAGE_IN]+current[PAGE_OUT];

       if(available[SWAP])
          current[SWAP]=current[SWAP_IN]+current[SWAP_OUT];

       for(i=0;i<N_OUTPUTS;i++)
          if(available[i]) outputs[n++]=&_outputs[i];
      }

    fclose(f);
   }

 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform an update on one of the statistics.

  int Update Returns 0 if OK, else -1.

  time_t now The current time.

  ProcMeterOutput *output The output that the value is wanted for.
  ++++++++++++++++++++++++++++++++++++++*/

int Update(time_t now,ProcMeterOutput *output)
{
 static time_t last=0;
 int i;

 /* Get the statistics from /proc/stat */

 if(now!=last)
   {
    FILE *f;
    char line[BUFFLEN+1];
    long *temp;

    temp=current;
    current=previous;
    previous=temp;

    f=fopen("/proc/stat","r");
    if(!f)
       return(-1);

    fgets(line,BUFFLEN,f);
    if(available[CPU])
       sscanf(line,"cpu %lu %lu %lu %lu",&current[CPU_USER],&current[CPU_NICE],&current[CPU_SYS],&current[CPU_IDLE]);
    fgets(line,BUFFLEN,f);
    while(line[0]=='c')      /* kernel version > ~2.1.84 */
       fgets(line,BUFFLEN,f);
    if(!kernel_version_240) /* kernel version < ~2.4.0-test4 */
      {
       unsigned long d0,d1,d2,d3;
       if(available[DISK])
         {sscanf(line,"disk %lu %lu %lu %lu",&d0,&d1,&d2,&d3);current[DISK]=d0+d1+d2+d3;}
       fgets(line,BUFFLEN,f);
       while(line[0]=='d')      /* kernel version > ~1.3.x */
         {
          if(available[DISK_READ])
             if(sscanf(line,"disk_rblk %lu %lu %lu %lu",&d0,&d1,&d2,&d3)==4)
               {current[DISK_READ]=d0+d1+d2+d3;}
          if(available[DISK_WRITE])
             if(sscanf(line,"disk_wblk %lu %lu %lu %lu",&d0,&d1,&d2,&d3)==4)
               {current[DISK_WRITE]=d0+d1+d2+d3;}
          fgets(line,BUFFLEN,f);
         }
      }
    if(available[PAGE])
       sscanf(line,"page %lu %lu",&current[PAGE_IN],&current[PAGE_OUT]);
    fgets(line,BUFFLEN,f);
    if(available[SWAP])
       sscanf(line,"swap %lu %lu",&current[SWAP_IN],&current[SWAP_OUT]);
    fgets(line,BUFFLEN,f);
    if(available[INTR])
       sscanf(line,"intr %lu",&current[INTR]);
    fgets(line,BUFFLEN,f);
    if(kernel_version_240) /* kernel version > ~2.4.0-test4 */
      {
       int num=8,nm;
       unsigned long d1,d3;

       current[DISK_READ]=0;
       current[DISK_WRITE]=0;

       if(available[DISK])
          while(sscanf(line+num," (%*d,%*d):(%*u,%lu,%*u,%lu)%n",&d1,&d3,&nm)==2)
            {
             current[DISK_READ] +=d1;
             current[DISK_WRITE]+=d3;

             num+=nm;
            }

       fgets(line,BUFFLEN,f);
      }
    if(available[CONTEXT])
       sscanf(line,"ctxt %lu",&current[CONTEXT]);

    if(available[CPU])
       current[CPU]=current[CPU_USER]+current[CPU_NICE]+current[CPU_SYS];

    if(available[PAGE])
       current[PAGE]=current[PAGE_IN]+current[PAGE_OUT];

    if(available[SWAP])
       current[SWAP]=current[SWAP_IN]+current[SWAP_OUT];

    if(available[DISK_READ] && available[DISK_WRITE])
       current[DISK]=current[DISK_READ]+current[DISK_WRITE];

    fclose(f);

    last=now;
   }

 for(i=0;i<N_OUTPUTS;i++)
    if(output==&_outputs[i])
      {
       long tot;
       double value;

       switch(i)
         {
         case CPU:
         case CPU_USER:
         case CPU_NICE:
         case CPU_SYS:
         case CPU_IDLE:
          tot=current[CPU]+current[CPU_IDLE]-previous[CPU]-previous[CPU_IDLE];

          if(tot)
             value=100.0*(double)(current[i]-previous[i]+0.5)/(double)tot;
          else
             value=0.0;
          if(value>100.0)
             value=100.0;
          else if(value<0.0)
             value=0.0;

          output->graph_value=PROCMETER_GRAPH_FLOATING(value/output->graph_scale);
          sprintf(output->text_value,"%.0f %%",value);
          break;

         default:
          if(previous[i]>current[i])
             value=0.0;
          else
             value=(double)(current[i]-previous[i])/output->interval;

          output->graph_value=PROCMETER_GRAPH_FLOATING(value/output->graph_scale);
          sprintf(output->text_value,"%.0f /s",value);
         }

       return(0);
      }

 return(-1);
}


/*++++++++++++++++++++++++++++++++++++++
  Tidy up and prepare to have the module unloaded.
  ++++++++++++++++++++++++++++++++++++++*/

void Unload(void)
{
}
