/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/stat.c,v 1.1 1998-09-19 15:25:57 amb Exp $

  ProcMeter - A system monitoring program for Linux.

  Module template source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
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

/* The interface information.  */

ProcMeterOutput _outputs[N_OUTPUTS]=
{
 /*+ The total cpu output +*/
 {
  /* char  name[16];         */ "CPU",
  /* char *description;      */ "The total fraction of the time that the CPU is busy.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 %",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 20,
  /* char  graph_units[8];   */ "(20%)"
 },
 /*+ The user cpu output +*/
 {
  /* char  name[16];         */ "CPU_User",
  /* char *description;      */ "The fraction of the time that the CPU is processing user level code (applications).",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 %",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 20,
  /* char  graph_units[8];   */ "(20%)"
 },
 /*+ The nice cpu output +*/
 {
  /* char  name[16];         */ "CPU_Nice",
  /* char *description;      */ "The fraction of the time that the CPU is running processes that run at a lowered priority.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 %",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 20,
  /* char  graph_units[8];   */ "(20%)"
 },
 /*+ The system cpu output +*/
 {
  /* char  name[16];         */ "CPU_System",
  /* char *description;      */ "The fraction of the time that the CPU is processing system level code (kernel).",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 %",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 20,
  /* char  graph_units[8];   */ "(20%)"
 },
 /*+ The idle cpu output +*/
 {
  /* char  name[16];         */ "CPU_Idle",
  /* char *description;      */ "The fraction of the time that the CPU is idle.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 %",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 20,
  /* char  graph_units[8];   */ "(20%)"
 },
 /*+ The disk blocks accessed per second +*/
 {
  /* char  name[16];         */ "Disk",
  /* char *description;      */ "The total number of disk blocks accessed per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(25/s)"
 },
 /*+ The disk blocks read per second +*/
 {
  /* char  name[16];         */ "Disk_Read",
  /* char *description;      */ "The number of disk blocks that are read per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(25/s)"
 },
 /*+ The disk blocks write per second +*/
 {
  /* char  name[16];         */ "Disk_Write",
  /* char *description;      */ "The number of disk blocks that are written per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(25/s)"
 },
 /*+ The swap blocks accessed per second +*/
 {
  /* char  name[16];         */ "Swap",
  /* char *description;      */ "The total number of swap space blocks accessed per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(50/s)"
 },
 /*+ The swap blocks in per second +*/
 {
  /* char  name[16];         */ "Swap_In",
  /* char *description;      */ "The number of swap space blocks that are read in per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(50/s)"
 },
 /*+ The swap blocks out per second +*/
 {
  /* char  name[16];         */ "Swap_Out",
  /* char *description;      */ "The number of swap space blocks that are written out per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(50/s)"
 },
 /*+ The page blocks accessed per second +*/
 {
  /* char  name[16];         */ "Page",
  /* char *description;      */ "The number of paged blocks accessed per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(50/s)"
 },
 /*+ The page blocks in per second +*/
 {
  /* char  name[16];         */ "Page_In",
  /* char *description;      */ "The number of paged blocks that are read in per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(50/s)"
 },
 /*+ The page blocks out per second +*/
 {
  /* char  name[16];         */ "Page_Out",
  /* char *description;      */ "The number of paged blocks that are dumped out per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 50,
  /* char  graph_units[8];   */ "(50/s)"
 },
 /*+ The number of context switches per second +*/
 {
  /* char  name[16];         */ "Context",
  /* char *description;      */ "The number of context switches between running processes per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 100,
  /* char  graph_units[8];   */ "(100/s)"
 },
 /*+ The number of interrupts per second +*/
 {
  /* char  name[16];         */ "Interrupts",
  /* char *description;      */ "The number of hardware interrupts per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 100,
  /* char  graph_units[8];   */ "(100/s)"
 }
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[N_OUTPUTS+1];

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];             */ "Statistics",
 /* char *description;         */ "Low level system statistics. [From /proc/stat]",
};


static int available[N_OUTPUTS];
static long *current,*previous,values[2][N_OUTPUTS];


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
 char line[256];
 int n;

 for(n=0;n<N_OUTPUTS;n++)
   {
    available[n]=0;
    values[0][n]=values[1][n]=0;
    outputs[n]=NULL;
   }
 outputs[N_OUTPUTS]=NULL;
 n=0;

 current=values[0];
 previous=values[1];

 /* Verify the statistics from /proc/stat */

 f=fopen("/proc/stat","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/stat'.\n",__FILE__);
 else
   {
    if(!fgets(line,256,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/stat'.\n",__FILE__);
    else
      {
       unsigned long d0,d1,d2,d3;
       int i;

       if(sscanf(line,"cpu %lu %lu %lu %lu",&current[CPU_USER],&current[CPU_NICE],&current[CPU_SYS],&current[CPU_IDLE])==4)
          available[CPU]=available[CPU_USER]=available[CPU_NICE]=available[CPU_SYS]=available[CPU_IDLE]=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'cpu' line in '/proc/stat'.\n",__FILE__);

       while(line[0]=='c')      /* kernel version > ~2.1.84 */
          fgets(line,256,f);

       if(sscanf(line,"disk %lu %lu %lu %lu",&d0,&d1,&d2,&d3)==4)
          {available[DISK]=1;current[DISK]=d0+d1+d2+d3;}
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'disk' line in '/proc/stat'.\n",__FILE__);

       fgets(line,256,f);
       if(sscanf(line,"disk_rio %lu %lu %lu %lu",&d0,&d1,&d2,&d3)==4)
         {
          available[DISK_READ]=1;current[DISK_READ]=d0+d1+d2+d3;

          fgets(line,256,f);
          if(sscanf(line,"disk_wio %lu %lu %lu %lu",&d0,&d1,&d2,&d3)==4)
            {available[DISK_WRITE]=1;current[DISK_WRITE]=d0+d1+d2+d3;}
         }

       while(line[0]=='d')      /* kernel version > ~1.3.0 */
          fgets(line,256,f);

       if(sscanf(line,"page %lu %lu",&current[PAGE_IN],&current[PAGE_OUT])==2)
          available[PAGE]=available[PAGE_IN]=available[PAGE_OUT]=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'page' line in '/proc/stat'.\n",__FILE__);

       fgets(line,256,f);
       if(sscanf(line,"swap %lu %lu",&current[SWAP_IN],&current[SWAP_OUT])==2)
          available[SWAP]=available[SWAP_IN]=available[SWAP_OUT]=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'swap' line in '/proc/stat'.\n",__FILE__);

       fgets(line,256,f);
       if(sscanf(line,"intr %lu",&current[INTR])==1)
          available[INTR]=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'intr' line in '/proc/stat'.\n",__FILE__);

       fgets(line,256,f);
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
    char line[256];
    unsigned long d0,d1,d2,d3;
    long *temp;

    temp=current;
    current=previous;
    previous=temp;

    f=fopen("/proc/stat","r");
    if(!f)
       return(-1);

    fgets(line,256,f);
    if(available[CPU])
       sscanf(line,"cpu %lu %lu %lu %lu",&current[CPU_USER],&current[CPU_NICE],&current[CPU_SYS],&current[CPU_IDLE]);
    while(line[0]=='c')      /* kernel version > ~2.1.84 */
       fgets(line,256,f);
    if(available[DISK])
      {sscanf(line,"disk %lu %lu %lu %lu",&d0,&d1,&d2,&d3);current[DISK]=d0+d1+d2+d3;}
    if(available[DISK_READ])
      {fgets(line,256,f);
       sscanf(line,"disk_rio %lu %lu %lu %lu",&d0,&d1,&d2,&d3);current[DISK_READ]=d0+d1+d2+d3;}
    if(available[DISK_WRITE])
      {fgets(line,256,f);
       sscanf(line,"disk_wio %lu %lu %lu %lu",&d0,&d1,&d2,&d3);current[DISK_WRITE]=d0+d1+d2+d3;}
    while(line[0]=='d')      /* kernel version > ~1.3.x */
       fgets(line,256,f);
    if(available[PAGE])
       sscanf(line,"page %lu %lu",&current[PAGE_IN],&current[PAGE_OUT]);
    fgets(line,256,f);
    if(available[SWAP])
       sscanf(line,"swap %lu %lu",&current[SWAP_IN],&current[SWAP_OUT]);
    fgets(line,256,f);
    if(available[INTR])
       sscanf(line,"intr %lu",&current[INTR]);
    fgets(line,256,f);
    if(available[CONTEXT])
       sscanf(line,"ctxt %lu",&current[CONTEXT]);

    if(available[CPU])
       current[CPU]=current[CPU_USER]+current[CPU_NICE]+current[CPU_SYS];

    if(available[PAGE])
       current[PAGE]=current[PAGE_IN]+current[PAGE_OUT];

    if(available[SWAP])
       current[SWAP]=current[SWAP_IN]+current[SWAP_OUT];

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