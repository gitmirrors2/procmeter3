/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/loadavg.c,v 1.4 1999-09-29 18:59:59 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.2.

  Load average and number of processes module source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The load average output +*/
ProcMeterOutput loadavg_output=
{
 /* char  name[16];         */ "Load",
 /* char *description;      */ "The system load, a rolling average of the number of processes running.",
 /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
 /* short interval;         */ 1,
 /* char  text_value[16];   */ "unknown",
 /* long  graph_value;      */ 0,
 /* short graph_scale;      */ 1,
 /* char  graph_units[8];   */ "(%d)"
};

/*+ The number of processes output. +*/
ProcMeterOutput processes_output=
{
 /* char  name[16];         */ "Processes",
 /* char *description;      */ "The number of processes that exist in the system.",
 /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
 /* short interval;         */ 1,
 /* char  text_value[16];   */ "unknown",
 /* long  graph_value;      */ 0,
 /* short graph_scale;      */ 10,
 /* char  graph_units[8];   */ "(%d)"
};

/*+ The number of forks per second output. +*/
ProcMeterOutput forks_output=
{
 /* char  name[16];         */ "Forks",
 /* char *description;      */ "The number of new processes that start per second.",
 /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
 /* short interval;         */ 1,
 /* char  text_value[16];   */ "unknown",
 /* long  graph_value;      */ 0,
 /* short graph_scale;      */ 5,
 /* char  graph_units[8];   */ "(%d/s)"
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[4];

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];          */ "Processes",
 /* char *description;      */ "The load average and the number of processes running and starting. [From /proc/loadavg]",
};


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
 char line[80];
 int n;

 for(n=0;n<sizeof(outputs)/sizeof(outputs[0]);n++)
    outputs[n]=NULL;
 n=0;

 /* Verify the statistics from /proc/loadavg */

 f=fopen("/proc/loadavg","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/loadavg'.\n",__FILE__);
 else
   {
    if(!fgets(line,80,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/loadavg'.\n",__FILE__);
    else
      {
       int loadavg_available=0,processes_available=0,forks_available=0;
       double d;
       long p1,p2;

       if(sscanf(line,"%lf %*f %*f %*d/%ld %ld",&d,&p1,&p2)==3) /* kernel version > ~1.2.0? */
          loadavg_available=processes_available=forks_available=1;
       else if(sscanf(line,"%lf %*f %*f %*d/%ld",&d,&p1)==2) /* kernel version > ~1.2.0 */
          loadavg_available=processes_available=1;
       else if(sscanf(line,"%lf",&d)==1) /* kernel version < ~1.2.0 */
          loadavg_available=1;
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected line in '/proc/loadavg'.\n",__FILE__);

       if(loadavg_available)   outputs[n++]=&loadavg_output;
       if(processes_available) outputs[n++]=&processes_output;
       if(forks_available)     outputs[n++]=&forks_output;
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
 static float l,delta_p;
 static long n,p=0;

 /* Get the statistics from /proc/loadavg */

 if(last!=now)
   {
    FILE *f;
    long p2;

    f=fopen("/proc/loadavg","r");
    if(!f)
       return(-1);

    fscanf(f,"%f %*f %*f %*d/%ld %ld",&l,&n,&p2);

    fclose(f);

    if(last && p)
      {
       while(p2<p)
          p-=32768;
       delta_p=(float)(p2-p)/(float)(now-last);
      }
    else
       delta_p=0.0;
    p=p2;

    last=now;
   }

 if(output==&loadavg_output)
   {
    sprintf(output->text_value,"%.2f",l);
    output->graph_value=PROCMETER_GRAPH_FLOATING(l/output->graph_scale);
    return(0);
   }
 else if(output==&processes_output)
   {
    sprintf(output->text_value,"%ld",n);
    output->graph_value=PROCMETER_GRAPH_FLOATING((double)n/output->graph_scale);
    return(0);
   }
 else if(output==&forks_output)
   {
    sprintf(output->text_value,"%.1f",delta_p);
    output->graph_value=PROCMETER_GRAPH_FLOATING(delta_p/output->graph_scale);
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
