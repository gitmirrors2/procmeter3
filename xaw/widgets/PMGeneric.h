/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMGeneric.h,v 1.1 1998-09-19 15:28:31 amb Exp $

  ProcMeter Generic Widget Public include file (for ProcMeter 3.0).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMGENERIC_H
#define PMGENERIC_H    /*+ To stop multiple inclusions. +*/

/*+ The ProcMeter Generic Widget Class Record. +*/
extern WidgetClass procMeterGenericWidgetClass;

/*+ An opaque reference to the ProcMeter Generic Widget Class Record type. +*/
typedef struct _ProcMeterGenericClassRec *ProcMeterGenericWidgetClass;

/*+ An opaque reference to the ProcMeter Generic Widget Record type. +*/
typedef struct _ProcMeterGenericRec      *ProcMeterGenericWidget;

/* The resource names */

#define XtNlabelForeground "labelforeground"
#define XtNlabelFont       "labelfont"

#define XtNlabelPosition   "labelPosition"

#define XtCLabelPosition   "LabelPosition"

/* The options for label placement */

#define ProcMeterLabelTop     1
#define ProcMeterLabelNone    0
#define ProcMeterLabelBottom -1

#endif /* PMGENERIC_H */
