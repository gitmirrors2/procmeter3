/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/template.c,v 1.4 2002-06-04 13:54:07 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3b.

  Module template source file.
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

#include "procmeter.h"

/* The interface information.  */

/*+ The graph output +*/
ProcMeterOutput graph_output=
{
 /* char  name[PROCMETER_NAME_LEN]; */ "Example_Graph",
 /* char *description;              */ "An example graph to show how it works",
 /* char  type;                     */ PROCMETER_GRAPH|PROCMETER_BAR,
 /* short interval;                 */ 1,
 /* char  text_value[16];           */ "n/a",
 /* long  graph_value;              */ 0,
 /* short graph_scale;              */ 10,
 /* char  graph_units[8];           */ "(%d)"
};

/*+ The text output. +*/
ProcMeterOutput text_output=
{
 /* char  name[PROCMETER_NAME_LEN]; */ "Example_Text",
 /* char *description;              */ "An example text field to show how it works",
 /* char  type;                     */ PROCMETER_TEXT,
 /* short interval;                 */ 10,
 /* char  text_value[16];           */ "unknown",
 /* long  graph_value;              */ -1,
 /* short graph_scale;              */ 0,
 /* char  graph_units[8];           */ "n/a"
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[]=
{
 &graph_output,
 &text_output,
 NULL
};

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[PROCMETER_NAME_LEN]; */ "Template",
 /* char *description;             */ "A source code template of the sort of module that ProcMeter can use.",
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
 if(output==&graph_output)
   {
    int result=now%100;

    output->graph_value=PROCMETER_GRAPH_FLOATING((double)result/output->graph_scale);

    return(0);
   }
 else if(output==&text_output)
   {
    static char result1[16]="Example Output";
    static char result2[16]="ProcMeter";

    if(now%60)
       strcpy(output->text_value,result1);
    else
       strcpy(output->text_value,result2);

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
