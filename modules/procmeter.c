/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/procmeter.c,v 1.2 1998-09-22 18:45:33 amb Exp $

  ProcMeter - A system monitoring program for Linux.

  Information about the ProcMeter program source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The version output +*/
ProcMeterOutput version_output=
{
 /* char  name[16];         */ "Version",
 /* char *description;      */ "The version of procmeter that is running.",
 /* char  type;             */ PROCMETER_TEXT,
 /* short interval;         */ 0,
 /* char  text_value[16];   */ "ProcMeter V" PROCMETER_VERSION,
 /* long  graph_value;      */ -1,
 /* short graph_scale;      */ 0,
 /* char  graph_units[8];   */ "n/a"
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[]=
{
 &version_output,
 NULL
};

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];             */ "ProcMeter",
 /* char *description;         */ "Information about the procmeter program itself.",
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
 if(output==&version_output)
    return(0);

 return(-1);
}


/*++++++++++++++++++++++++++++++++++++++
  Tidy up and prepare to have the module unloaded.
  ++++++++++++++++++++++++++++++++++++++*/

void Unload(void)
{
}
