/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/procmeter.c,v 1.5 2002-12-07 19:38:59 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4.

  Information about the ProcMeter program source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2002 Andrew M. Bishop
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
 /* char  name[];          */ "Version",
 /* char *description;     */ "The version of procmeter that is running.",
 /* char  type;            */ PROCMETER_TEXT,
 /* short interval;        */ 0,
 /* char  text_value[];    */ "ProcMeter V" PROCMETER_VERSION,
 /* long  graph_value;     */ -1,
 /* short graph_scale;     */ 0,
 /* char  graph_units[];   */ "n/a"
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
 /* char name[];           */ "ProcMeter",
 /* char *description;     */ "Information about the procmeter program itself.",
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
