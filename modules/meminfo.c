/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/meminfo.c,v 1.10 2004-03-27 16:43:23 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4b.

  Memory status module source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2002,03,04 Andrew M. Bishop
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

#define MEM_TOTAL  7
#define SWAP_TOTAL 8

#define N_LINES 9

/* The interface information.  */

ProcMeterOutput _outputs[N_OUTPUTS]=
{
 /*+ The mem free output +*/
 {
  /* char  name[];          */ "Mem_Free",
  /* char *description;     */ "The amount of memory that is free, completely unused, wasted.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%dMB)"
 },
 /*+ The mem used output +*/
 {
  /* char  name[];          */ "Mem_Used",
  /* char *description;     */ "The amount of memory that is used, excluding cache and buffers.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%dMB)"
 },
 /*+ The mem buff output +*/
 {
  /* char  name[];          */ "Mem_Buff",
  /* char *description;     */ "The amount of memory that is used in buffers.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%dMB)"
 },
 /*+ The mem cache output +*/
 {
  /* char  name[];          */ "Mem_Cache",
  /* char *description;     */ "The amount of memory that is used for disk cache.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%dMB)"
 },
 /*+ The mem avail output +*/
 {
  /* char  name[];          */ "Mem_Avail",
  /* char *description;     */ "The amount of memory that is available for programs, free plus cache.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%dMB)"
 },
 /*+ The mem swap free output +*/
 {
  /* char  name[];          */ "Swap_Free",
  /* char *description;     */ "The amount of memory that is free in the swap space.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%dMB)"
 },
 /*+ The mem swap used output +*/
 {
  /* char  name[];          */ "Swap_Used",
  /* char *description;     */ "The amount of memory that is used in the swap space.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 0, /* calculated later */
  /* char  graph_units[];   */ "(%dMB)"
 }
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[N_OUTPUTS+1];

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];            */ "Memory",
 /* char *description;      */ "The amount of memory that is used for programs, buffers, cache and the amount that is free. "
                               "[From /proc/meminfo]",
};


static int proc_meminfo_V2_1_41=0;

static int contents[20];

static int available[N_LINES];


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

 for(n=0;n<=N_OUTPUTS;n++)
    outputs[n]=NULL;

 for(n=0;n<N_LINES;n++)
    available[n]=0;

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
          unsigned long long mem_tot,mem_free,mem_used,mem_buff,mem_cache,swap_tot,swap_free,swap_used;
          int i;

          proc_meminfo_V2_1_41=!strncmp(line,"MemTotal:",9);

          if(proc_meminfo_V2_1_41)
            {
             int lineno;

             for(lineno=(sizeof(contents)/sizeof(contents[0]))-1;lineno>=0;lineno--)
                contents[lineno]=0;

             lineno=1;
             do
               {
                if(sscanf(line,"MemTotal: %llu",&mem_tot)==1)
                   contents[lineno]=MEM_TOTAL,available[MEM_TOTAL]=1;
                else if(sscanf(line,"MemFree: %llu",&mem_free)==1)
                   contents[lineno]=MEM_FREE,available[MEM_FREE]=1;
                else if(sscanf(line,"Buffers: %llu",&mem_buff)==1)
                   contents[lineno]=MEM_BUFF,available[MEM_BUFF]=1;
                else if(sscanf(line,"Cached: %llu",&mem_cache)==1)
                   contents[lineno]=MEM_CACHE,available[MEM_CACHE]=1;
                else if(sscanf(line,"SwapTotal: %llu",&swap_tot)==1)
                   contents[lineno]=SWAP_TOTAL,available[SWAP_TOTAL]=1;
                else if(sscanf(line,"SwapFree: %llu",&swap_free)==1)
                   contents[lineno]=SWAP_FREE,available[SWAP_FREE]=1;
               }
             while(fgets(line,80,f) && ++lineno<(sizeof(contents)/sizeof(contents[0])));

             if(available[MEM_TOTAL] && available[MEM_FREE])
                available[MEM_USED]=1;
             if(available[SWAP_TOTAL] && available[SWAP_FREE])
                available[SWAP_USED]=1;

             if(!available[MEM_TOTAL])
                fprintf(stderr,"ProcMeter(%s): Did not find 'MemTotal' line in '/proc/meminfo'.\n",__FILE__);
             if(!available[MEM_FREE])
                fprintf(stderr,"ProcMeter(%s): Did not find 'MemFree' line in '/proc/meminfo'.\n",__FILE__);
             if(!available[MEM_BUFF])
                fprintf(stderr,"ProcMeter(%s): Did not find 'Buffers' line in '/proc/meminfo'.\n",__FILE__);
             if(!available[MEM_CACHE])
                fprintf(stderr,"ProcMeter(%s): Did not find 'Cached' line in '/proc/meminfo'.\n",__FILE__);
             if(!available[SWAP_TOTAL])
                fprintf(stderr,"ProcMeter(%s): Did not find 'SwapTotal' line in '/proc/meminfo'.\n",__FILE__);
             if(!available[SWAP_FREE])
                fprintf(stderr,"ProcMeter(%s): Did not find 'SwapFree' line in '/proc/meminfo'.\n",__FILE__);
            }
          else
            {
             fgets(line,80,f);
             if(sscanf(line,"Mem: %llu %llu %llu %*u %llu %llu",&mem_tot,&mem_free,&mem_used,&mem_buff,&mem_cache)==5)
                available[MEM_FREE]=available[MEM_USED]=available[MEM_BUFF]=available[MEM_CACHE]=1;
             else
                if(sscanf(line,"Mem: %llu %llu %llu %*u %llu",&mem_tot,&mem_free,&mem_used,&mem_buff)==4)
                   available[MEM_FREE]=available[MEM_USED]=available[MEM_BUFF]=1;
                else
                   fprintf(stderr,"ProcMeter(%s): Unexpected 'Mem' line in '/proc/meminfo'.\n",__FILE__);

             fgets(line,80,f);
             if(sscanf(line,"Swap: %llu %llu",&swap_tot,&swap_used)==2)
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

          n=0;
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
 static unsigned long long mem_free,mem_used,mem_buff,mem_cache,mem_avail,swap_free,swap_used;

 /* Get the statistics from /proc/meminfo */

 if(now!=last)
   {
    FILE *f;
    char line[80];
    unsigned long long mem_tot=0,swap_tot=0;

    f=fopen("/proc/meminfo","r");
    if(!f)
       return(-1);

    if(proc_meminfo_V2_1_41)
      {
       int lineno=0;

       while(fgets(line,80,f) && ++lineno<(sizeof(contents)/sizeof(contents[0])))
         {
          switch(contents[lineno])
            {
            case MEM_TOTAL:
             sscanf(line,"MemTotal: %llu",&mem_tot);
             break;
            case MEM_FREE:
             sscanf(line,"MemFree: %llu",&mem_free);
             break;
            case MEM_BUFF:
             sscanf(line,"Buffers: %llu",&mem_buff);
             break;
            case MEM_CACHE:
             sscanf(line,"Cached: %llu",&mem_cache);
             break;
            case SWAP_TOTAL:
             sscanf(line,"SwapTotal: %llu",&swap_tot);
             break;
            case SWAP_FREE:
             sscanf(line,"SwapFree: %llu",&swap_free);
             break;
            default:
             ;
            }
         }

       mem_used=mem_tot-mem_free;
       swap_used=swap_tot-swap_free;
      }
    else
      {
       fgets(line,80,f);
       fgets(line,80,f);
       if(available[MEM_FREE])
          sscanf(line,"Mem: %*u %llu %llu %*u %llu %llu",&mem_used,&mem_free,&mem_buff,&mem_cache);
       fgets(line,80,f);
       if(available[SWAP_FREE])
          sscanf(line,"Swap: %llu %llu",&swap_tot,&swap_used);
       swap_free=swap_tot-swap_used;

       mem_free >>=10;
       mem_used >>=10;
       mem_buff >>=10;
       mem_cache>>=10;
       swap_free>>=10;
       swap_used>>=10;
      }

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
