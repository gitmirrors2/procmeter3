/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMGraphP.h,v 1.2 1999-02-13 11:37:24 amb Exp $

  ProcMeter Graph Widget Private header file (for ProcMeter3 3.1).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMGRAPHP_H
#define PMGRAPHP_H    /*+ To stop multiple inclusions. +*/

#include "PMGraph.h"

/*+ The Class Part of the Widget, shared among all instances of the ProcMeter Graph Widget. +*/
typedef struct _ProcMeterGraphClassPart
{
 int unused;                    /*+ Not used. +*/
}
ProcMeterGraphClassPart;

/*+ The complete Class Record for the ProcMeter Graph Widget, includes the Core Widget Class Part. +*/
typedef struct _ProcMeterGraphClassRec
{
 CoreClassPart core_class;                          /*+ The Core Widget Class Part. +*/
 ProcMeterGenericClassPart procmeter_generic_class; /*+ The ProcMeter Generic Widget Class Part. +*/
 ProcMeterGraphClassPart procmeter_graph_class;     /*+ The ProcMeter Graph Widget Class Part. +*/
}
ProcMeterGraphClassRec;

/*+ The actual Class Record for the ProcMeter Graph Widget. +*/
extern ProcMeterGraphClassRec procMeterGraphClassRec;

/*+ The ProcMeter Graph Widget Part that is used in each of the ProcMeter Graph Widgets. +*/
typedef struct _ProcMeterGraphPart
{
 char*           grid_units;       /*+ The number of things per grid line (Set & Get via Xt). +*/
 Dimension       grid_units_x;     /*+ The position of the grid units. +*/
 Pixel           grid_pixel;       /*+ The grid lines colour (Set & Get via Xt). +*/
 GC              grid_gc;          /*+ The graphics context for the grid lines. +*/
 int             grid_min;         /*+ The minimum number of grid lines (Set & Get via Xt). +*/
 int             grid_max;         /*+ The maximum number of grid lines (Set & Get via Xt). +*/
 int             grid_maxvis;      /*+ The maximum number of grid lines before removing them. +*/
 int             grid_drawn;       /*+ If 1 then draw as normal, if 0 never draw, if -1 draw only one line. +*/
 int             grid_num;         /*+ The actual number of grid lines. +*/

 Boolean         line_solid;       /*+ True if the area under the graph is to be filled (Set & Get via Xt). +*/

 unsigned short* data;             /*+ The data for the graph. +*/
 unsigned short  data_max;         /*+ The maximum data value. +*/
 unsigned int    data_num;         /*+ The number of data points. +*/
 int             data_index;       /*+ An index into the array for the new value. +*/
}
ProcMeterGraphPart;

/*+ The complete Widget Record that is used per ProcMeter Graph Widget. +*/
typedef struct _ProcMeterGraphRec
{
 CorePart  core;                         /*+ The Core Widget Part. +*/
 ProcMeterGenericPart procmeter_generic; /*+ The ProcMeter Generic Widget Part. +*/
 ProcMeterGraphPart procmeter_graph;     /*+ The ProcMeter Graph Widget Part. +*/
}
ProcMeterGraphRec;

#endif /* PMGRAPHP_H */
