/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMGraph.h,v 1.3 2000-12-16 17:02:52 amb Exp $

  ProcMeter Graph Widget Public include file (for ProcMeter3 3.3).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,99,2000 Andrew M. Bishop
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

void ProcMeterGraphAddDatum(Widget pmw,unsigned short datum);

#endif /* PMGRAPH_H */
