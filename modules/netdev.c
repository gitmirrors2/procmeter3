/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/netdev.c,v 1.17 2002-12-07 19:40:25 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4.

  Network devices traffic source file.
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

/* The interface information.  */

/*+ The template for the network devices +*/
ProcMeterOutput _outputs[6]=
{
 /*+ The total packets +*/
 {
  /* char  name[];          */ "Pkt_%s",
  /* char *description;     */ "The total number of packets per second on the %s network interface.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The total bytes +*/
 {
  /* char  name[];          */ "Byte_%s",
  /* char *description;     */ "The total number of bytes per second on the %s network interface.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 kB/s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%dkB/s)"
 },
 /*+ The transmitted packets +*/
 {
  /* char  name[];          */ "Pkt_Tx_%s",
  /* char *description;     */ "The number of packets transmitted per second on the %s network interface.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The transmitted bytes +*/
 {
  /* char  name[];          */ "Byte_Tx_%s",
  /* char *description;     */ "The number of bytes transmitted per second on the %s network interface.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 kB/s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%dkB/s)"
 },
 /*+ The received packets +*/
 {
  /* char  name[];          */ "Pkt_Rx_%s",
  /* char *description;     */ "The number of packets received per second on the %s network interface.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The received bytes +*/
 {
  /* char  name[];          */ "Byte_Rx_%s",
  /* char *description;     */ "The number of bytes received per second on the %s network interface.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 kB/s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%dkB/s)"
 }
};


/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];            */ "Network",
 /* char *description;      */ "The network devices and the amount of traffic on each of them. [From /proc/net/dev]  "
                               "(Use 'options=ppp0' in the configuration file to specify extra network devices."
};

static char *proc_net_dev_format=NULL;

static char *proc_net_dev_format1="%lu %*u %*u %*u %*u %lu"; /* kernel version < ~2.1.28 */
static char *proc_net_dev_format2="%lu %lu %*u %*u %*u %*u %lu %lu"; /* ~2.1.28 < kernel version < ~2.1.80 (two possiblities) */
static char *proc_net_dev_format3="%lu %lu %*u %*u %*u %*u %*u %*u %lu %lu"; /* ~2.1.91 < kernel version */

static int ndevices=0;
static unsigned long *current=NULL,*previous=NULL;
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
 char line[256];

 outputs=(ProcMeterOutput**)malloc(sizeof(ProcMeterOutput*));
 outputs[0]=NULL;

 /* Verify the statistics from /proc/net/dev */

 f=fopen("/proc/net/dev","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/net/dev'.\n",__FILE__);
 else
   {
    if(!fgets(line,256,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/net/dev'.\n",__FILE__);
    else
       if(strcmp(line,"Inter-|   Receive                  |  Transmit\n") && /* kernel version < ~2.1.80 */
          strcmp(line,"Inter-|   Receive                           |  Transmit\n") && /* ~2.1.80 < kernel version > ~2.1.91 */
          strcmp(line,"Inter-|   Receive                                                |  Transmit\n")) /* ~2.1.91 < kernel version */
          fprintf(stderr,"ProcMeter(%s): Unexpected header line 1 in '/proc/net/dev'.\n",__FILE__);
       else
         {
          fgets(line,256,f);
          if(strcmp(line," face |packets errs drop fifo frame|packets errs drop fifo colls carrier\n") && /* kernel version < ~2.1.28 */
             strcmp(line," face |bytes    packets errs drop fifo frame|bytes    packets errs drop fifo colls carrier\n") && /* ~2.1.28 < kernel version < ~2.1.80 */
             strcmp(line," face |bytes    packets errs drop fifo frame|bytes    packets errs drop fifo colls carrier multicast\n") && /* ~2.1.80 < kernel version < ~2.1.91 */
             strcmp(line," face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n")) /* ~2.1.91 < kernel version */
             fprintf(stderr,"ProcMeter(%s): Unexpected header line 2 in '/proc/net/dev'.\n",__FILE__);
          else
            {
             if(!strcmp(line," face |packets errs drop fifo frame|packets errs drop fifo colls carrier\n"))
                proc_net_dev_format=proc_net_dev_format1; /* kernel version < ~2.1.28 */
             else if(!strcmp(line," face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n"))
                proc_net_dev_format=proc_net_dev_format3; /* ~2.1.91 < kernel version */
             else
                proc_net_dev_format=proc_net_dev_format2; /* ~2.1.28 < kernel version < ~2.1.80 (two possiblities) */

             while(fgets(line,256,f))
               {
                int i;
                char *dev=line;
                long rxp=0,txp=0,rxb=0,txb=0;

                for(;*dev==' ';dev++) ;
                for(i=strlen(line);i>6 && line[i]!=':';i--); line[i++]=0;
                if(!strcmp(&line[i]," No statistics available.\n") ||
                   (proc_net_dev_format==proc_net_dev_format1 && sscanf(&line[i],proc_net_dev_format,&rxp,&txp)==2) ||
                   (proc_net_dev_format!=proc_net_dev_format1 && sscanf(&line[i],proc_net_dev_format,&rxb,&rxp,&txb,&txp)==4))
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

 current =(long*)calloc(sizeof(long),ndevices);
 previous=(long*)calloc(sizeof(long),ndevices);

 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a new device to the list.

  char *dev The name of the device to add.
  ++++++++++++++++++++++++++++++++++++++*/

static void add_device(char *dev)
{
 int pscale,bscale,nstats;
 int i;

 for(i=0;i<ndevices;i++)
    if(!strcmp(device[i],dev))
       return;

 if(*dev=='l' || *dev=='d') /* 'lo' or 'dummy' devices. */
    pscale=100,bscale=100,nstats=1;
 else
    if(*dev=='s' || *dev=='p' || (*dev=='f' && *(dev+1)=='l') || *dev=='i')
       /* 'sl' or 'ppp'/'plip' or 'flip' or 'isdn'/'ippp' devices. */
       pscale=5,bscale=1,nstats=3;
    else /* other devices */
       pscale=50,bscale=100,nstats=3;

 if(proc_net_dev_format!=proc_net_dev_format1)
    nstats*=2;

 outputs=(ProcMeterOutput**)realloc((void*)outputs,(ndevices+nstats+1)*sizeof(ProcMeterOutput*));
 device=(char**)realloc((void*)device,(ndevices+nstats+1)*sizeof(char*));

 for(i=0;nstats;nstats--)
   {
    outputs[ndevices]=(ProcMeterOutput*)malloc(sizeof(ProcMeterOutput));
    device[ndevices]=(char*)malloc(strlen(dev)+1);

    *outputs[ndevices]=_outputs[i];
    snprintf(outputs[ndevices]->name, PROCMETER_NAME_LEN+1, _outputs[i].name, dev);
    outputs[ndevices]->description=(char*)malloc(strlen(dev)+strlen(_outputs[i].description)+4);
    sprintf(outputs[ndevices]->description,_outputs[i].description,dev);
    if(i%2)
       outputs[ndevices]->graph_scale=bscale;
    else
       outputs[ndevices]->graph_scale=pscale;

    strcpy(device[ndevices],dev);

    ndevices++;

    if(proc_net_dev_format==proc_net_dev_format1)
       i+=2;
    else
       i++;
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
    char line[256];
    long *temp;

    temp=current;
    current=previous;
    previous=temp;

    for(j=0;outputs[j];j++)
       current[j]=0;

    f=fopen("/proc/net/dev","r");
    if(!f)
       return(-1);

    fgets(line,256,f);
    fgets(line,256,f);
    while(fgets(line,256,f))
      {
       int i;
       long rxp=0,txp=0,rxb=0,txb=0;
       char *dev=line;

       for(;*dev==' ';dev++) ;
       for(i=strlen(line);i>6 && line[i]!=':';i--); line[i++]=0;
       if(proc_net_dev_format==proc_net_dev_format1)
          sscanf(&line[i],proc_net_dev_format,&rxp,&txp);
       else
          sscanf(&line[i],proc_net_dev_format,&rxb,&rxp,&txb,&txp);

       for(j=0;outputs[j];j++)
          if(!strcmp(device[j],dev))
            {
             if(proc_net_dev_format==proc_net_dev_format1 && outputs[j+1] && !strcmp(device[j+1],dev))
               {
                current[  j]=rxp+txp;
                current[++j]=txp;
                current[++j]=rxp;
               }
             else if(proc_net_dev_format!=proc_net_dev_format1 && outputs[j+2] && !strcmp(device[j+2],dev))
               {
                current[  j]=rxp+txp;
                current[++j]=rxb+txb;
                current[++j]=txp;
                current[++j]=txb;
                current[++j]=rxp;
                current[++j]=rxb;
               }
             else if(proc_net_dev_format==proc_net_dev_format1)
                current[j]=txp;
             else /* proc_net_dev_format!=proc_net_dev_format1 */
               {
                current[  j]=txp;
                current[++j]=txb;
               }
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

       if(current[j]==0) /* stopped ppp0 device */
          value=0.0;
       else if(previous[j]>current[j]) /* wrap around of 32 bit counter */
          value=(4.294967296e9-(double)(previous[j]-current[j]))/output->interval;
       else
          value=(double)(current[j]-previous[j])/output->interval;

       if(proc_net_dev_format!=proc_net_dev_format1 && j%2)
          value/=1024.0;

       output->graph_value=PROCMETER_GRAPH_FLOATING(value/output->graph_scale);
       if(proc_net_dev_format!=proc_net_dev_format1 && j%2)
          sprintf(output->text_value,"%.1f kB/s",value);
       else
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
}
