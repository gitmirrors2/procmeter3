/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/sensors.c,v 1.6 2002-12-07 19:40:25 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4.

  Temperature indicators for Mainboard and CPU
  Based on loadavg.c, stat-cpu.c by Andrew M. Bishop
  ******************/ /******************
  Written by Matt Kemner, Andrew M. Bishop

  This file Copyright 1999 Matt Kemner, Andrew M. Bishop
  parts of it are Copyright 1998,99,2002 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The mainboard temperature output. +*/
ProcMeterOutput _temp_output=
{
 /* char  name[];          */ "Temp%d",
 /* char *description;     */ "Temperature sensor number %d [from %s].",
 /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
 /* short interval;        */ 1,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ 0,
 /* short graph_scale;     */ 10,
 /* char  graph_units[];   */ "(%d C)"
};

/*+ The mainboard fan output. +*/
ProcMeterOutput _fan_output=
{
 /* char  name[];          */ "Fan%d",
 /* char *description;     */ "Fan speed sensor number %d [from %s].",
 /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
 /* short interval;        */ 1,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ 0,
 /* short graph_scale;     */ 1000,
 /* char  graph_units[];   */ "(%d rpm)"
};

/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];           */ "Sensors",
 /* char *description;     */ "Hardware status information, temperature, fan speed etc. [From /proc/sys/dev/sensors/*/*]"
                              " (Requires version 2.x.x of lm78 sensors from http://www.netroedge.com/~lm78/).",
};

/*+ The temperature outputs. +*/
ProcMeterOutput *temp_outputs=NULL;

/*+ The fan outputs. +*/
ProcMeterOutput *fan_outputs=NULL;

/*+ The names of the files containing the temperature information. +*/
static char **temp_filename=NULL;

/*+ The names of the files containing the fan speed information. +*/
static char **fan_filename=NULL;

/*+ The number of temperature sensors. +*/
static int ntemps=0;

/*+ The number of fan speed sensors. +*/
static int nfans=0;

static void add_temperature(char *filename);
static void add_fan(char *filename);


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
 DIR *d1,*d2;
 struct dirent* ent1,*ent2;
 struct stat buf;
 int n=0,i;

 /* Find the directory with the sensor information in it. */

 d1=opendir("/proc/sys/dev/sensors");
 if(!d1)
    ;                           /* Don't give an error message for most users. */
 else
   {
    char dirname[64];

    while((ent1=readdir(d1)))
      {
       if(!strcmp(ent1->d_name,"."))
          continue;
       if(!strcmp(ent1->d_name,".."))
          continue;

       sprintf(dirname,"/proc/sys/dev/sensors/%s",ent1->d_name);
       if(stat(dirname,&buf)==0 && S_ISDIR(buf.st_mode))
         {
          d2=opendir(dirname);
          if(!d2)
             fprintf(stderr,"ProcMeter(%s): The directory '%s' exists but cannot be read.\n",__FILE__,dirname);
          else
            {
             while((ent2=readdir(d2)))
               {
                char filename[80];

                if(!strcmp(ent2->d_name,"."))
                   continue;
                if(!strcmp(ent2->d_name,".."))
                   continue;

                sprintf(filename,"%s/%s",dirname,ent2->d_name);
                if(stat(filename,&buf)!=0 || !S_ISREG(buf.st_mode))
                   continue;

                if(!strcmp(ent2->d_name,"temp"))
                   add_temperature(filename);
                else if(!strcmp(ent2->d_name,"remote_temp"))
                   add_temperature(filename);
                else if(!strncmp(ent2->d_name,"temp",4) && isdigit(ent2->d_name[4]) && !ent2->d_name[5])
                   add_temperature(filename);
                else if(!strncmp(ent2->d_name,"fan",3) && isdigit(ent2->d_name[3]) && !ent2->d_name[4])
                   add_fan(filename);
               }

             closedir(d2);
            }
         }
      }

    closedir(d1);
   }

 outputs=(ProcMeterOutput**)malloc((1+ntemps+nfans)*sizeof(ProcMeterOutput*));
 for(i=0;i<ntemps;i++,n++)
    outputs[n]=&temp_outputs[i];
 for(i=0;i<nfans;i++,n++)
    outputs[n]=&fan_outputs[i];
 outputs[n]=NULL;

 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a temperature output to the module.

  char *filename The name of the file.
  ++++++++++++++++++++++++++++++++++++++*/

static void add_temperature(char *filename)
{
 FILE *f;
 char line[80];

 f=fopen(filename,"r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '%s'.\n",__FILE__,filename);
 else
   {
    if(!fgets(line,80,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '%s'.\n",__FILE__,filename);
    else
      {
       double t1,t2,t3;

       if(sscanf(line,"%lf %lf %lf",&t1,&t2,&t3)!=3)
          fprintf(stderr,"ProcMeter(%s): Unexpected line in '%s'.\n",__FILE__,filename);
       else
         {
          temp_filename=(char**)realloc((void*)temp_filename,(ntemps+1)*sizeof(char*));
          temp_filename[ntemps]=(char*)malloc(strlen(filename)+1);
          strcpy(temp_filename[ntemps],filename);

          temp_outputs=(ProcMeterOutput*)realloc((void*)temp_outputs,(ntemps+1)*sizeof(ProcMeterOutput));
          temp_outputs[ntemps]=_temp_output;
          snprintf(temp_outputs[ntemps].name, PROCMETER_NAME_LEN+1, _temp_output.name, ntemps);
          temp_outputs[ntemps].description=(char*)malloc(strlen(_temp_output.description)+8+strlen(filename));
          sprintf(temp_outputs[ntemps].description,_temp_output.description,ntemps,filename);
          ntemps++;
         }
      }

    fclose(f);
   }
}

/*++++++++++++++++++++++++++++++++++++++
  Add a fan speed output to the module.

  char *filename The name of the file.
  ++++++++++++++++++++++++++++++++++++++*/

static void add_fan(char *filename)
{
 FILE *f;
 char line[80];

 f=fopen(filename,"r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '%s'.\n",__FILE__,filename);
 else
   {
    if(!fgets(line,80,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '%s'.\n",__FILE__,filename);
    else
      {
       int f1,f2;

       if(sscanf(line,"%d %d",&f1,&f2)!=2)
          fprintf(stderr,"ProcMeter(%s): Unexpected line in '%s'.\n",__FILE__,filename);
       else
         {
          fan_filename=(char**)realloc((void*)fan_filename,(nfans+1)*sizeof(char*));
          fan_filename[nfans]=(char*)malloc(strlen(filename)+1);
          strcpy(fan_filename[nfans],filename);

          fan_outputs=(ProcMeterOutput*)realloc((void*)fan_outputs,(nfans+1)*sizeof(ProcMeterOutput));
          fan_outputs[nfans]=_fan_output;
          snprintf(fan_outputs[nfans].name, PROCMETER_NAME_LEN, _fan_output.name, nfans);
          fan_outputs[nfans].description=(char*)malloc(strlen(_fan_output.description)+8+strlen(filename));
          sprintf(fan_outputs[nfans].description,_fan_output.description,nfans,filename);
          nfans++;
         }
      }

    fclose(f);
   }
}

/*++++++++++++++++++++++++++++++++++++++
  Perform an update on one of the statistics.

  int Update Returns 0 if OK, else -1.

  time_t now The current time.

  ProcMeterOutput *output The output that the value is wanted for.
  ++++++++++++++++++++++++++++++++++++++*/

int Update(time_t now,ProcMeterOutput *output)
{
 int i;

 /* Get the statistics from /proc/sys/dev/sensors/.../temp* */

 for(i=0;i<ntemps;i++)
   {
    if(output==&temp_outputs[i])
      {
       FILE *f;
       double temp;

       f=fopen(temp_filename[i],"r");
       if(!f)
          return(-1);

       if(fscanf(f,"%*f %*f %lf",&temp)!=1)
          return(-1);
       
       fclose(f);

       sprintf(output->text_value,"%.1f C",temp);
       output->graph_value=PROCMETER_GRAPH_FLOATING(temp/output->graph_scale);
       return(0);
      }
   }

 /* Get the statistics from /proc/sys/dev/sensors/.../fan* */

 for(i=0;i<nfans;i++)
   {
    if(output==&fan_outputs[i])
      {
       FILE *f;
       int fan;

       f=fopen(fan_filename[i],"r");
       if(!f)
          return(-1);

       if(fscanf(f,"%*d %d",&fan)!=1)
          return(-1);
       
       fclose(f);

       sprintf(output->text_value,"%d rpm",fan);
       output->graph_value=PROCMETER_GRAPH_FLOATING((double)fan/output->graph_scale);
       return(0);
      }
   }

 return(-1);
}


/*++++++++++++++++++++++++++++++++++++++
  Tidy up and prepare to have the module unloaded.
  ++++++++++++++++++++++++++++++++++++++*/

void Unload(void)
{
 int i;

 for(i=0;i<ntemps;i++)
    free(temp_filename[i]);
 if(temp_filename)
    free(temp_filename);
 for(i=0;i<ntemps;i++)
    free(temp_outputs[i].description);
 if(temp_outputs)
    free(temp_outputs);

 for(i=0;i<nfans;i++)
    free(fan_filename[i]);
 if(fan_filename)
    free(fan_filename);
 for(i=0;i<nfans;i++)
    free(fan_outputs[i].description);
 if(fan_outputs)
    free(fan_outputs);

 free(outputs);
}
