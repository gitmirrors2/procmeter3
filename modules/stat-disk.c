/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/stat-disk.c,v 1.2 1999-06-19 14:50:37 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.1b.

  Disk statistics source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>

#include "procmeter.h"

#define DISK        0
#define DISK_READ   1
#define DISK_WRITE  2
#define N_OUTPUTS   3

/*+ The number of disks +*/
#define NDISKS 4

/* The interface information.  */

/*+ The total disk statistics +*/
ProcMeterOutput _outputs[N_OUTPUTS]=
{
 /*+ The disk blocks accessed per second +*/
 {
  /* char  name[16];         */ "Disk",
  /* char *description;      */ "The total number of disk blocks accessed per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks read per second +*/
 {
  /* char  name[16];         */ "Disk_Read",
  /* char *description;      */ "The number of disk blocks that are read per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks write per second +*/
 {
  /* char  name[16];         */ "Disk_Write",
  /* char *description;      */ "The number of disk blocks that are written per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 }
};

/*+ The per disk statistics +*/
ProcMeterOutput _disk_outputs[N_OUTPUTS]=
{
 /*+ The disk blocks accessed per second +*/
 {
  /* char  name[16];         */ "Disk%d",
  /* char *description;      */ "The total number of disk blocks accessed per second on disk %d.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks read per second +*/
 {
  /* char  name[16];         */ "Disk%d_Read",
  /* char *description;      */ "The number of disk blocks that are read per second on disk %d.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks write per second +*/
 {
  /* char  name[16];         */ "Disk%d_Write",
  /* char *description;      */ "The number of disk blocks that are written per second on disk %d.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 }
};

/*+ The extra outputs with multiple disks +*/
ProcMeterOutput disk_outputs[N_OUTPUTS*NDISKS];

/*+ The outputs. +*/
ProcMeterOutput *outputs[N_OUTPUTS*(NDISKS+1)+1];

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];             */ "Stat-Disk",
 /* char *description;         */ "Disk usage statistics. [From /proc/stat]",
};


static unsigned long *current,*previous,values[2][N_OUTPUTS*(NDISKS+1)];


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
 int read_avail=0,write_avail=0;

 outputs[0]=NULL;

 current=values[0];
 previous=values[1];

 /* Verify the statistics from /proc/stat */

 f=fopen("/proc/stat","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/stat'.\n",__FILE__);
 else
   {
    if(!fgets(line,256,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/stat'.\n",__FILE__);
    else
      {
       fgets(line,256,f);
       while(line[0]=='c')      /* kernel version > ~2.1.84 */
          fgets(line,256,f);

       if(sscanf(line,"disk %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK],&current[N_OUTPUTS*2+DISK],&current[N_OUTPUTS*3+DISK],&current[N_OUTPUTS*4+DISK])==4)
         {
          int i,j;

          current[DISK]=current[N_OUTPUTS*1+DISK]+current[N_OUTPUTS*2+DISK]+current[N_OUTPUTS*3+DISK]+current[N_OUTPUTS*4+DISK];

          fgets(line,256,f);
          if(sscanf(line,"disk_rio %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK_READ],&current[N_OUTPUTS*2+DISK_READ],&current[N_OUTPUTS*3+DISK_READ],&current[N_OUTPUTS*4+DISK_READ])==4)
            {
             read_avail=1;
             current[DISK_READ]=current[N_OUTPUTS*1+DISK_READ]+current[N_OUTPUTS*2+DISK_READ]+current[N_OUTPUTS*3+DISK_READ]+current[N_OUTPUTS*4+DISK_READ];

             fgets(line,256,f);
             if(sscanf(line,"disk_wio %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK_WRITE],&current[N_OUTPUTS*2+DISK_WRITE],&current[N_OUTPUTS*3+DISK_WRITE],&current[N_OUTPUTS*4+DISK_WRITE])==4)
               {
                write_avail=1;
                current[DISK_WRITE]=current[N_OUTPUTS*1+DISK_WRITE]+current[N_OUTPUTS*2+DISK_WRITE]+current[N_OUTPUTS*3+DISK_WRITE]+current[N_OUTPUTS*4+DISK_WRITE];
               }
            }

          for(i=0;i<N_OUTPUTS;i++)
             for(j=0;j<NDISKS;j++)
               {
                disk_outputs[i+j*N_OUTPUTS]=_disk_outputs[i];
                sprintf(disk_outputs[i+j*N_OUTPUTS].name,_disk_outputs[i].name,j);
                disk_outputs[i+j*N_OUTPUTS].description=(char*)malloc(strlen(_disk_outputs[i].description)+8);
                sprintf(disk_outputs[i+j*N_OUTPUTS].description,_disk_outputs[i].description,j);
               }

          for(i=0;i<N_OUTPUTS;i++)
             if(i==DISK || (i==DISK_READ && read_avail) || (i==DISK_WRITE && write_avail))
                outputs[n++]=&_outputs[i];

          for(j=0;j<NDISKS;j++)
             for(i=0;i<N_OUTPUTS;i++)
                if(i==DISK || (i==DISK_READ && read_avail) || (i==DISK_WRITE && write_avail))
                   outputs[n++]=&disk_outputs[i+j*N_OUTPUTS];

          outputs[n]=NULL;
         }
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'disk' line in '/proc/stat'.\n",__FILE__);
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

    f=fopen("/proc/stat","r");
    if(!f)
       return(-1);

    fgets(line,256,f);
    fgets(line,256,f);
    while(line[0]=='c')      /* kernel version > ~2.1.84 */
       fgets(line,256,f);

    sscanf(line,"disk %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK],&current[N_OUTPUTS*2+DISK],&current[N_OUTPUTS*3+DISK],&current[N_OUTPUTS*4+DISK]);
    current[DISK]=current[N_OUTPUTS*1+DISK]+current[N_OUTPUTS*2+DISK]+current[N_OUTPUTS*3+DISK]+current[N_OUTPUTS*4+DISK];

    fgets(line,256,f);
    if(sscanf(line,"disk_rio %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK_READ],&current[N_OUTPUTS*2+DISK_READ],&current[N_OUTPUTS*3+DISK_READ],&current[N_OUTPUTS*4+DISK_READ])==4)
      {
       current[DISK_READ]=current[N_OUTPUTS*1+DISK_READ]+current[N_OUTPUTS*2+DISK_READ]+current[N_OUTPUTS*3+DISK_READ]+current[N_OUTPUTS*4+DISK_READ];

       fgets(line,256,f);
       if(sscanf(line,"disk_wio %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK_WRITE],&current[N_OUTPUTS*2+DISK_WRITE],&current[N_OUTPUTS*3+DISK_WRITE],&current[N_OUTPUTS*4+DISK_WRITE])==4)
          current[DISK_WRITE]=current[N_OUTPUTS*1+DISK_WRITE]+current[N_OUTPUTS*2+DISK_WRITE]+current[N_OUTPUTS*3+DISK_WRITE]+current[N_OUTPUTS*4+DISK_WRITE];
      }

    fclose(f);

    last=now;
   }

 for(i=0;i<(NDISKS+1)*N_OUTPUTS;i++)
    if(output==outputs[i])
      {
       double value;

       if(previous[i]>current[i])
          value=0.0;
       else
          value=(double)(current[i]-previous[i])/output->interval;

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
 int i,j;

 for(i=0;i<N_OUTPUTS;i++)
    for(j=0;j<NDISKS;j++)
       free(disk_outputs[i+j*N_OUTPUTS].description);
}
