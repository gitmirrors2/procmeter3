/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMGraph.h,v 1.2 1999-02-13 11:37:24 amb Exp $

  ProcMeter Graph Widget Public include file (for ProcMeter3 3.1).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMGRAPH_H
#define PMGRAPH_H    /*+ To stop multiple inclusions. +*/

/*+ The ProcMeter Graph Widget Class Record. +*/
extern WidgetClass procMeterGraphWidgetClass;

/*+ An opaque reference to the ProcMeter Graph Widget Class Record type. +*/
typedef struct _ProcMeterGraphClassRec *ProcMeterGraphWidgetClass;

/*+ An opaque reference to the ProcMeter Graph Widget Record type. +*/
typedef struct _ProcMeterGraphRec      *ProcMeterGraphWidget;

/* The resource names */

#define XtNsolid          "solid"
#define XtNgridUnits      "gridUnits"
#define XtNgridForeground "gridForeground"
#define XtNgridMin        "gridMin"
#define XtNgridMax        "gridMax"

#define XtCSolid         "Solid"
#define XtCGridMin       "GridMin"
#define XtCGridMax       "GridMax"

/* Public functions */

void ProcMeterGraphWidgetAddDatum(Widget pmw,unsigned short datum);

#endif /* PMGRAPH_H */
