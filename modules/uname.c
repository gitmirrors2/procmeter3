/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/uname.c,v 1.2 1998-10-24 09:02:09 amb Exp $

  ProcMeter - A system monitoring program for Linux (v3.0a).

  Uname system information module source file.
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

#include <sys/utsname.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The hostname output. +*/
ProcMeterOutput hostname_output=
{
 /* char  name[16];         */ "Hostname",
 /* char *description;      */ "The canonical name of the computer.",
 /* char  type;             */ PROCMETER_TEXT,
 /* short interval;         */ 0,
 /* char  text_value[16];   */ "unknown",
 /* long  graph_value;      */ -1,
 /* short graph_scale;      */ 0,
 /* char  graph_units[8];   */ "n/a"
};

/*+ The kernel version output. +*/
ProcMeterOutput kernelversion_output=
{
 /* char  name[16];         */ "Kernel_Version",
 /* char *description;      */ "The kernel version that is running (%s).",
 /* char  type;             */ PROCMETER_TEXT,
 /* short interval;         */ 0,
 /* char  text_value[16];   */ "unknown",
 /* long  graph_value;      */ -1,
 /* short graph_scale;      */ 0,
 /* char  graph_units[8];   */ "n/a"
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[]=
{
 &hostname_output,
 &kernelversion_output,
 NULL
};

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];             */ "Uname",
 /* char *description;         */ "The system information from the uname program.",
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
 char *format,line[128];
 struct utsname buf;
 FILE *f;

 strcpy(line,"unknown");

 f=fopen("/proc/version","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/version'\n",__FILE__);
 else
   {
    if(!fgets(line,128,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/version'\n",__FILE__);
    fclose(f);
   }

 format=kernelversion_output.description;
 kernelversion_output.description=(char*)malloc(strlen(format)+strlen(line));
 sprintf(kernelversion_output.description,format,line);

 if(uname(&buf))
    fprintf(stderr,"ProcMeter(%s): Error calling uname()\n",__FILE__);
 else
   {
    char *p;

    strncpy(hostname_output.text_value,buf.nodename,16);     hostname_output.text_value[15]=0;
    strncpy(kernelversion_output.text_value,buf.release,16); kernelversion_output.text_value[15]=0;

    p=hostname_output.text_value;
    while(*p && *p!='.')
       p++;
    *p=0;
   }

 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform an update on one of the statistics.

  void *Update Returns the value (int or char*).

  time_t now The current time.

  ProcMeterOutput* output The output that the value is wanted for.
  ++++++++++++++++++++++++++++++++++++++*/

int Update(time_t now,ProcMeterOutput* output)
{
 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Tidy up and prepare to have the module unloaded.
  ++++++++++++++++++++++++++++++++++++++*/

void Unload(void)
{
 free(kernelversion_output.description);
}
