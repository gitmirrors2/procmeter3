/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMBarP.h,v 1.1 1999-09-30 17:42:18 amb Exp $

  ProcMeter Bar Widget Private header file (for ProcMeter3 3.2).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMBARP_H
#define PMBARP_H    /*+ To stop multiple inclusions. +*/

#include "PMBar.h"

/*+ The Class Part of the Widget, shared among all instances of the ProcMeter Bar Widget. +*/
typedef struct _ProcMeterBarClassPart
{
 int unused;                    /*+ Not used. +*/
}
ProcMeterBarClassPart;

/*+ The complete Class Record for the ProcMeter Bar Widget, includes the Core Widget Class Part. +*/
typedef struct _ProcMeterBarClassRec
{
 CoreClassPart core_class;                          /*+ The Core Widget Class Part. +*/
 ProcMeterGenericClassPart procmeter_generic_class; /*+ The ProcMeter Generic Widget Class Part. +*/
 ProcMeterBarClassPart procmeter_bar_class;         /*+ The ProcMeter Bar Widget Class Part. +*/
}
ProcMeterBarClassRec;

/*+ The actual Class Record for the ProcMeter Bar Widget. +*/
extern ProcMeterBarClassRec procMeterBarClassRec;

/*+ The ProcMeter Bar Widget Part that is used in each of the ProcMeter Bar Widgets. +*/
typedef struct _ProcMeterBarPart
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

 unsigned short  data;             /*+ The data for the bar. +*/
}
ProcMeterBarPart;

/*+ The complete Widget Record that is used per ProcMeter Bar Widget. +*/
typedef struct _ProcMeterBarRec
{
 CorePart  core;                         /*+ The Core Widget Part. +*/
 ProcMeterGenericPart procmeter_generic; /*+ The ProcMeter Generic Widget Part. +*/
 ProcMeterBarPart procmeter_bar;         /*+ The ProcMeter Bar Widget Part. +*/
}
ProcMeterBarRec;

#endif /* PMBARP_H */
