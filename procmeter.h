/***************************************
  $Header: /home/amb/CVS/procmeter3/procmeter.h,v 1.2 1998-10-24 13:12:02 amb Exp $

  ProcMeter - A system monitoring program for Linux (v3.0a).

  Global public header file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PROCMETER_H
#define PROCMETER_H    /*+ To stop multiple inclusions. +*/

#include <time.h>


#define PROCMETER_VERSION "3.0a"


#define PROCMETER_MAJOR_VERSION 3
#define PROCMETER_MINOR_VERSION 0


#define PROCMETER_GRAPH 1
#define PROCMETER_TEXT  2

/*+ The scaling factor used to get the number of grid lines. +*/
#define PROCMETER_GRAPH_SCALE 1024

/*+ A scaling function for floating point numbers. +*/
#define PROCMETER_GRAPH_FLOATING(xx) ((long)((xx)*1024))

/*+ A scaling function for integers. +*/
#define PROCMETER_GRAPH_INTEGER(xx)  ((long)((xx)<<10))


/*+ The information about one of the outputs. +*/
typedef struct _ProcMeterOutput
{
 char  name[16];                /*+ The name of the output. +*/
 char *description;             /*+ A long description of the output. +*/
 char  type;                    /*+ The type of output. +*/
 short interval;                /*+ The interval between updates. +*/
 char  text_value[16];          /*+ The textual value (if applicable). +*/
 long  graph_value;             /*+ The graph value (if applicable). +*/
 short graph_scale;             /*+ The scaling factor for the graph (if applicable). +*/
 char  graph_units[8];          /*+ The units on the graph (if applicable). +*/
}
ProcMeterOutput;


/*+ The information about one of the modules. +*/
typedef struct _ProcMeterModule
{
 char name[16];                 /*+ The module's name. +*/
 char *description;             /*+ A long description of the module. +*/
}
ProcMeterModule;


/* The funtions exported from the modules. */

ProcMeterModule *Load(void);

ProcMeterOutput** Initialise(char *options);

int Update(time_t now,ProcMeterOutput *output);

void Unload(void);


#endif /* PROCMETER_H */
