/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/stat.c,v 1.18 2008-05-05 18:45:36 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.5b.

  Low level system statistics source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998-2008 Andrew M. Bishop
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

/* The interface information.  */

/*+ The statistics +*/
ProcMeterOutput _outputs[N_OUTPUTS]=
{
 /*+ The total cpu output +*/
 {
  /* char  name[];          */ "CPU",
  /* char *description;     */ "The total fraction of the time that the CPU is busy.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The user cpu output +*/
 {
  /* char  name[];          */ "CPU_User",
  /* char *description;     */ "The fraction of the time that the CPU is processing user level code (applications).",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The nice cpu output +*/
 {
  /* char  name[];          */ "CPU_Nice",
  /* char *description;     */ "The fraction of the time that the CPU is running processes that run at a lowered priority.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The system cpu output +*/
 {
  /* char  name[];          */ "CPU_System",
  /* char *description;     */ "The fraction of the time that the CPU is processing system level code (kernel).",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The idle cpu output +*/
 {
  /* char  name[];          */ "CPU_Idle",
  /* char *description;     */ "The fraction of the time that the CPU is idle.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The disk blocks accessed per second +*/
 {
  /* char  name[];          */ "Disk",
  /* char *description;     */ "The total number of disk blocks accessed per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The disk blocks read per second +*/
 {
  /* char  name[];          */ "Disk_Read",
  /* char *description;     */ "The total number of disk blocks that are read per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The disk blocks write per second +*/
 {
  /* char  name[];          */ "Disk_Write",
  /* char *description;     */ "The total number of disk blocks that are written per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The swap blocks accessed per second +*/
 {
  /* char  name[];          */ "Swap",
  /* char *description;     */ "The total number of swap space blocks accessed per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 50,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The swap blocks in per second +*/
 {
  /* char  name[];          */ "Swap_In",
  /* char *description;     */ "The number of swap space blocks that are read in per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 50,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The swap blocks out per second +*/
 {
  /* char  name[];          */ "Swap_Out",
  /* char *description;     */ "The number of swap space blocks that are written out per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 50,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The page blocks accessed per second +*/
 {
  /* char  name[];          */ "Page",
  /* char *description;     */ "The number of paged blocks accessed per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 50,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The page blocks in per second +*/
 {
  /* char  name[];          */ "Page_In",
  /* char *description;     */ "The number of paged blocks that are read in per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 50,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The page blocks out per second +*/
 {
  /* char  name[];          */ "Page_Out",
  /* char *description;     */ "The number of paged blocks that are dumped out per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 50,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The number of context switches per second +*/
 {
  /* char  name[];          */ "Context",
  /* char *description;     */ "The number of context switches between running processes per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 100,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The number of interrupts per second +*/
 {
  /* char  name[];          */ "Interrupts",
  /* char *description;     */ "The number of hardware interrupts per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 100,
  /* char  graph_units[];   */ "(%d/s)"
 }
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[N_OUTPUTS+1];

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];            */ "Statistics",
 /* char *description;      */ "Low level system statistics. [From /proc/stat]",
};


/* The line buffer */
static char *line=NULL;
static size_t length=0;

/* Information about the availability and the current and previous values */
static int available[N_OUTPUTS];
static unsigned long long *current,*previous,values[2][N_OUTPUTS];

/*+ A flag to indicate that this is kernel v2.4 and has many disk IO counters. +*/
static int kernel_version_240=0;

/*+ A flag to indicate that this is kernel v2.6 and has 8 CPU counters. +*/
static int kernel_26=0;


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
    if(!fgets_realloc(&line,&length,f)) /* cpu */
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/stat'.\n",__FILE__);
    else
      {
       int i;
       unsigned long long d1,d2,d3,d4,d5,d6,d7,d8;

       if(sscanf(line,"cpu %llu %llu %llu %llu %llu %llu %llu %llu",&d1,&d2,&d3,&d4,&d5,&d6,&d7,&d8)==8)
          kernel_26=1;
 
       if(kernel_26 || sscanf(line,"cpu %llu %llu %llu %llu",&d1,&d2,&d3,&d4)==4)
          available[CPU]=available[CPU_USER]=available[CPU_NICE]=available[CPU_SYS]=available[CPU_IDLE]=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'cpu' line in '/proc/stat'.\n"
                  "    expected: 'cpu %%llu %%llu %%llu %%llu'\n"
                  "          or: 'cpu %%llu %%llu %%llu %%llu %%llu %%llu %%llu %%llu'\n"
                  "    found:    %s",__FILE__,line);

       /* cpu or disk or page or intr */
       while(fgets_realloc(&line,&length,f) && (line[0]=='c' && line[1]=='p' && line[2]=='u')) /* kernel version > ~2.1.84 */
          ;

       if(!strncmp(line,"disk ",5)) /* kernel version < ~2.4.0-test4 */
         {
          unsigned long long d1,d2,d3,d4;

          if(sscanf(line,"disk %llu %llu %llu %llu",&d1,&d2,&d3,&d4)==4)
             available[DISK]=1;
          else
             fprintf(stderr,"ProcMeter(%s): Unexpected 'disk' line in '/proc/stat'.\n"
                            "    expected: 'disk %%llu %%llu %%llu %%llu'\n"
                            "    found:    %s",__FILE__,line);

          /* disk_* or page */
          while(fgets_realloc(&line,&length,f) && line[0]=='d') /* kernel version > ~1.3.0 */
            {
             if(sscanf(line,"disk_rblk %llu %llu %llu %llu",&d1,&d2,&d3,&d4)==4)
                available[DISK_READ]=1;
             if(sscanf(line,"disk_wblk %llu %llu %llu %llu",&d1,&d2,&d3,&d4)==4)
                available[DISK_WRITE]=1;
            }
         }

       if(!strncmp(line,"page",4)) /* kernel version < ~2.5.30 */
         {
          if(sscanf(line,"page %llu %llu",&d1,&d2)==2)
            {
             available[PAGE]=available[PAGE_IN]=available[PAGE_OUT]=1;
             fgets_realloc(&line,&length,f); /* swap or intr */
            }
          else
             fprintf(stderr,"ProcMeter(%s): Unexpected 'page' line in '/proc/stat'.\n"
                            "    expected: 'page %%llu %%llu'\n"
                            "    found:    %s",__FILE__,line);

          if(sscanf(line,"swap %llu %llu",&d1,&d2)==2)
            {
             available[SWAP]=available[SWAP_IN]=available[SWAP_OUT]=1;
             fgets_realloc(&line,&length,f); /* intr */
            }
          else
             fprintf(stderr,"ProcMeter(%s): Unexpected 'swap' line in '/proc/stat'.\n"
                            "    expected: 'swap %%llu %%llu'\n"
                            "    found:    %s",__FILE__,line);
         }

       if(sscanf(line,"intr %llu",&d1)==1)
         {
          available[INTR]=1;
          fgets_realloc(&line,&length,f); /* disk_io or ctxt */
         }
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'intr' line in '/proc/stat'.\n"
                         "    expected: 'intr %%llu ...'\n"
                         "    found:    %s",__FILE__,line);

       if(!strncmp(line,"disk_io: ",9)) /* kernel version > ~2.4.0-test4 */
         {
          int maj,idx,num=8,nm,nr;
          unsigned long long d0,d1,d2,d3,d4;

          kernel_version_240=1;

          while((nr=sscanf(line+num," (%d,%d):(%llu,%llu,%llu,%llu,%llu)%n",&maj,&idx,&d0,&d1,&d2,&d3,&d4,&nm))==7 ||
                (nr=sscanf(line+num," (%d,%d):(%llu,%llu,%llu,%llu)%n",&maj,&idx,&d0,&d1,&d2,&d3,&nm))==6)
            {
             num+=nm;

             kernel_version_240=nr;

             available[DISK]=1;
             available[DISK_READ]=1;
             available[DISK_WRITE]=1;
            }

          fgets_realloc(&line,&length,f); /* ctxt */
         }

       if(sscanf(line,"ctxt %llu",&d1)==1)
          available[CONTEXT]=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'ctxt' line in '/proc/stat'.\n"
                         "    expected: 'ctxt %%llu'\n"
                         "    found:    %s",__FILE__,line);

       for(i=0;i<N_OUTPUTS;i++)
         {
          current[i]=previous[i]=0;
          if(available[i])
             outputs[n++]=&_outputs[i];
         }
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
    unsigned long long *temp;
    unsigned long long cpu_iowait,cpu_irq,cpu_softirq,cpu_steal;

    temp=current;
    current=previous;
    previous=temp;

    f=fopen("/proc/stat","r");
    if(!f)
       return(-1);

    fgets_realloc(&line,&length,f); /* cpu */
    if(available[CPU])
       sscanf(line,"cpu %llu %llu %llu %llu %llu %llu %llu %llu",&current[CPU_USER],&current[CPU_NICE],&current[CPU_SYS],&current[CPU_IDLE],
                                                                 &cpu_iowait,&cpu_irq,&cpu_softirq,&cpu_steal);

    /* cpu or disk or page or intr */
    while(fgets_realloc(&line,&length,f) && line[0]=='c') /* kernel version > ~2.1.84 */
       ;

    if(available[DISK] && !kernel_version_240) /* kernel version < ~2.4.0-test4 */
      {
       unsigned long long d0,d1,d2,d3;

       if(available[DISK])
         {sscanf(line,"disk %llu %llu %llu %llu",&d0,&d1,&d2,&d3);current[DISK]=d0+d1+d2+d3;}

       /* disk_* or page */
       while(fgets_realloc(&line,&length,f) && line[0]=='d') /* kernel version > ~1.3.x */
         {
          if(available[DISK_READ])
             if(sscanf(line,"disk_rblk %llu %llu %llu %llu",&d0,&d1,&d2,&d3)==4)
               {current[DISK_READ]=d0+d1+d2+d3;}
          if(available[DISK_WRITE])
             if(sscanf(line,"disk_wblk %llu %llu %llu %llu",&d0,&d1,&d2,&d3)==4)
               {current[DISK_WRITE]=d0+d1+d2+d3;}
         }
      }

    if(available[PAGE])
      {
       sscanf(line,"page %llu %llu",&current[PAGE_IN],&current[PAGE_OUT]);
       fgets_realloc(&line,&length,f); /* swap */
      }

    if(available[SWAP])
      {
       sscanf(line,"swap %llu %llu",&current[SWAP_IN],&current[SWAP_OUT]);
       fgets_realloc(&line,&length,f); /* intr */
      }

    if(available[INTR])
      {
       sscanf(line,"intr %llu",&current[INTR]);
       fgets_realloc(&line,&length,f); /* disk_io or ctxt */
      }

    if(kernel_version_240 && available[DISK]) /* kernel version > ~2.4.0-test4 */
      {
       int num=8,nm,nr=0;
       unsigned long long d1,d3;

       current[DISK_READ]=0;
       current[DISK_WRITE]=0;

       if(available[DISK])
          while(1)
            {
             if(kernel_version_240==6)
                nr=sscanf(line+num," (%*d,%*d):(%*u,%llu,%*u,%llu)%n",&d1,&d3,&nm);
             else if(kernel_version_240==7)
                nr=sscanf(line+num," (%*d,%*d):(%*u,%llu,%*u,%llu,%*u)%n",&d1,&d3,&nm);

             if(nr!=2)
                break;

             current[DISK_READ] +=d1;
             current[DISK_WRITE]+=d3;

             num+=nm;
            }

       if(available[DISK])
          current[DISK]=current[DISK_READ]+current[DISK_WRITE];

       fgets_realloc(&line,&length,f); /* ctxt */
      }

    if(available[CONTEXT])
       sscanf(line,"ctxt %llu",&current[CONTEXT]);

    if(available[CPU])
      {
       current[CPU]=current[CPU_USER]+current[CPU_NICE]+current[CPU_SYS];
       if(kernel_26)
          current[CPU]+=cpu_iowait+cpu_irq+cpu_softirq+cpu_steal;
      }

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
 if(line)
    free(line);
}
