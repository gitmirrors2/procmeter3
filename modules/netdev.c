/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/netdev.c,v 1.1 1998-09-19 15:25:34 amb Exp $

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
#include <string.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The template for the network devices +*/
ProcMeterOutput _outputs[3]=
{
 /*+ The total packets +*/
 {
  /* char  name[16];         */ "Pkt_%s",
  /* char *description;      */ "The total number of packets per second on the %s network interface.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 0,
  /* char  graph_units[8];   */ "(%d)"
 },
 /*+ The transmitted packets +*/
 {
  /* char  name[16];         */ "Pkt_Tx_%s",
  /* char *description;      */ "The number of packets transmitted per second on the %s network interface.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 0,
  /* char  graph_units[8];   */ "(%d)"
 },
 /*+ The received packets +*/
 {
  /* char  name[16];         */ "Pkt_Rx_%s",
  /* char *description;      */ "The number of packets received per second on the %s network interface.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 0,
  /* char  graph_units[8];   */ "(%d)"
 }
};


/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];             */ "Network",
 /* char *description;         */ "The network devices and the amount of traffic on each of them. [From /proc/net/dev]  "
                                  "(Use 'options=ppp0' in the configuration file to specify extra network devices."
};

static char *proc_net_dev_format=NULL;

static int ndevices=0;
static long *current=NULL,*previous=NULL;
static char **device=NULL;

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
 char line[128];

 /* Verify the statistics from /proc/net/dev */

 f=fopen("/proc/net/dev","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/net/dev'.\n",__FILE__);
 else
   {
    if(!fgets(line,128,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/net/dev'.\n",__FILE__);
    else
       if(strcmp(line,"Inter-|   Receive                  |  Transmit\n") && /* kernel version < ~2.1.80 */
          strcmp(line,"Inter-|   Receive                           |  Transmit\n") && /* ~2.1.80 < kernel version > ~2.1.91 */
          strcmp(line,"Inter-|   Receive                                                |  Transmit\n")) /* ~2.1.91 < kernel version */
          fprintf(stderr,"ProcMeter(%s): Unexpected header line 1 in '/proc/net/dev'.\n",__FILE__);
       else
         {
          fgets(line,128,f);
          if(strcmp(line," face |packets errs drop fifo frame|packets errs drop fifo colls carrier\n") && /* kernel version < ~2.1.28 */
             strcmp(line," face |bytes    packets errs drop fifo frame|bytes    packets errs drop fifo colls carrier\n") && /* ~2.1.28 < kernel version < ~2.1.80 */
             strcmp(line," face |bytes    packets errs drop fifo frame|bytes    packets errs drop fifo colls carrier multicast\n") && /* ~2.1.80 < kernel version < ~2.1.91 */
             strcmp(line," face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n")) /* ~2.1.91 < kernel version */
             fprintf(stderr,"ProcMeter(%s): Unexpected header line 2 in '/proc/net/dev'.\n",__FILE__);
          else
            {
             if(!strcmp(line," face |packets errs drop fifo frame|packets errs drop fifo colls carrier\n"))
                proc_net_dev_format="%lu %*u %*u %*u %*u %lu"; /* kernel version < ~2.1.28 */
             else if(!strcmp(line," face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n"))
                proc_net_dev_format="%*u %lu %*u %*u %*u %*u %*u %*u %*u %lu"; /* ~2.1.91 < kernel version */
             else
                proc_net_dev_format="%*u %lu %*u %*u %*u %*u %*u %lu"; /* ~2.1.28 < kernel version < ~2.1.80 (two possiblities) */

             while(fgets(line,128,f))
               {
                int i;
                char *dev=line;
                long rx=0,tx=0;

                for(;*dev==' ';dev++) ;
                for(i=strlen(line);i>6 && line[i]!=':';i--); line[i++]=0;
                if(sscanf(&line[i],proc_net_dev_format,&rx,&tx)==2 || !strcmp(&line[i]," No statistics available.\n"))
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
       char *r=l;

       while(*r && *r!=' ')
          r++;

       if(*r==' ')
          *r=0;

       add_device(l);

       *r=' ';
       while(*r && *r==' ')
          r++;

       if(!*r)
          break;

       l=r;
      }
   }

 current =(long*)malloc(sizeof(long)*ndevices);
 previous=(long*)malloc(sizeof(long)*ndevices);

 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a new device to the list.

  char *dev The name of the device to add.
  ++++++++++++++++++++++++++++++++++++++*/

static void add_device(char *dev)
{
 int scale,nstats;
 int i;

 if(*dev=='l' || *dev=='d') /* 'lo' or 'dummy' devices. */
    scale=100,nstats=1;
 else
    if(*dev=='s' || *dev=='p' || (*dev=='f' && *(dev+1)=='l') || *dev=='i')
       /* 'sl' or 'ppp'/'plip' or 'flip' or 'isdn'/'ippp' devices. */
       scale=5,nstats=3;
    else /* other devices */
       scale=50,nstats=3;

 outputs=(ProcMeterOutput**)realloc((void*)outputs,(ndevices+nstats+1)*sizeof(ProcMeterOutput*));
 device=(char **)realloc((void*)device,(ndevices+nstats+1)*sizeof(char*));
 for(i=0;nstats;i++,nstats--)
   {
    outputs[ndevices]=(ProcMeterOutput*)malloc(sizeof(ProcMeterOutput));
    device[ndevices]=(char*)malloc(strlen(dev)+1);
    strcpy(device[ndevices],dev);
    *outputs[ndevices]=_outputs[i];
    sprintf(outputs[ndevices]->name,_outputs[i].name,dev);
    outputs[ndevices]->description=(char*)malloc(strlen(dev)+strlen(_outputs[i].description)+4);
    sprintf(outputs[ndevices]->description,_outputs[i].description,dev);
    outputs[ndevices]->graph_scale=scale;
    sprintf(outputs[ndevices]->graph_units,_outputs[i].graph_units,scale);
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
    char line[128];
    long *temp;

    temp=current;
    current=previous;
    previous=temp;

    f=fopen("/proc/net/dev","r");
    if(!f)
       return(-1);

    fgets(line,128,f);
    fgets(line,128,f);
    while(fgets(line,128,f))
      {
       int i;
       long rx=0,tx=0;
       char *dev=line;

       for(;*dev==' ';dev++) ;
       for(i=strlen(line);i>6 && line[i]!=':';i--); line[i++]=0;
       sscanf(&line[i],proc_net_dev_format,&rx,&tx);

       for(j=0;outputs[j];j++)
          if(!strcmp(device[j],dev))
            {
             if(outputs[j+1] && !strcmp(device[j+1],dev))
               {
                current[j]=rx+tx;
                current[++j]=tx;
                if(outputs[j+1] && !strcmp(device[j+1],dev))
                   current[++j]=rx;
               }
             else
                current[j]=tx;
             break;
            }
      }

    fclose(f);

    last=now;
   }

 for(j=0;outputs[j];j++)
    if(output==outputs[j])
      {
       double value;

       value=(double)(current[j]-previous[j])/output->interval;

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
 int i;

 for(i=0;outputs[i];i++)
   {
    free(outputs[i]->description);
    free(outputs[i]);
   }

 if(outputs)
    free(outputs);
 if(current)
    free(current);
 if(previous)
    free(previous);
 if(device)
    free(device);
}
