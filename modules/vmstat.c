/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/vmstat.c,v 1.4 2005-10-15 18:16:46 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4e.

  Low level system VM statistics source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2000,02,04 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "procmeter.h"

#define PAGE       0
#define PAGE_IN    1
#define PAGE_OUT   2
#define SWAP       3
#define SWAP_IN    4
#define SWAP_OUT   5
#define N_OUTPUTS  6

/*+ The length of the buffer for reading in lines. +*/
#define BUFFLEN 256

/* The interface information.  */

/*+ The statistics +*/
ProcMeterOutput _outputs[N_OUTPUTS]=
{
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
 }
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[N_OUTPUTS+1];

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];            */ "VM_Statistics",
 /* char *description;      */ "Low level system virtual memory statistics. [From /proc/vmstat]",
};


static int available[N_OUTPUTS];
static unsigned long *current,*previous,values[2][N_OUTPUTS];


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
 char line[BUFFLEN+1],*l;
 int n;

 outputs[0]=NULL;
 for(n=0;n<N_OUTPUTS;n++)
    available[n]=0;
 n=0;

 current=values[0];
 previous=values[1];

 /* Verify the statistics from /proc/stat */

 f=fopen("/proc/vmstat","r");
 if(!f)
    ; /*fprintf(stderr,"ProcMeter(%s): Could not open '/proc/vmstat'.\n",__FILE__); */
 else
   {
    l=fgets(line,BUFFLEN,f);
    if(!l)
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/vmstat'.\n",__FILE__);
    else
      {
       unsigned long d;
       int lineno=1,i;

       do
         {
          if(sscanf(line,"pgpgin %lu",&d)==1)
             available[PAGE_IN]=lineno;
          if(sscanf(line,"pgpgout %lu",&d)==1)
             available[PAGE_OUT]=lineno;
          if(sscanf(line,"pswpin %lu",&d)==1)
             available[SWAP_IN]=lineno;
          if(sscanf(line,"pswpout %lu",&d)==1)
             available[SWAP_OUT]=lineno;

          l=fgets(line,BUFFLEN,f);
          lineno++;
         }
       while(l);

       if(available[PAGE_IN] && available[PAGE_OUT])
          available[PAGE]=1;

       if(available[SWAP_IN] && available[SWAP_OUT])
          available[SWAP]=1;

       for(i=0;i<N_OUTPUTS;i++)
          if(available[i])
             outputs[n++]=&_outputs[i];

       for(i=0;i<N_OUTPUTS;i++)
          current[i]=previous[i]=0;
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

 /* Get the statistics from /proc/vmstat */

 if(now!=last)
   {
    FILE *f;
    char line[BUFFLEN+1],*l;
    unsigned long *temp;
    int lineno=1;

    temp=current;
    current=previous;
    previous=temp;

    f=fopen("/proc/vmstat","r");
    if(!f)
       return(-1);

    while((l=fgets(line,BUFFLEN,f)))
      {
       if(available[PAGE_IN]==lineno)
          sscanf(line,"pgpgin %lu",&current[PAGE_IN]);

       if(available[PAGE_OUT]==lineno)
          sscanf(line,"pgpgout %lu",&current[PAGE_OUT]);

       if(available[SWAP_IN]==lineno)
          sscanf(line,"pswpin %lu",&current[SWAP_IN]);

       if(available[SWAP_OUT]==lineno)
          sscanf(line,"pswpout %lu",&current[SWAP_OUT]);

       lineno++;
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
       double value;

       if(previous[i]>current[i])
          value=0.0;
       else
          value=(double)(current[i]-previous[i])/output->interval;

       output->graph_value=PROCMETER_GRAPH_FLOATING(value/output->graph_scale);
       sprintf(output->text_value,"%.0f /s",value);

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
