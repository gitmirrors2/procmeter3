/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/cpuinfo.c,v 1.4 2008-05-05 18:45:17 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.5b.

  CPU information.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998-2008, 2016, 2017 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "procmeter.h"

#define CPU_SPEED   0
#define NOUTPUTS    1

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

/*+ The module if using /proc/cpuinfo. +*/
ProcMeterModule _module_cpuinfo=
{
 /* char name[];            */ "CPUInfo",
 /* char *description;      */ "CPU Information. [From /proc/cpuinfo]",
};

/*+ The module if using /sys/devices/system/cpu/cpufreq. +*/
ProcMeterModule _module_cpufreq=
{
 /* char name[];            */ "CPUInfo",
 /* char *description;      */ "CPU Information. [From /sys/devices/system/cpu/cpufreq]",
};

/*+ The module. +*/
ProcMeterModule *module=NULL;


/* The line buffer */
static char *line=NULL;
static size_t length=0;

/* The current and previous information */
static float *current,*previous,*values[2]={NULL,NULL};

/*+ The number of CPUs. +*/
static int ncpus=0;


/*++++++++++++++++++++++++++++++++++++++
  Load the module.

  ProcMeterModule *Load Returns the module information.
  ++++++++++++++++++++++++++++++++++++++*/

ProcMeterModule *Load(void)
{
 FILE *f;

 f=fopen("/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_cur_freq","r");
 if(f)
   {
    module=&_module_cpufreq;
    fclose(f);
   }
 else
    module=&_module_cpufreq;

 return(module);
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise the module, creating the outputs as required.

  ProcMeterOutput **Initialise Returns a NULL terminated list of outputs.

  char *options The options string for the module from the .procmeterrc file.
  ++++++++++++++++++++++++++++++++++++++*/

ProcMeterOutput **Initialise(char *options)
{
 FILE *f;
 int i;

 if(module==&_module_cpuinfo)
   {
    /* Verify the statistics from /proc/cpuinfo */

    f=fopen("/proc/cpuinfo","r");
    if(!f)
       fprintf(stderr,"ProcMeter(%s): Could not open '/proc/cpuinfo'.\n",__FILE__);
    else
      {
       if(!fgets_realloc(&line,&length,f)) /* cpu */
          fprintf(stderr,"ProcMeter(%s): Could not read '/proc/cpuinfo'.\n",__FILE__);
       else
         {
          int nspeeds=0;

          do
            {
             int count;
             float speed;

             if(sscanf(line,"processor : %d",&count)==1)
                ncpus++;

             if(sscanf(line,"cpu MHz : %f",&speed)==1)
                nspeeds++;
            }
          while(fgets_realloc(&line,&length,f));

          if(nspeeds!=ncpus)
             ncpus=0;
         }

       fclose(f);
      }
   }
 else
   {
    struct dirent *ent;
    DIR *dir;

    /* Verify the statistics from /sys/devices/system/cpu/cpufreq/ */

    dir=opendir("/sys/devices/system/cpu/cpufreq/");
    if(!dir)
      {
       fprintf(stderr,"ProcMeter(%s): Could not open '/sys/devices/system/cpu/cpufreq/'.\n",__FILE__);
       return(NULL);
      }

    while((ent = readdir(dir)) != NULL)
      {
       if(!strncmp(ent->d_name,"policy",6))
         {
          char filename[64+NAME_MAX];

          sprintf(filename,"/sys/devices/system/cpu/cpufreq/%s/affected_cpus",ent->d_name);

          f=fopen(filename,"r");
          if(!f)
             fprintf(stderr,"ProcMeter(%s): Could not open '%s'.\n",__FILE__,filename);
          else
            {
             unsigned cpu;

             while(fscanf(f,"%u",&cpu)==1)
                if(cpu>=ncpus)
                   ncpus=cpu+1;

             fclose(f);
            }
         }
      }

    closedir(dir);
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

 if(now!=last)
   {
    float *temp;

    temp=current;
    current=previous;
    previous=temp;

    if(module==&_module_cpuinfo)
      {
       FILE *f;

       /* Get the statistics from /proc/cpuinfo */

       f=fopen("/proc/cpuinfo","r");
       if(!f)
          return(-1);

       i=0;

       while(fgets_realloc(&line,&length,f))
         {
          float speed;

          if(sscanf(line,"cpu MHz : %f",&speed)==1)
            {
             current[i]=speed;
             i++;
            }
         }

       fclose(f);
      }
    else
      {
       struct dirent *ent;
       DIR *dir;
       FILE *f;

       /* Get the statistics from /sys/devices/system/cpu/cpufreq/ */

       dir=opendir("/sys/devices/system/cpu/cpufreq/");
       if(!dir)
         {
          fprintf(stderr,"ProcMeter(%s): Could not open '/sys/devices/system/cpu/cpufreq/'.\n",__FILE__);
          return(-1);
         }

       i=0;

       while((ent = readdir(dir)) != NULL)
         {
          if(!strncmp(ent->d_name,"policy",6))
            {
             char filename[64+NAME_MAX];
             float speed;

             sprintf(filename,"/sys/devices/system/cpu/cpufreq/%s/scaling_cur_freq",ent->d_name);

             f=fopen(filename,"r");
             if(f)
               {
                fscanf(f,"%f",&speed);

                fclose(f);
               }

             sprintf(filename,"/sys/devices/system/cpu/cpufreq/%s/affected_cpus",ent->d_name);

             f=fopen(filename,"r");
             if(f)
               {
                unsigned cpu;

                while(fscanf(f,"%u",&cpu)==1)
                   if(cpu<ncpus)
                      current[cpu]=speed/1000;

                fclose(f);
               }
            }
         }

       closedir(dir);
      }

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

 if(line)
    free(line);
}
