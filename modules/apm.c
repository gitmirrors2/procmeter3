/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/apm.c,v 1.3 2002-06-04 13:53:39 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3b.

  Advanced Power Management module source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,2002 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The battery status output +*/
ProcMeterOutput batt_status_output=
{
 /* char  name[PROCMETER_NAME_LEN]; */ "Battery_Status",
 /* char *description;              */ "The estimated status of the battery, one of the states unknown, critical, low or high "
                                       "and whether it is currently being charged or not.",
 /* char  type;                     */ PROCMETER_TEXT,
 /* short interval;                 */ 60,
 /* char  text_value[16];           */ "unknown",
 /* long  graph_value;              */ -1,
 /* short graph_scale;              */ 0,
 /* char  graph_units[8];           */ "n/a"
};

/*+ The battery life output. +*/
ProcMeterOutput batt_life_output=
{
 /* char  name[PROCMETER_NAME_LEN]; */ "Battery_Life",
 /* char *description;              */ "The current estimated fraction of the battery life that remains.",
 /* char  type;                     */ PROCMETER_TEXT,
 /* short interval;                 */ 60,
 /* char  text_value[16];           */ "unknown",
 /* long  graph_value;              */ -1,
 /* short graph_scale;              */ 0,
 /* char  graph_units[8];           */ "n/a"
};

/*+ The battery remaining time output. +*/
ProcMeterOutput batt_remain_output=
{
 /* char  name[PROCMETER_NAME_LEN]; */ "Battery_Time",
 /* char *description;              */ "The current estimated battery lifetime remaining in minutes or seconds.",
 /* char  type;                     */ PROCMETER_TEXT,
 /* short interval;                 */ 60,
 /* char  text_value[16];           */ "unknown",
 /* long  graph_value;              */ -1,
 /* short graph_scale;              */ 0,
 /* char  graph_units[8];           */ "n/a"
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[4];

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[PROCMETER_NAME_LEN]; */ "APM",
 /* char *description;             */ "Advanced Power Management information.  These outputs are only available if you have "
                                      "configured the kernel to have the APM feature. [From /proc/apm]",
};


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

 for(n=0;n<sizeof(outputs)/sizeof(outputs[0]);n++)
    outputs[n]=NULL;

 /* Verify the statistics from /proc/apm */

 f=fopen("/proc/apm","r");
 if(!f)
    ;                           /* Don't bother giving an error message for 99% of systems. */
 else
   {
    if(!fgets(line,80,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/apm'.\n",__FILE__);
    else
      {
       long status,life,remain;
       char remainunits[8];

       if(sscanf(line,"%*f %*f %*x %*x %*x %lx %ld%% %ld %7s",&status,&life,&remain,remainunits)==4)
         {
          outputs[0]=&batt_status_output;
          outputs[1]=&batt_life_output;
          outputs[2]=&batt_remain_output;
         }
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected line in '/proc/apm'.\n",__FILE__);
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
 time_t last=0;
 static long status,life,remain;
 static char remainunits[8];

 /* Get the statistics from /proc/apm */

 if(last!=now)
   {
    FILE *f;

    f=fopen("/proc/apm","r");
    if(!f)
       return(-1);

    fscanf(f,"%*f %*f %*x %*x %*x %lx %ld%% %ld %7s",&status,&life,&remain,remainunits);

    fclose(f);

    last=now;
   }

 if(output==&batt_status_output)
   {
    if(status&0x01)             /* high */
       sprintf(output->text_value,"high");
    else if(status&0x02)        /* low */
       sprintf(output->text_value,"low");
    else if(status&0x04)        /* critical */
       sprintf(output->text_value,"critical");
    else                        /* other means unknown */
       sprintf(output->text_value,"unknown");
    if(status&0x08)             /* charging */
       strcat(output->text_value," (chg)");
    return(0);
   }
 else if(output==&batt_life_output)
   {
    if(life==-1)
       strcpy(output->text_value,"unknown");
    else
       sprintf(output->text_value,"%3ld%%",life);
    return(0);
   }
 else if(output==&batt_remain_output)
   {
    if(remain==-1)
       strcpy(output->text_value,"unknown");
    else
       sprintf(output->text_value,"%ld %s",remain,remainunits);
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
