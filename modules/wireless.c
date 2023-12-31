/***************************************
  ProcMeter - A system monitoring program for Linux - Version 3.6a.

  Wireless network devices info source file.
  ******************/ /******************
  Written by Joey Hess (with heavy borrowing from netdev.c)

  Original file Copyright 2001 Joey Hess
  Parts of this file Copyright 2001-2011,2016 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The template for the network devices +*/
ProcMeterOutput _outputs[3]=
{
 /*+ Link quality +*/
 {
  /* char  name[];          */ "Link_%s",
  /* char *description;     */ "The link quality on the %s network interface.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 10,
  /* char  graph_units[];   */ "%d"
 },
 /*+ Signal level +*/
 {
  /* char  name[];          */ "Signal_%s",
  /* char *description;     */ "The signal level on the %s network interface.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 10,
  /* char  graph_units[];   */ "(%d dBm)"
 },
 /*+ Noise level +*/
 {
  /* char  name[];          */ "Noise_%s",
  /* char *description;     */ "The noise level on the %s network interface.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 10,
  /* char  graph_units[];   */ "(%d dBm)"
 }
};


/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];            */ "Wireless",
 /* char *description;      */ "The wireless network devices and link quality data on each of them. [From /proc/net/wireless]  "
                               "(Use 'options=eth0' in the configuration file to specify extra wireless devices."
};

/* Whether there is a dot after each status int depends on whether the value
 * was updated since last read. */
static char *proc_net_wireless_format="%*i %i%*1[. ] %i%*1[. ] %i";

/* The line buffer */
static char *line=NULL;
static size_t length=0;

/* Information about the devices and the current and previous values */
static int ndevices=0;
static unsigned long *current=NULL,*previous=NULL;
static char **device=NULL;
static int nstats=3;

/* Add a new device */
static void add_device(char *dev);


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

 outputs=(ProcMeterOutput**)malloc(sizeof(ProcMeterOutput*));
 outputs[0]=NULL;

 /* Verify the statistics from /proc/net/dev */

 f=fopen("/proc/net/wireless","r");
 if(!f)
    ;                           /* Don't bother giving an error message for 99% of systems. */
 else
   {
    if(!fgets_realloc(&line,&length,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/net/wireless'.\n",__FILE__);
    else
       if(strcmp(line,"Inter-| sta-|   Quality        |   Discarded packets               | Missed\n") &&
          strcmp(line,"Inter-| sta-|   Quality        |   Discarded packets               | Missed | WE\n"))
          fprintf(stderr,"ProcMeter(%s): Unexpected header line 1 in '/proc/net/wireless'.\n",__FILE__);
       else
         {
          fgets_realloc(&line,&length,f);
          if(strncmp(line," face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon",75))
             fprintf(stderr,"ProcMeter(%s): Unexpected header line 2 in '/proc/net/wireless'.\n",__FILE__);
          else
            {
             while(fgets_realloc(&line,&length,f))
               {
                int i;
                char *dev=line;
                int link=0, level=0, noise=0;

                for(;*dev==' ';dev++);
                for(i=strlen(line);i>6 && line[i]!=':';i--);
                line[i++]=0;

                if(sscanf(&line[i],proc_net_wireless_format,&link,&level,&noise)==3)
                   add_device(dev);
               }
            }
         }

    fclose(f);
   }

 /* Get the other options */

 if(options)
   {
    char *l=options;

    while(*l && *l==' ')
       l++;

    while(*l)
      {
       char *r=l,pr;

       while(*r && *r!=' ')
          r++;

       pr=*r;
       *r=0;

       add_device(l);

       *r=pr;
       while(*r && *r==' ')
          r++;

       if(!*r)
          break;

       l=r;
      }
   }

 current =(unsigned long*)calloc(sizeof(long),ndevices);
 previous=(unsigned long*)calloc(sizeof(long),ndevices);

 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a new device to the list.

  char *dev The name of the device to add.
  ++++++++++++++++++++++++++++++++++++++*/

static void add_device(char *dev)
{
 int i;

 for(i=0;i<ndevices;i++)
    if(!strcmp(device[i],dev))
       return;

 outputs=(ProcMeterOutput**)realloc((void*)outputs,(ndevices+nstats+1)*sizeof(ProcMeterOutput*));
 device=(char**)realloc((void*)device,(ndevices+nstats+1)*sizeof(char*));

 for(i=0;i<nstats;i++)
   {
    outputs[ndevices]=(ProcMeterOutput*)malloc(sizeof(ProcMeterOutput));
    device[ndevices]=(char*)malloc(strlen(dev)+1);

    *outputs[ndevices]=_outputs[i];
    sprintf(outputs[ndevices]->name,_outputs[i].name,dev);
    outputs[ndevices]->description=(char*)malloc(strlen(dev)+strlen(_outputs[i].description)+4);
    sprintf(outputs[ndevices]->description,_outputs[i].description,dev);

    strcpy(device[ndevices],dev);

    ndevices++;
   }

 outputs[ndevices]=NULL;
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
 int j;

 /* Get the statistics from /proc/net/dev */

 if(now!=last)
   {
    FILE *f;
    unsigned long *temp;

    temp=current;
    current=previous;
    previous=temp;

    for(j=0;outputs[j];j++)
       current[j]=0;

    f=fopen("/proc/net/wireless","r");
    if(!f)
       return(-1);

    fgets_realloc(&line,&length,f);
    fgets_realloc(&line,&length,f);
    while(fgets_realloc(&line,&length,f))
      {
       int i;
       signed int link=0;
       int level=0, noise=0;
       char *dev=line;

       for(;*dev==' ';dev++);
       for(i=strlen(line);i>6 && line[i]!=':';i--);
       line[i++]=0;

       sscanf(&line[i],proc_net_wireless_format,&link,&level,&noise);

       for(j=0;outputs[j];j++)
          if(!strcmp(device[j],dev))
             switch(j%nstats)
               {
               case 0:
                current[j]=link;
                break;
               case 1:
                current[j]=level;
                break;
               case 2:
                current[j]=noise;
                break;
               default:
                ;
               }
      }

    fclose(f);

    last=now;
   }

 for(j=0;outputs[j];j++)
    if(output==outputs[j])
      {
       float val = (float)((float)(abs(current[j]))/output->graph_scale);
       output->graph_value=PROCMETER_GRAPH_FLOATING(val);

       switch(j%nstats)
         {
         case 0:
          sprintf(output->text_value,"%li",current[j]);
          break;
         case 1:
         case 2:
          sprintf(output->text_value,"%li dBm",current[j]);
          break;
         default:
          ;
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
 int i;

 if(outputs)
   {
    for(i=0;outputs[i];i++)
      {
       free(outputs[i]->description);
       free(outputs[i]);
      }
    free(outputs);
   }
 if(current)
    free(current);
 if(previous)
    free(previous);
 if(device)
   {
    for(i=0;i<ndevices;i++)
       free(device[i]);
    free(device);
   }

 if(line)
    free(line);
}
