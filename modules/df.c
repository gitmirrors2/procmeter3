/***************************************
  ProcMeter - A system monitoring program for Linux - Version 3.6a.

  Disk capacity monitoring source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998-2012 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#define _FILE_OFFSET_BITS 64    /* Force statfs to use 64-bit values */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <sys/vfs.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The template for the disk outputs +*/
ProcMeterOutput _outputs[2]=
{
 /*+ The percentage used space +*/
 {
  /* char  name[];         */ "DF_Used_%s",
  /* char *description;    */ "The percentage of the %s device mounted on %s that is occupied with files.  "
                              "(This can exceed 100%% on UNIX format drives due to the reserved minimum free space.)",
  /* char  type;           */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;       */ 10,
  /* char  text_value[];   */ "unknown",
  /* long  graph_value;    */ 0,
  /* short graph_scale;    */ 20,
  /* char  graph_units[];  */ "(%d%%)"
 },
 /*+ The amount of free space +*/
 {
  /* char  name[];         */ "DF_Free_%s",
  /* char *description;    */ "The amount of space on the %s device mounted on %s that is available for non-root use.  "
                              "(This can be negative on UNIX format drives since it does not include the reserved minimum free space.)",
  /* char  type;           */ PROCMETER_TEXT,
  /* short interval;       */ 10,
  /* char  text_value[];   */ "0 MB",
  /* long  graph_value;    */ -1,
  /* short graph_scale;    */ 0,
  /* char  graph_units[];  */ "n/a"
 }
};


/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];           */ "DiskUsage",
 /* char *description;     */ "The fraction of the disk that is occupied and the amount of space available."
};


/* The line buffer */
static char *line=NULL;
static size_t length=128;

/* The information about the disks */
static int ndisks=0;
static char **disk=NULL;
static int *mounted;

/* A function to add a disk */
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

 outputs=(ProcMeterOutput**)malloc(sizeof(ProcMeterOutput*));
 outputs[0]=NULL;

 /* Use the devices in /proc/mounts */

 f=fopen("/proc/mounts","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/mounts'.\n",__FILE__);
 else
   {
    if(!fgets_realloc(&line,&length,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/mounts'.\n",__FILE__);
    else
       do
         {
          char device[65],mount[65];

          if(sscanf(line,"%64s %64s",device,mount)==2)
             if(strcmp(mount,"none") && *mount=='/' && (*device=='/' || strstr(device, ":/")))
                add_disk(device,mount);
         }
       while(fgets_realloc(&line,&length,f));

    fclose(f);
   }

 /* Use the devices in /etc/fstab */

 f=fopen("/etc/fstab","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/etc/fstab'.\n",__FILE__);
 else
   {
    if(!fgets_realloc(&line,&length,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/etc/fstab'.\n",__FILE__);
    else
       do
         {
          char device[65],mount[65];

          if(*line=='#')
             continue;

          if(sscanf(line,"%64s %64s",device,mount)==2)
             if(strcmp(mount,"none") && *mount=='/' && (*device=='/' || strstr(device, ":/")))
                add_disk(device,mount);
         }
       while(fgets_realloc(&line,&length,f));

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
    snprintf(outputs[j]->name,PROCMETER_NAME_LEN+1,_outputs[i].name,mnt);
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

    for(i=0;i<ndisks;i++)
       mounted[i]=0;

    f=fopen("/proc/mounts","r");
    if(!f)
       return(-1);
    else
      {
       if(!fgets_realloc(&line,&length,f))
          return(-1);
       else
          do
            {
             char device[65],mount[65];

             if(sscanf(line,"%64s %64s",device,mount)==2)
                if(strcmp(mount,"none") && *mount=='/' && (*device=='/' || strstr(device, ":/")))
                   for(i=0;i<ndisks;i++)
                      if(!strcmp(disk[i],mount))
                         mounted[i]=1;
            }
          while(fgets_realloc(&line,&length,f));

       fclose(f);
      }

    last=now;
   }

 for(i=0;outputs[i];i++)
    if(output==outputs[i])
      {
       struct statfs buf;

       if(!mounted[i/2])
         {
          output->graph_value=0;
          strcpy(output->text_value,"not found");
         }
       else if(statfs(disk[i/2],&buf))
         {
          output->graph_value=0;

          if(errno == EOVERFLOW)
             strcpy(output->text_value,"statfs overflow");
          else
             strcpy(output->text_value,"statfs error");
         }
       else
         {
          if(i%2)
            {
             long long avail=(buf.f_bavail>>5)*(buf.f_bsize>>5);
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

 if(outputs)
   {
    for(i=0;outputs[i];i++)
      {
       free(outputs[i]->description);
       free(outputs[i]);
      }
    free(outputs);
   }

 if(ndisks)
   {
    for(i=0;i<ndisks;i++)
       free(disk[i]);
    free(disk);
    free(mounted);
   }

 if(line)
    free(line);
}
