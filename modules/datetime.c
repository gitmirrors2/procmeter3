/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/datetime.c,v 1.6 2002-06-04 13:54:06 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3b.

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

#include <time.h>
#include <sys/time.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The current date output with year. +*/
ProcMeterOutput date_dmy_output=
{
 /* char  name[PROCMETER_NAME_LEN]; */ "Date_DMY",
 /* char *description;              */ "The current date in the local timezone; day of week, day of month, month, year.",
 /* char  type;                     */ PROCMETER_TEXT,
 /* short interval;                 */ 3600,
 /* char  text_value[16];           */ "unknown",
 /* long  graph_value;              */ -1,
 /* short graph_scale;              */ 0,
 /* char  graph_units[8];           */ "n/a"
};

/*+ The current date output. +*/
ProcMeterOutput date_dm_output=
{
 /* char  name[PROCMETER_NAME_LEN]; */ "Date_DM",
 /* char *description;              */ "The current date in the local timezone; day of week, day of month, month.",
 /* char  type;                     */ PROCMETER_TEXT,
 /* short interval;                 */ 3600,
 /* char  text_value[16];           */ "unknown",
 /* long  graph_value;              */ -1,
 /* short graph_scale;              */ 0,
 /* char  graph_units[8];           */ "n/a"
};

/*+ The current time output with seconds. +*/
ProcMeterOutput time_hms_output=
{
 /* char  name[PROCMETER_NAME_LEN]; */ "Time_HMS",
 /* char *description;              */ "The current time in the local timezone; hours, minutes, seconds and timezone.",
 /* char  type;                     */ PROCMETER_TEXT,
 /* short interval;                 */ 1,
 /* char  text_value[16];           */ "unknown",
 /* long  graph_value;              */ -1,
 /* short graph_scale;              */ 0,
 /* char  graph_units[8];           */ "n/a"
};

/*+ The current time output. +*/
ProcMeterOutput time_hm_output=
{
 /* char  name[PROCMETER_NAME_LEN]; */ "Time_HM",
 /* char *description;              */ "The current time in the local timezone; hours, minutes and timezone.",
 /* char  type;                     */ PROCMETER_TEXT,
 /* short interval;                 */ 60,
 /* char  text_value[16];           */ "unknown",
 /* long  graph_value;              */ -1,
 /* short graph_scale;              */ 0,
 /* char  graph_units[8];           */ "n/a"
};

/*+ The current uptime output +*/
ProcMeterOutput uptime_dhm_output=
{
 /* char  name[PROCMETER_NAME_LEN]; */ "Uptime_DHM",
 /* char *description;              */ "The amount of time that the system has been booted for; days, hours and minutes.",
 /* char  type;                     */ PROCMETER_TEXT,
 /* short interval;                 */ 60,
 /* char  text_value[16];           */ "unknown",
 /* long  graph_value;              */ -1,
 /* short graph_scale;              */ 0,
 /* char  graph_units[8];           */ "n/a"
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[]=
{
 &date_dmy_output,
 &date_dm_output,
 &time_hms_output,
 &time_hm_output,
 NULL,                          /* Insert uptime_dhm_output here if /proc/uptime exists. */
 NULL
};

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[PROCMETER_NAME_LEN]; */ "Date_Time",
 /* char *description;             */ "The current date and time and the amount of time since the system was last booted.",
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
 if(output==&date_dmy_output)
   {
    static char *week[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    static char *month[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    struct tm *tim;

    tim=localtime(&now);
    if(tim->tm_isdst<0)
       tim=gmtime(&now);

    sprintf(output->text_value,"%3s %02d %3s %4d",
            week[tim->tm_wday],
            tim->tm_mday,
            month[tim->tm_mon],
            tim->tm_year+1900);

    return(0);
   }
 if(output==&date_dm_output)
   {
    static char *week[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    static char *month[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    struct tm *tim;

    tim=localtime(&now);
    if(tim->tm_isdst<0)
       tim=gmtime(&now);

    sprintf(output->text_value,"%3s %02d %3s",
            week[tim->tm_wday],
            tim->tm_mday,
            month[tim->tm_mon]);

    return(0);
   }
 else if(output==&time_hms_output)
   {
    struct tm *tim;
    int utc=0;

    tim=localtime(&now);
    if(tim->tm_isdst<0)
      {tim=gmtime(&now);utc=1;}

    if (twelve_hour && tim->tm_hour > 12)
      tim->tm_hour-=12;
    else if (twelve_hour && tim->tm_hour == 0)
      tim->tm_hour=12;
    
    sprintf(output->text_value,"%02d:%02d:%02d %s",
            tim->tm_hour,
            tim->tm_min,
            tim->tm_sec,
            utc?"GMT":tzname[tim->tm_isdst>0]);

    return(0);
   }
 else if(output==&time_hm_output)
   {
    struct tm *tim;
    int utc=0;

    tim=localtime(&now);
    if(tim->tm_isdst<0)
      {tim=gmtime(&now);utc=1;}

    if (twelve_hour && tim->tm_hour > 12)
      tim->tm_hour-=12;
    else if (twelve_hour && tim->tm_hour == 0)
      tim->tm_hour=12;
    
    sprintf(output->text_value,"%02d:%02d %s",
            tim->tm_hour,
            tim->tm_min,
            utc?"GMT":tzname[tim->tm_isdst>0]);

    return(0);
   }
 else if(output==&uptime_dhm_output && boot_time)
   {
    time_t uptime=now-boot_time;
    int days =uptime/(24*3600);
    int hours=(uptime%(24*3600))/3600;
    int mins =(uptime%3600)/60;

    sprintf(output->text_value,"%dD %2dH %2dM",
            days,
            hours,
            mins);

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
