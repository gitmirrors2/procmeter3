/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/meminfo.c,v 1.5 1999-02-24 19:44:26 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.1.

  Memory status module source file.
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

#include "procmeter.h"

#define MEM_FREE  0
#define MEM_USED  1
#define MEM_BUFF  2
#define MEM_CACHE 3
#define MEM_AVAIL 4
#define SWAP_FREE 5
#define SWAP_USED 6
#define N_OUTPUTS 7

/* The interface information.  */

ProcMeterOutput _outputs[N_OUTPUTS]=
{
 /*+ The mem free output +*/
 {
  /* char  name[16];         */ "Mem_Free",
  /* char *description;      */ "The amount of memory that is free, completely unused, wasted.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "unknown",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 0, /* calculated later */
  /* char  graph_units[8];   */ "(%dMB)"
 },
 /*+ The mem used output +*/
 {
  /* char  name[16];         */ "Mem_Used",
  /* char *description;      */ "The amount of memory that is used, excluding cache and buffers.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "unknown",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 0, /* calculated later */
  /* char  graph_units[8];   */ "(%dMB)"
 },
 /*+ The mem buff output +*/
 {
  /* char  name[16];         */ "Mem_Buff",
  /* char *description;      */ "The amount of memory that is used in buffers.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "unknown",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 0, /* calculated later */
  /* char  graph_units[8];   */ "(%dMB)"
 },
 /*+ The mem cache output +*/
 {
  /* char  name[16];         */ "Mem_Cache",
  /* char *description;      */ "The amount of memory that is used for disk cache.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "unknown",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 0, /* calculated later */
  /* char  graph_units[8];   */ "(%dMB)"
 },
 /*+ The mem avail output +*/
 {
  /* char  name[16];         */ "Mem_Avail",
  /* char *description;      */ "The amount of memory that is available for programs, free plus cache.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "unknown",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 0, /* calculated later */
  /* char  graph_units[8];   */ "(%dMB)"
 },
 /*+ The mem swap free output +*/
 {
  /* char  name[16];         */ "Swap_Free",
  /* char *description;      */ "The amount of memory that is free in the swap space.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "unknown",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 0, /* calculated later */
  /* char  graph_units[8];   */ "(%dMB)"
 },
 /*+ The mem swap used output +*/
 {
  /* char  name[16];         */ "Swap_Used",
  /* char *description;      */ "The amount of memory that is used in the swap space.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "unknown",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 0, /* calculated later */
  /* char  graph_units[8];   */ "(%dMB)"
 }
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[N_OUTPUTS+1];

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];             */ "Memory",
 /* char *description;         */ "The amount of memory that is used for programs, buffers, cache and the amount that is free. "
                                  "[From /proc/meminfo]",
};


static int proc_meminfo_V2_1_41=0;

static int available[N_OUTPUTS];


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
 char line[80];
 int n;

 for(n=0;n<N_OUTPUTS;n++)
   {
    available[n]=0;
    outputs[n]=NULL;
   }
 outputs[N_OUTPUTS]=NULL;
 n=0;

 /* Verify the statistics from /proc/meminfo */

 f=fopen("/proc/meminfo","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/meminfo'.\n",__FILE__);
 else
   {
    if(!fgets(line,80,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/meminfo'.\n",__FILE__);
    else
      {
       if(strcmp(line,"        total:   used:    free:   shared:  buffers:\n") && /* kernel version < ~2.0.0 */
          strcmp(line,"        total:    used:    free:  shared: buffers:  cached:\n") && /* kernel version > ~2.0.0 */
          strncmp(line,"MemTotal:",9)) /* kernel version > ~2.1.41 */
          fprintf(stderr,"ProcMeter(%s): Unexpected first line in '/proc/meminfo'.\n",__FILE__);
       else
         {
          unsigned long mem_tot,mem_free,mem_used,mem_buff,mem_cache,swap_tot,swap_free,swap_used;
          int i;

          proc_meminfo_V2_1_41=!strncmp(line,"MemTotal:",9);

          if(proc_meminfo_V2_1_41)
            {
             sscanf(line,"MemTotal: %lu",&mem_tot);
             if(fgets(line,80,f) && sscanf(line,"MemFree: %lu",&mem_free)==1)
               available[MEM_FREE]=available[MEM_USED]=1;
             else
                fprintf(stderr,"ProcMeter(%s): Expected 'MemTotal' line in '/proc/meminfo'.\n",__FILE__);
             fgets(line,80,f); /* MemShared */
             if(fgets(line,80,f) && sscanf(line,"Buffers: %lu",&mem_buff)==1)
                available[MEM_BUFF]=1;
             else
                fprintf(stderr,"ProcMeter(%s): Expected 'Buffers' line in '/proc/meminfo'.\n",__FILE__);
             if(fgets(line,80,f) && sscanf(line,"Cached: %lu",&mem_cache)==1)
                available[MEM_CACHE]=1;
             else
                fprintf(stderr,"ProcMeter(%s): Expected 'Cached' line in '/proc/meminfo'.\n",__FILE__);
             if(fgets(line,80,f) && sscanf(line,"SwapTotal: %lu",&swap_tot)==1 &&
                fgets(line,80,f) && sscanf(line,"SwapFree: %lu",&swap_free)==1)
                available[SWAP_FREE]=available[SWAP_USED]=1;
             else
                fprintf(stderr,"ProcMeter(%s): Expected 'SwapTotal' and 'SwapFree' lines in '/proc/meminfo'.\n",__FILE__);
            }
          else
            {
             fgets(line,80,f);
             if(sscanf(line,"Mem: %lu %lu %lu %*u %lu %lu",&mem_tot,&mem_free,&mem_used,&mem_buff,&mem_cache)==5)
                available[MEM_FREE]=available[MEM_USED]=available[MEM_BUFF]=available[MEM_CACHE]=1;
             else
                if(sscanf(line,"Mem: %lu %lu %lu %*u %lu",&mem_tot,&mem_free,&mem_used,&mem_buff)==4)
                   available[MEM_FREE]=available[MEM_USED]=available[MEM_BUFF]=1;
                else
                   fprintf(stderr,"ProcMeter(%s): Unexpected 'Mem' line in '/proc/meminfo'.\n",__FILE__);

             fgets(line,80,f);
             if(sscanf(line,"Swap: %lu %lu",&swap_tot,&swap_used)==2)
                available[SWAP_FREE]=available[SWAP_USED]=1;
             else
                fprintf(stderr,"ProcMeter(%s): Unexpected 'Swap' line in '/proc/meminfo'.\n",__FILE__);

             mem_tot>>=10;
            }

          if(available[MEM_FREE] && available[MEM_CACHE])
             available[MEM_AVAIL]=1;

          if(available[MEM_FREE])
            {
             long mem_scale=1;

             mem_tot>>=14;
             while(mem_tot)
               {mem_tot>>=1; mem_scale<<=1;}

             for(i=0;i<N_OUTPUTS;i++)
                _outputs[i].graph_scale=mem_scale;
            }

          for(i=0;i<N_OUTPUTS;i++)
             if(available[i])
                outputs[n++]=&_outputs[i];
         }
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
 static unsigned long mem_free,mem_used,mem_buff,mem_cache,mem_avail,swap_free,swap_used;

 /* Get the statistics from /proc/meminfo */

 if(now!=last)
   {
    FILE *f;
    char line[80];
    unsigned long mem_tot=0,swap_tot=0;

    f=fopen("/proc/meminfo","r");
    if(!f)
       return(-1);

    if(proc_meminfo_V2_1_41)
      {
       fgets(line,80,f);
       sscanf(line,"MemTotal: %lu",&mem_tot);
       fgets(line,80,f);
       sscanf(line,"MemFree: %lu",&mem_free);
       mem_used=mem_tot-mem_free;
       fgets(line,80,f); /* MemShared */
       fgets(line,80,f);
       sscanf(line,"Buffers: %lu",&mem_buff);
       fgets(line,80,f);
       sscanf(line,"Cached: %lu",&mem_cache);
       fgets(line,80,f);
       sscanf(line,"SwapTotal: %lu",&swap_tot);
       fgets(line,80,f);
       sscanf(line,"SwapFree: %lu",&swap_free);
       swap_used=swap_tot-swap_free;
      }
    else
      {
       fgets(line,80,f);
       fgets(line,80,f);
       if(available[MEM_FREE])
          sscanf(line,"Mem: %*u %lu %lu %*u %lu %lu",&mem_used,&mem_free,&mem_buff,&mem_cache);
       fgets(line,80,f);
       if(available[SWAP_FREE])
          sscanf(line,"Swap: %lu %lu",&swap_tot,&swap_used);
       swap_free=swap_tot-swap_used;

       mem_free >>=10;
       mem_used >>=10;
       mem_buff >>=10;
       mem_cache>>=10;
       swap_free>>=10;
       swap_used>>=10;
      }

    if(available[MEM_AVAIL])
       mem_avail=mem_free+mem_cache;

    if(available[MEM_BUFF])
       mem_used-=mem_buff;
    if(available[MEM_CACHE])
       mem_used-=mem_cache;

    fclose(f);

    last=now;
   }

 if(output==&_outputs[MEM_FREE])
   {
    sprintf(output->text_value,"%.3f MB",(double)mem_free/1024.0);
    output->graph_value=PROCMETER_GRAPH_FLOATING((double)(mem_free>>10)/output->graph_scale);
    return(0);
   }
 else if(output==&_outputs[MEM_USED])
   {
    sprintf(output->text_value,"%.3f MB",(double)mem_used/1024.0);
    output->graph_value=PROCMETER_GRAPH_FLOATING((double)(mem_used>>10)/output->graph_scale);
    return(0);
   }
 else if(output==&_outputs[MEM_BUFF])
   {
    sprintf(output->text_value,"%.3f MB",(double)mem_buff/1024.0);
    output->graph_value=PROCMETER_GRAPH_FLOATING((double)(mem_buff>>10)/output->graph_scale);
    return(0);
   }
 else if(output==&_outputs[MEM_CACHE])
   {
    sprintf(output->text_value,"%.3f MB",(double)mem_cache/1024.0);
    output->graph_value=PROCMETER_GRAPH_FLOATING((double)(mem_cache>>10)/output->graph_scale);
    return(0);
   }
 else if(output==&_outputs[MEM_AVAIL])
   {
    sprintf(output->text_value,"%.3f MB",(double)mem_avail/1024.0);
    output->graph_value=PROCMETER_GRAPH_FLOATING((double)(mem_avail>>10)/output->graph_scale);
    return(0);
   }
 else if(output==&_outputs[SWAP_FREE])
   {
    sprintf(output->text_value,"%.3f MB",(double)swap_free/1024.0);
    output->graph_value=PROCMETER_GRAPH_FLOATING((double)(swap_free>>10)/output->graph_scale);
    return(0);
   }
 else if(output==&_outputs[SWAP_USED])
   {
    sprintf(output->text_value,"%.3f MB",(double)swap_used/1024.0);
    output->graph_value=PROCMETER_GRAPH_FLOATING((double)(swap_used>>10)/output->graph_scale);
    return(0);
   }

 return(-1);
}


/*++++++++++++++++++++++++++++++++++++++
  Tidy up and prepare to have the module unloaded.
  ++++++++++++++++++++++++++++++++++++++*/

void Unload(void)
{
}
