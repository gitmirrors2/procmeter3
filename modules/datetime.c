/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/datetime.c,v 1.9 2003-06-21 18:39:25 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4a.

  Date and Time Information module source file.
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

#include <time.h>
#include <sys/time.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The current date output with year. +*/
ProcMeterOutput date_dmy_output=
{
 /* char  name[];          */ "Date_DMY",
 /* char *description;     */ "The current date in the local timezone; day of week, day of month, month, year.",
 /* char  type;            */ PROCMETER_TEXT,
 /* short interval;        */ 3600,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ -1,
 /* short graph_scale;     */ 0,
 /* char  graph_units[];   */ "n/a"
};

/*+ The current date output. +*/
ProcMeterOutput date_dm_output=
{
 /* char  name[];          */ "Date_DM",
 /* char *description;     */ "The current date in the local timezone; day of week, day of month, month.",
 /* char  type;            */ PROCMETER_TEXT,
 /* short interval;        */ 3600,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ -1,
 /* short graph_scale;     */ 0,
 /* char  graph_units[];   */ "n/a"
};

/*+ The current time output with seconds. +*/
ProcMeterOutput time_hms_output=
{
 /* char  name[];          */ "Time_HMS",
 /* char *description;     */ "The current time in the local timezone; hours, minutes and seconds.",
 /* char  type;            */ PROCMETER_TEXT,
 /* short interval;        */ 1,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ -1,
 /* short graph_scale;     */ 0,
 /* char  graph_units[];   */ "n/a"
};

/*+ The current time output with seconds, with timezone. +*/
ProcMeterOutput time_hms_tz_output=
{
 /* char  name[];          */ "Time_HMS_TZ",
 /* char *description;     */ "The current time in the local timezone; hours, minutes, seconds and timezone.",
 /* char  type;            */ PROCMETER_TEXT,
 /* short interval;        */ 1,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ -1,
 /* short graph_scale;     */ 0,
 /* char  graph_units[];   */ "n/a"
};

/*+ The current time output, without seconds. +*/
ProcMeterOutput time_hm_output=
{
 /* char  name[];          */ "Time_HM",
 /* char *description;     */ "The current time in the local timezone; hours and minutes.",
 /* char  type;            */ PROCMETER_TEXT,
 /* short interval;        */ 60,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ -1,
 /* short graph_scale;     */ 0,
 /* char  graph_units[];   */ "n/a"
};

/*+ The current time output, without seconds, with timezone. +*/
ProcMeterOutput time_hm_tz_output=
{
 /* char  name[];          */ "Time_HM_TZ",
 /* char *description;     */ "The current time in the local timezone; hours, minutes and timezone.",
 /* char  type;            */ PROCMETER_TEXT,
 /* short interval;        */ 60,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ -1,
 /* short graph_scale;     */ 0,
 /* char  graph_units[];   */ "n/a"
};

/*+ The current uptime output +*/
ProcMeterOutput uptime_dhm_output=
{
 /* char  name[];          */ "Uptime_DHM",
 /* char *description;     */ "The amount of time that the system has been booted for; days, hours and minutes.",
 /* char  type;            */ PROCMETER_TEXT,
 /* short interval;        */ 60,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ -1,
 /* short graph_scale;     */ 0,
 /* char  graph_units[];   */ "n/a"
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[]=
{
 &date_dmy_output,
 &date_dm_output,
 &time_hms_output,
 &time_hms_tz_output,
 &time_hm_output,
 &time_hm_tz_output,
 NULL,                          /* Insert uptime_dhm_output here if /proc/uptime exists. */
 NULL
};

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];           */ "Date_Time",
 /* char *description;     */ "The current date and time and the amount of time since the system was last booted.",
};


static int twelve_hour = 0;
static time_t boot_time=0;


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

 f=fopen("/proc/uptime","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/uptime'.\n",__FILE__);
 else
   {
    time_t now=time(NULL),uptime;

    if(fscanf(f,"%ld",&uptime)!=1)
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/uptime'.\n",__FILE__);
    else
      {
       boot_time=now-uptime;

       outputs[sizeof(outputs)/sizeof(outputs[0])-2]=&uptime_dhm_output;
      }

    fclose(f);
   }

 if (options && strcmp(options, "12") == 0)
    twelve_hour = 1;
 
 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform an update on one of the statistics.

  int Update Returns 0 if OK, else -1.

  time_t now The current time.

  ProcMeterOutput* output The output that the value is wanted for.
  ++++++++++++++++++++++++++++++++++++++*/

int Update(time_t now,ProcMeterOutput* output)
{
 if(output==&uptime_dhm_output)
   {
    time_t uptime=now-boot_time;
    int days =uptime/(24*3600);
    int hours=(uptime%(24*3600))/3600;
    int mins =(uptime%3600)/60;

    if(boot_time)
      {
       sprintf(output->text_value,"%dD %2dH %2dM",
               days,
               hours,
               mins);
      }
    else
       return(-1);

    return(0);
   }
 else
   {
    struct tm *tim;

    tim=localtime(&now);
    if(tim->tm_isdst<0)
       tim=gmtime(&now);

    if(output==&date_dmy_output)
      {
       strftime(output->text_value,PROCMETER_TEXT_LEN,"%a %e %b %Y",tim);
      }
    else if(output==&date_dm_output)
      {
       strftime(output->text_value,PROCMETER_TEXT_LEN,"%a %e %b",tim);
      }
    else if(output==&time_hms_output)
      {
       if(twelve_hour)
          strftime(output->text_value,PROCMETER_TEXT_LEN,"%I:%M:%S %p",tim);
       else
          strftime(output->text_value,PROCMETER_TEXT_LEN,"%H:%M:%S",tim);
      }
    else if(output==&time_hms_tz_output)
      {
       if(twelve_hour)
          strftime(output->text_value,PROCMETER_TEXT_LEN,"%I:%M:%S %p %Z",tim);
       else
          strftime(output->text_value,PROCMETER_TEXT_LEN,"%H:%M:%S %Z",tim);
      }
    else if(output==&time_hm_output)
      {
       if(twelve_hour)
          strftime(output->text_value,PROCMETER_TEXT_LEN,"%I:%M %p",tim);
       else
          strftime(output->text_value,PROCMETER_TEXT_LEN,"%H:%M",tim);
      }
    else if(output==&time_hm_tz_output)
      {
       if(twelve_hour)
          strftime(output->text_value,PROCMETER_TEXT_LEN,"%I:%M %p %Z",tim);
       else
          strftime(output->text_value,PROCMETER_TEXT_LEN,"%H:%M %Z",tim);
      }
    else
       return(-1);

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
