/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/stat-cpu.c,v 1.6 2002-06-04 13:54:06 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3b.

  Low level system statistics for CPU usage.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2002 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procmeter.h"

#define CPU         0
#define CPU_USER    1
#define CPU_NICE    2
#define CPU_SYS     3
#define CPU_IDLE    4
#define N_OUTPUTS   5

/* The interface information.  */

/*+ The normal outputs +*/
ProcMeterOutput _outputs[N_OUTPUTS]=
{
 /*+ The total cpu output +*/
 {
  /* char  name[PROCMETER_NAME_LEN]; */ "CPU",
  /* char *description;              */ "The total fraction of the time that the CPU is busy.",
  /* char  type;                     */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;                 */ 1,
  /* char  text_value[16];           */ "0 %",
  /* long  graph_value;              */ 0,
  /* short graph_scale;              */ 20,
  /* char  graph_units[8];           */ "(%d%%)"
 },
 /*+ The user cpu output +*/
 {
  /* char  name[PROCMETER_NAME_LEN]; */ "CPU_User",
  /* char *description;              */ "The fraction of the time that the CPU is processing user level code (applications).",
  /* char  type;                     */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;                 */ 1,
  /* char  text_value[16];           */ "0 %",
  /* long  graph_value;              */ 0,
  /* short graph_scale;              */ 20,
  /* char  graph_units[8];           */ "(%d%%)"
 },
 /*+ The nice cpu output +*/
 {
  /* char  name[PROCMETER_NAME_LEN]; */ "CPU_Nice",
  /* char *description;              */ "The fraction of the time that the CPU is running processes that run at a lowered priority.",
  /* char  type;                     */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;                 */ 1,
  /* char  text_value[16];           */ "0 %",
  /* long  graph_value;              */ 0,
  /* short graph_scale;              */ 20,
  /* char  graph_units[8];           */ "(%d%%)"
 },
 /*+ The system cpu output +*/
 {
  /* char  name[PROCMETER_NAME_LEN]; */ "CPU_System",
  /* char *description;              */ "The fraction of the time that the CPU is processing system level code (kernel).",
  /* char  type;                     */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;                 */ 1,
  /* char  text_value[16];           */ "0 %",
  /* long  graph_value;              */ 0,
  /* short graph_scale;              */ 20,
  /* char  graph_units[8];           */ "(%d%%)"
 },
 /*+ The idle cpu output +*/
 {
  /* char  name[PROCMETER_NAME_LEN]; */ "CPU_Idle",
  /* char *description;              */ "The fraction of the time that the CPU is idle.",
  /* char  type;                     */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;                 */ 1,
  /* char  text_value[16];           */ "0 %",
  /* long  graph_value;              */ 0,
  /* short graph_scale;              */ 20,
  /* char  graph_units[8];           */ "(%d%%)"
 }
};

/*+ The outputs with multiple CPUs +*/
ProcMeterOutput _smp_outputs[]=
{
 /*+ The total cpu output +*/
 {
  /* char  name[PROCMETER_NAME_LEN]; */ "CPU%d",
  /* char *description;              */ "The total fraction of the time that the CPU number %d is busy.",
  /* char  type;                     */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;                 */ 1,
  /* char  text_value[16];           */ "0 %",
  /* long  graph_value;              */ 0,
  /* short graph_scale;              */ 20,
  /* char  graph_units[8];           */ "(%d%%)"
 },
 /*+ The user cpu output +*/
 {
  /* char  name[PROCMETER_NAME_LEN]; */ "CPU%d_User",
  /* char *description;              */ "The fraction of the time that the CPU number %d is processing user level code (applications).",
  /* char  type;                     */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;                 */ 1,
  /* char  text_value[16];           */ "0 %",
  /* long  graph_value;              */ 0,
  /* short graph_scale;              */ 20,
  /* char  graph_units[8];           */ "(%d%%)"
 },
 /*+ The nice cpu output +*/
 {
  /* char  name[PROCMETER_NAME_LEN]; */ "CPU%d_Nice",
  /* char *description;              */ "The fraction of the time that the CPU number %d is running processes that run at a lowered priority.",
  /* char  type;                     */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;                 */ 1,
  /* char  text_value[16];           */ "0 %",
  /* long  graph_value;              */ 0,
  /* short graph_scale;              */ 20,
  /* char  graph_units[8];           */ "(%d%%)"
 },
 /*+ The system cpu output +*/
 {
  /* char  name[PROCMETER_NAME_LEN]; */ "CPU%d_System",
  /* char *description;              */ "The fraction of the time that the CPU number %d is processing system level code (kernel).",
  /* char  type;                     */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;                 */ 1,
  /* char  text_value[16];           */ "0 %",
  /* long  graph_value;              */ 0,
  /* short graph_scale;              */ 20,
  /* char  graph_units[8];           */ "(%d%%)"
 },
 /*+ The idle cpu output +*/
 {
  /* char  name[PROCMETER_NAME_LEN]; */ "CPU%d_Idle",
  /* char *description;              */ "The fraction of the time that the CPU number %d is idle.",
  /* char  type;                     */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;                 */ 1,
  /* char  text_value[16];           */ "0 %",
  /* long  graph_value;              */ 0,
  /* short graph_scale;              */ 20,
  /* char  graph_units[8];           */ "(%d%%)"
 }
};


/*+ The number of CPUs (or 0 for only 1!). +*/
int ncpus=0;

/*+ The extra outputs with multiple CPUs +*/
ProcMeterOutput *smp_outputs=NULL;

/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[PROCMETER_NAME_LEN]; */ "Stat-CPU",
 /* char *description;             */ "CPU usage statistics. [From /proc/stat]",
};


static long *current,*previous,values[2][N_OUTPUTS];
static long *smp_current,*smp_previous,*smp_values[2]={NULL,NULL};


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
 int n=0;

 outputs=(ProcMeterOutput**)malloc(sizeof(ProcMeterOutput*));
 outputs[0]=NULL;

 current=values[0];
 previous=values[1];

 /* Verify the statistics from /proc/stat */

 f=fopen("/proc/stat","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/stat'.\n",__FILE__);
 else
   {
    if(!fgets(line,256,f)) /* cpu */
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/stat'.\n",__FILE__);
    else
      {
       if(sscanf(line,"cpu %lu %lu %lu %lu",&current[CPU_USER],&current[CPU_NICE],&current[CPU_SYS],&current[CPU_IDLE])==4)
         {
          int i;

          current[CPU]=current[CPU_USER]+current[CPU_NICE]+current[CPU_SYS];

          fgets(line,256,f); /* cpu or disk or page */
          while(line[0]=='c' && line[1]=='p' && line[2]=='u') /* kernel version > ~2.1.84 */
            {
             int ncpu;
             long cpu_user,cpu_nice,cpu_sys,cpu_idle;

             if(sscanf(line,"cpu%d %lu %lu %lu %lu",&ncpu,&cpu_user,&cpu_nice,&cpu_sys,&cpu_idle)==5)
               {
                ncpus++;

                smp_values[0]=(long*)realloc((void*)smp_values[0],ncpus*N_OUTPUTS*sizeof(long));
                smp_values[1]=(long*)realloc((void*)smp_values[1],ncpus*N_OUTPUTS*sizeof(long));
                smp_current=smp_values[0]; smp_previous=smp_values[1];

                smp_current[CPU_USER+ncpu*N_OUTPUTS]=cpu_user;
                smp_current[CPU_NICE+ncpu*N_OUTPUTS]=cpu_nice;
                smp_current[CPU_SYS +ncpu*N_OUTPUTS]=cpu_sys;
                smp_current[CPU_IDLE+ncpu*N_OUTPUTS]=cpu_idle;

                smp_current[CPU+ncpu*N_OUTPUTS]=smp_current[CPU_USER+ncpu*N_OUTPUTS]+smp_current[CPU_NICE+ncpu*N_OUTPUTS]+smp_current[CPU_SYS+ncpu*N_OUTPUTS];

                smp_outputs=(ProcMeterOutput*)realloc((void*)smp_outputs,ncpus*N_OUTPUTS*sizeof(ProcMeterOutput));

                for(i=0;i<N_OUTPUTS;i++)
                  {
                   smp_outputs[i+ncpu*N_OUTPUTS]=_smp_outputs[i];
                   snprintf(smp_outputs[i+ncpu*N_OUTPUTS].name, PROCMETER_NAME_LEN, _smp_outputs[i].name, ncpu);
                   smp_outputs[i+ncpu*N_OUTPUTS].description=(char*)malloc(strlen(_smp_outputs[i].description)+8);
                   sprintf(smp_outputs[i+ncpu*N_OUTPUTS].description,_smp_outputs[i].description,ncpu);
                  }
               }
             else
                fprintf(stderr,"ProcMeter(%s): Unexpected 'cpu%d' line in '/proc/stat'.\n",__FILE__,ncpu);

             fgets(line,256,f); /* cpu or disk or page */
            }

          outputs=(ProcMeterOutput**)realloc((void*)outputs,(1+N_OUTPUTS+ncpus*N_OUTPUTS)*sizeof(ProcMeterOutput*));

          for(i=0;i<N_OUTPUTS;i++)
             outputs[n++]=&_outputs[i];

          for(i=0;i<ncpus*N_OUTPUTS;i++)
             outputs[n++]=&smp_outputs[i];

          outputs[n]=NULL;
         }
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'cpu' line in '/proc/stat'.\n",__FILE__);
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
    long *temp;

    temp=current;
    current=previous;
    previous=temp;

    temp=smp_current;
    smp_current=smp_previous;
    smp_previous=temp;

    f=fopen("/proc/stat","r");
    if(!f)
       return(-1);

    fgets(line,256,f); /* cpu */
    sscanf(line,"cpu %lu %lu %lu %lu",&current[CPU_USER],&current[CPU_NICE],&current[CPU_SYS],&current[CPU_IDLE]);
    current[CPU]=current[CPU_USER]+current[CPU_NICE]+current[CPU_SYS];

    fgets(line,256,f); /* cpu or disk or page */
    while(line[0]=='c' && line[1]=='p' && line[2]=='u') /* kernel version > ~2.1.84 */
      {
       int ncpu;
       long cpu_user,cpu_nice,cpu_sys,cpu_idle;

       sscanf(line,"cpu%d %lu %lu %lu %lu",&ncpu,&cpu_user,&cpu_nice,&cpu_sys,&cpu_idle);

       smp_current[CPU_USER+ncpu*N_OUTPUTS]=cpu_user;
       smp_current[CPU_NICE+ncpu*N_OUTPUTS]=cpu_nice;
       smp_current[CPU_SYS+ncpu*N_OUTPUTS]=cpu_sys;
       smp_current[CPU_IDLE+ncpu*N_OUTPUTS]=cpu_idle;

       smp_current[CPU+ncpu*N_OUTPUTS]=smp_current[CPU_USER+ncpu*N_OUTPUTS]+smp_current[CPU_NICE+ncpu*N_OUTPUTS]+smp_current[CPU_SYS+ncpu*N_OUTPUTS];

       fgets(line,256,f); /* cpu or disk or page */
      }

    fclose(f);

    last=now;
   }

 for(i=0;i<N_OUTPUTS;i++)
    if(output==&_outputs[i])
      {
       long tot;
       double value;

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

       return(0);
      }

 for(i=0;i<ncpus*N_OUTPUTS;i++)
    if(output==&smp_outputs[i])
      {
       int ncpu=i/N_OUTPUTS;
       long tot;
       double value;

       tot=smp_current[CPU+ncpu*N_OUTPUTS]+smp_current[CPU_IDLE+ncpu*N_OUTPUTS]-smp_previous[CPU+ncpu*N_OUTPUTS]-smp_previous[CPU_IDLE+ncpu*N_OUTPUTS];

       if(tot)
          value=100.0*(double)(smp_current[i]-smp_previous[i]+0.5)/(double)tot;
       else
          value=0.0;
       if(value>100.0)
          value=100.0;
       else if(value<0.0)
          value=0.0;

       output->graph_value=PROCMETER_GRAPH_FLOATING(value/output->graph_scale);
       sprintf(output->text_value,"%.0f %%",value);

       return(0);
      }

 return(-1);
}


/*++++++++++++++++++++++++++++++++++++++
  Tidy up and prepare to have the module unloaded.
  ++++++++++++++++++++++++++++++++++++++*/

void Unload(void)
{
 if(ncpus)
   {
    int i;

    for(i=0;i<ncpus*N_OUTPUTS;i++)
       free(smp_outputs[i].description);

    free(smp_outputs);

    free(smp_values[0]);
    free(smp_values[1]);
   }

 free(outputs);
}
