/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMBar.h,v 1.1 1999-09-30 17:42:08 amb Exp $

  ProcMeter Bar Widget Public include file (for ProcMeter3 3.2).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMBAR_H
#define PMBAR_H    /*+ To stop multiple inclusions. +*/

/*+ The ProcMeter Bar Widget Class Record. +*/
extern WidgetClass procMeterBarWidgetClass;

/*+ An opaque reference to the ProcMeter Bar Widget Class Record type. +*/
typedef struct _ProcMeterBarClassRec *ProcMeterBarWidgetClass;

/*+ An opaque reference to the ProcMeter Bar Widget Record type. +*/
typedef struct _ProcMeterBarRec      *ProcMeterBarWidget;

/* The resource names */

#define XtNgridUnits      "gridUnits"
#define XtNgridForeground "gridForeground"
#define XtNgridMin        "gridMin"
#define XtNgridMax        "gridMax"

#define XtCGridMin       "GridMin"
#define XtCGridMax       "GridMax"

/* Public functions */

void ProcMeterBarWidgetAddDatum(Widget pmw,unsigned short datum);

#endif /* PMBAR_H */
