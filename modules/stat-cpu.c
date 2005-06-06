/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/stat-cpu.c,v 1.11 2005-06-06 18:31:30 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4e.

  Low level system statistics for CPU usage.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2002,04,05 Andrew M. Bishop
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
#define CPU_IOWAIT  5
#define CPU_IRQ     6
#define CPU_SOFTIRQ 7
#define CPU_STEAL   8

#define N_OUTPUTS_24 5
#define N_OUTPUTS_26 9

/*+ The length of the buffer for reading in lines. +*/
#define BUFFLEN 2048

/* The interface information.  */

/*+ The normal outputs +*/
ProcMeterOutput _outputs[N_OUTPUTS_26]=
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
 /*+ The iowait cpu output +*/
 {
  /* char  name[];          */ "CPU_IOWait",
  /* char *description;     */ "The fraction of the time that the CPU is waiting for IO.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The irq cpu output +*/
 {
  /* char  name[];          */ "CPU_IRQ",
  /* char *description;     */ "The fraction of the time that the CPU is waiting for IRQs.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The softirq cpu output +*/
 {
  /* char  name[];          */ "CPU_SoftIRQ",
  /* char *description;     */ "The fraction of the time that the CPU is ???.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The steal cpu output +*/
 {
  /* char  name[];          */ "CPU_Steal",
  /* char *description;     */ "The fraction of the time that the CPU is ???.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 }
};

/*+ The outputs with multiple CPUs +*/
ProcMeterOutput _smp_outputs[N_OUTPUTS_26]=
{
 /*+ The total cpu output +*/
 {
  /* char  name[];          */ "CPU%d",
  /* char *description;     */ "The total fraction of the time that the CPU number %d is busy.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The user cpu output +*/
 {
  /* char  name[];          */ "CPU%d_User",
  /* char *description;     */ "The fraction of the time that the CPU number %d is processing user level code (applications).",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The nice cpu output +*/
 {
  /* char  name[];          */ "CPU%d_Nice",
  /* char *description;     */ "The fraction of the time that the CPU number %d is running processes that run at a lowered priority.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The system cpu output +*/
 {
  /* char  name[];          */ "CPU%d_System",
  /* char *description;     */ "The fraction of the time that the CPU number %d is processing system level code (kernel).",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The idle cpu output +*/
 {
  /* char  name[];          */ "CPU%d_Idle",
  /* char *description;     */ "The fraction of the time that the CPU number %d is idle.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The iowait cpu output +*/
 {
  /* char  name[];          */ "CPU%d_IOWait",
  /* char *description;     */ "The fraction of the time that the CPU number %d is waiting for IO.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The irq cpu output +*/
 {
  /* char  name[];          */ "CPU%d_IRQ",
  /* char *description;     */ "The fraction of the time that the CPU number %d is waiting for IRQs.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The softirq cpu output +*/
 {
  /* char  name[];          */ "CPU%d_SoftIRQ",
  /* char *description;     */ "The fraction of the time that the CPU number %d is ???.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 },
 /*+ The steal cpu output +*/
 {
  /* char  name[];          */ "CPU%d_Steal",
  /* char *description;     */ "The fraction of the time that the CPU number %d is ???.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 %",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 20,
  /* char  graph_units[];   */ "(%d%%)"
 }
};


/*+ The extra outputs with multiple CPUs +*/
ProcMeterOutput *smp_outputs=NULL;

/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];            */ "Stat-CPU",
 /* char *description;      */ "CPU usage statistics. [From /proc/stat]",
};


static unsigned long long *current,*previous,values[2][N_OUTPUTS_26];
static unsigned long long *smp_current,*smp_previous,*smp_values[2]={NULL,NULL};

/*+ The number of CPUs (or 0 for only 1!). +*/
static int ncpus=0;

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
 char line[BUFFLEN],*l;
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
    if(!fgets(line,BUFFLEN,f)) /* cpu */
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/stat'.\n",__FILE__);
    else
      {
       unsigned long long d1,d2,d3,d4,d5,d6,d7,d8;

       if(sscanf(line,"cpu %llu %llu %llu %llu %llu %llu %llu %llu",&d1,&d2,&d3,&d4,&d5,&d6,&d7,&d8)==8)
          kernel_26=1;

       if(kernel_26 || sscanf(line,"cpu %llu %llu %llu %llu",&d1,&d2,&d3,&d4)==4)
         {
          int i,n_outputs;

          if(kernel_26)
             n_outputs=N_OUTPUTS_26;
          else
             n_outputs=N_OUTPUTS_24;

          l=fgets(line,BUFFLEN,f); /* cpu or disk or page */
          while(l && line[0]=='c' && line[1]=='p' && line[2]=='u') /* kernel version > ~2.1.84 */
            {
             int ncpu;

             if((kernel_26 && sscanf(line,"cpu%d %llu %llu %llu %llu %llu %llu %llu %llu",&ncpu,&d1,&d2,&d3,&d4,&d5,&d6,&d7,&d8)==9) ||
                sscanf(line,"cpu%d %llu %llu %llu %llu",&ncpu,&d1,&d2,&d3,&d4)==5)
               {
                ncpus++;

                smp_values[0]=(unsigned long long*)realloc((void*)smp_values[0],ncpus*n_outputs*sizeof(unsigned long long));
                smp_values[1]=(unsigned long long*)realloc((void*)smp_values[1],ncpus*n_outputs*sizeof(unsigned long long));
                smp_current=smp_values[0]; smp_previous=smp_values[1];

                smp_outputs=(ProcMeterOutput*)realloc((void*)smp_outputs,ncpus*n_outputs*sizeof(ProcMeterOutput));

                for(i=0;i<n_outputs;i++)
                  {
                   smp_outputs[i+ncpu*n_outputs]=_smp_outputs[i];
                   snprintf(smp_outputs[i+ncpu*n_outputs].name, PROCMETER_NAME_LEN+1, _smp_outputs[i].name, ncpu);
                   smp_outputs[i+ncpu*n_outputs].description=(char*)malloc(strlen(_smp_outputs[i].description)+8);
                   sprintf(smp_outputs[i+ncpu*n_outputs].description,_smp_outputs[i].description,ncpu);
                  }
               }
             else
                fprintf(stderr,"ProcMeter(%s): Unexpected 'cpu%d' line in '/proc/stat'.\n"
                               "    expected: 'cpu%d %%llu %%llu %%llu %%llu'\n"
                               "          or: 'cpu%d %%llu %%llu %%llu %%llu %%llu %%llu %%llu %%llu'\n"
                               "    found:    %s",__FILE__,ncpu,ncpu,ncpu,line);

             l=fgets(line,BUFFLEN,f); /* cpu or disk or page */
            }

          outputs=(ProcMeterOutput**)realloc((void*)outputs,(1+n_outputs+ncpus*n_outputs)*sizeof(ProcMeterOutput*));

          for(i=0;i<n_outputs;i++)
             outputs[n++]=&_outputs[i];

          for(i=0;i<ncpus*n_outputs;i++)
             outputs[n++]=&smp_outputs[i];

          for(i=0;i<N_OUTPUTS_26;i++)
             current[i]=previous[i]=0;

          for(i=0;i<ncpus*N_OUTPUTS_26;i++)
             smp_current[i]=smp_previous[i]=0;

          outputs[n]=NULL;
         }
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'cpu' line in '/proc/stat'.\n"
                         "    expected: 'cpu %%llu %%llu %%llu %%llu'\n"
                         "          or: 'cpu %%llu %%llu %%llu %%llu %%llu %%llu %%llu %%llu'\n"
                         "    found:    %s",__FILE__,line);
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
 int i,n_outputs;

 /* Get the statistics from /proc/stat */

 if(now!=last)
   {
    FILE *f;
    char line[BUFFLEN],*l;
    unsigned long long *temp;

    temp=current;
    current=previous;
    previous=temp;

    temp=smp_current;
    smp_current=smp_previous;
    smp_previous=temp;

    f=fopen("/proc/stat","r");
    if(!f)
       return(-1);

    l=fgets(line,BUFFLEN,f); /* cpu */
    sscanf(line,"cpu %llu %llu %llu %llu %llu %llu %llu %llu",&current[CPU_USER],&current[CPU_NICE],&current[CPU_SYS],&current[CPU_IDLE],
                                                              &current[CPU_IOWAIT],&current[CPU_IRQ],&current[CPU_SOFTIRQ],&current[CPU_STEAL]);

    current[CPU]=current[CPU_USER]+current[CPU_NICE]+current[CPU_SYS];
    if(kernel_26)
       current[CPU]+=current[CPU_IOWAIT]+current[CPU_IRQ]+current[CPU_SOFTIRQ]+current[CPU_STEAL];

    l=fgets(line,BUFFLEN,f); /* cpu or disk or page */
    while(l && line[0]=='c' && line[1]=='p' && line[2]=='u') /* kernel version > ~2.1.84 */
      {
       int ncpu,offset;
       unsigned long long cpu_user,cpu_nice,cpu_sys,cpu_idle,cpu_iowait,cpu_irq,cpu_softirq,cpu_steal;

       sscanf(line,"cpu%d %llu %llu %llu %llu %llu %llu %llu %llu",&ncpu,&cpu_user,&cpu_nice,&cpu_sys,&cpu_idle,
                                                                   &cpu_iowait,&cpu_irq,&cpu_softirq,&cpu_steal);

       offset=ncpu*N_OUTPUTS_26;

       smp_current[CPU_USER   +offset]=cpu_user;
       smp_current[CPU_NICE   +offset]=cpu_nice;
       smp_current[CPU_SYS    +offset]=cpu_sys;
       smp_current[CPU_IDLE   +offset]=cpu_idle;
       smp_current[CPU_IOWAIT +offset]=cpu_iowait;
       smp_current[CPU_IRQ    +offset]=cpu_irq;
       smp_current[CPU_SOFTIRQ+offset]=cpu_softirq;
       smp_current[CPU_STEAL  +offset]=cpu_steal;

       smp_current[CPU+offset]=smp_current[CPU_USER+offset]+smp_current[CPU_NICE+offset]+smp_current[CPU_SYS+offset];
       if(kernel_26)
          current[CPU+offset]+=current[CPU_IOWAIT+offset]+current[CPU_IRQ+offset]+current[CPU_SOFTIRQ+offset]+current[CPU_STEAL+offset];

       l=fgets(line,BUFFLEN,f); /* cpu or disk or page */
      }

    fclose(f);

    last=now;
   }

 if(kernel_26)
    n_outputs=N_OUTPUTS_26;
 else
    n_outputs=N_OUTPUTS_24;

 for(i=0;i<n_outputs;i++)
    if(output==&_outputs[i])
      {
       unsigned long long tot;
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

 for(i=0;i<ncpus*n_outputs;i++)
    if(output==&smp_outputs[i])
      {
       int ncpu=i/n_outputs,offset=ncpu*N_OUTPUTS_26;
       unsigned long long tot;
       double value;

       tot=smp_current[CPU+offset]+smp_current[CPU_IDLE+offset]-smp_previous[CPU+offset]-smp_previous[CPU_IDLE+offset];

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
    int i,n_outputs;

    if(kernel_26)
       n_outputs=N_OUTPUTS_26;
    else
       n_outputs=N_OUTPUTS_24;

    for(i=0;i<ncpus*n_outputs;i++)
       free(smp_outputs[i].description);

    free(smp_outputs);

    free(smp_values[0]);
    free(smp_values[1]);
   }

 free(outputs);
}
