/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/df.c,v 1.3 1999-02-13 11:37:59 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.1.

  Disk capacity monitoring source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/vfs.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The template for the disk outputs +*/
ProcMeterOutput _outputs[2]=
{
 /*+ The percentage used space +*/
 {
  /* char  name[16];         */ "DF_Used_%s",
  /* char *description;      */ "The percentage of the %s device mounted on %s that is occupied with files.  "
                                "(This can exceed 100%% on UNIX format drives due to the reserved minimum free space.)",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 10,
  /* char  text_value[16];   */ "unknown",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 10,
  /* char  graph_units[8];   */ "(%d%)"
 },
 /*+ The amount of free space +*/
 {
  /* char  name[16];         */ "DF_Free_%s",
  /* char *description;      */ "The amount of space on the %s device mounted on %s that is available for non-root use.  "
                                "(This can be negative on UNIX format drives since it does not include the reserved minimum free space.)",
  /* char  type;             */ PROCMETER_TEXT,
  /* short interval;         */ 10,
  /* char  text_value[16];   */ "0 MB",
  /* long  graph_value;      */ -1,
  /* short graph_scale;      */ 0,
  /* char  graph_units[8];   */ "n/a"
 }
};


/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];             */ "DiskUsage",
 /* char *description;         */ "The fraction of the disk that is occupied and the amount of space available."
};


static int ndisks=0;
static char **disk=NULL;
static int *mounted;

static void add_disk(char *dev,char *mnt);


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

 outputs=(ProcMeterOutput**)malloc(sizeof(ProcMeterOutput*));
 outputs[0]=NULL;

 /* Use the devices in /proc/mounts */

 f=fopen("/proc/mounts","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/mounts'.\n",__FILE__);
 else
   {
    if(!fgets(line,128,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/mounts'.\n",__FILE__);
    else
       do
         {
          char device[32],mount[32];

          if(sscanf(line,"%s %s",device,mount)==2)
             if(strcmp(device,"none") && !strchr(device,':') && *mount=='/')
                add_disk(device,mount);
         }
       while(fgets(line,128,f));

    fclose(f);
   }

 /* Use the devices in /etc/fstab */

 f=fopen("/etc/fstab","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/etc/fstab'.\n",__FILE__);
 else
   {
    if(!fgets(line,128,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/etc/fstab'.\n",__FILE__);
    else
       do
         {
          char device[32],mount[32];

          if(sscanf(line,"%s %s",device,mount)==2)
             if(strcmp(device,"none") && !strchr(device,':') && *mount=='/')
                add_disk(device,mount);
         }
       while(fgets(line,128,f));

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

       add_disk("(unknown device)",l);

       *r=pr;
       while(*r && *r==' ')
          r++;

       if(!*r)
          break;

       l=r;
      }
   }

 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a new disk to the list.

  char *dev The name of the device to add.

  char *mnt The mount point for the device.
  ++++++++++++++++++++++++++++++++++++++*/

static void add_disk(char *dev,char *mnt)
{
 int i,j;

 for(i=0;i<ndisks;i++)
    if(!strcmp(disk[i],mnt))
       return;

 outputs=(ProcMeterOutput**)realloc((void*)outputs,(ndisks*2+3)*sizeof(ProcMeterOutput*));

 for(i=0,j=ndisks*2;i<2;i++,j++)
   {
    outputs[j]=(ProcMeterOutput*)malloc(sizeof(ProcMeterOutput));

    *outputs[j]=_outputs[i];
    if(strlen(mnt)>7)
      {char old=mnt[7];mnt[7]=0;sprintf(outputs[j]->name,_outputs[i].name,mnt);mnt[7]=old;}
    else
       sprintf(outputs[j]->name,_outputs[i].name,mnt);
    outputs[j]->description=(char*)malloc(strlen(dev)+strlen(mnt)+strlen(_outputs[i].description)+4);
    sprintf(outputs[j]->description,_outputs[i].description,dev,mnt);
   }

 disk=(char**)realloc((void*)disk,(ndisks+1)*sizeof(char*));
 mounted=(int*)realloc((void*)mounted,(ndisks+1)*sizeof(int));

 disk[ndisks]=(char*)malloc(strlen(mnt)+1);
 strcpy(disk[ndisks],mnt);

 ndisks++;

 outputs[j]=NULL;
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

 /* Get the mounted disks from /proc/mounts */

 if(now!=last)
   {
    FILE *f;
    char line[128];

    for(i=0;i<ndisks;i++)
       mounted[i]=0;

    f=fopen("/proc/mounts","r");
    if(!f)
       return(-1);
    else
      {
       if(!fgets(line,128,f))
          return(-1);
       else
          do
            {
             char device[32],mount[32];

             if(sscanf(line,"%s %s",device,mount)==2)
                if(strcmp(device,"none") && !strchr(device,':') && *mount=='/')
                   for(i=0;i<ndisks;i++)
                      if(!strcmp(disk[i],mount))
                         mounted[i]=1;
            }
          while(fgets(line,128,f));

       fclose(f);
      }

    last=now;
   }

 for(i=0;outputs[i];i++)
    if(output==outputs[i])
      {
       struct statfs buf;

       if(!mounted[i/2] || statfs(disk[i/2],&buf))
         {
          output->graph_value=0;
          strcpy(output->text_value,"unknown");
         }
       else
         {
          if(i%2)
            {
             long avail=(buf.f_bavail>>10)*buf.f_bsize;

             sprintf(output->text_value,"%.1f MB",avail/1024.0);
            }
          else
            {
             double frac=100.0*(double)(buf.f_blocks-buf.f_bfree)/(double)(buf.f_blocks-buf.f_bfree+buf.f_bavail);

             output->graph_value=PROCMETER_GRAPH_FLOATING(frac/output->graph_scale);
             sprintf(output->text_value,"%.1f %%",frac);
            }
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

 for(i=0;outputs[i];i++)
   {
    free(outputs[i]->description);
    free(outputs[i]);
   }

 if(outputs)
    free(outputs);
 if(ndisks)
   {
    for(i=0;i<ndisks;i++)
       free(disk[i]);
    free(disk);
    free(mounted);
   }
}
