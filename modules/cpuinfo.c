/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/cpuinfo.c,v 1.3 2007-08-21 17:32:54 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4g.

  CPU information.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2002,04,05,06,07 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procmeter.h"

#define CPU_SPEED   0
#define NOUTPUTS    1

/*+ The length of the buffer for reading in lines. +*/
#define BUFFLEN 2048

/* The interface information.  */

/*+ The normal outputs +*/
ProcMeterOutput _output=
 /*+ The cpu speed output +*/
 {
  /* char  name[];          */ "CPU_Speed",
  /* char *description;     */ "The speed of the CPU in MHz.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 500,
  /* char  graph_units[];   */ "(%d)"
 };

/*+ The outputs with multiple CPUs +*/
ProcMeterOutput _smp_output=
 /*+ The total cpu output +*/
 {
  /* char  name[];          */ "CPU%d_Speed",
  /* char *description;     */ "The speed of the CPU number %d in MHz.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 500,
  /* char  graph_units[];   */ "(%d)"
 };


/*+ The outputs (with single or multiple CPUs). +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];            */ "CPUInfo",
 /* char *description;      */ "CPU Information. [From /proc/cpuinfo]",
};


static float *current,*previous,*values[2]={NULL,NULL};

/*+ The number of CPUs. +*/
static int ncpus=0;


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
 int i;

 /* Verify the statistics from /proc/stat */

 f=fopen("/proc/cpuinfo","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/cpuinfo'.\n",__FILE__);
 else
   {
    if(!fgets(line,BUFFLEN,f)) /* cpu */
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/cpuinfo'.\n",__FILE__);
    else
      {
       int count;

       do
         {
          if(sscanf(line,"processor : %d",&count)==1)
             ncpus++;
         }
         while((l=fgets(line,BUFFLEN,f)));
      }

    fclose(f);
   }

 /* Create the outputs */

 outputs=(ProcMeterOutput**)malloc((ncpus+1)*sizeof(ProcMeterOutput*));
 outputs[ncpus]=NULL;

 values[0]=(float*)malloc(ncpus*sizeof(float));
 values[1]=(float*)malloc(ncpus*sizeof(float));

 current=values[0];
 previous=values[1];

 if(ncpus==1)
   {
    outputs[0]=(ProcMeterOutput*)malloc(sizeof(ProcMeterOutput));
    *outputs[0]=_output;
   }
 else
    for(i=0;i<ncpus;i++)
      {
       outputs[i]=(ProcMeterOutput*)malloc(sizeof(ProcMeterOutput));
       *outputs[i]=_smp_output;
       snprintf(outputs[i]->name,PROCMETER_NAME_LEN+1,_smp_output.name,i);
       outputs[i]->description=(char*)malloc(strlen(_smp_output.description)+8);
       sprintf(outputs[i]->description,_smp_output.description,i);
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

 /* Get the statistics from /proc/cpuinfo */

 if(now!=last)
   {
    FILE *f;
    char line[BUFFLEN],*l;
    float *temp;

    temp=current;
    current=previous;
    previous=temp;

    f=fopen("/proc/cpuinfo","r");
    if(!f)
       return(-1);

    i=0;

    while((l=fgets(line,BUFFLEN,f)))
      {
       float speed;

       if(sscanf(line,"cpu MHz : %f",&speed)==1)
         {
          current[i]=speed;
          i++;
         }
      }

    fclose(f);

    last=now;
   }

 for(i=0;i<ncpus;i++)
    if(output==outputs[i])
      {
       double value;

       value=current[i];

       output->graph_value=PROCMETER_GRAPH_FLOATING(value/output->graph_scale);
       sprintf(output->text_value,"%.1f MHz",value);

       return(0);
      }

 return(-1);
}


/*++++++++++++++++++++++++++++++++++++++
  Tidy up and prepare to have the module unloaded.
  ++++++++++++++++++++++++++++++++++++++*/

void Unload(void)
{
 int i;

 if(ncpus>1)
    for(i=0;i<ncpus;i++)
       free(outputs[i]->description);

 for(i=0;i<ncpus;i++)
    free(outputs[i]);

 free(outputs);

 free(values[0]);
 free(values[1]);
}
