/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMText.h,v 1.1 1998-09-19 15:29:29 amb Exp $

  ProcMeter Text Widget Public include file (for ProcMeter 3.0).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMTEXT_H
#define PMTEXT_H    /*+ To stop multiple inclusions. +*/

/*+ The ProcMeter Text Widget Class Record. +*/
extern WidgetClass procMeterTextWidgetClass;

/*+ An opaque reference to the ProcMeter Text Widget Class Record type. +*/
typedef struct _ProcMeterTextClassRec *ProcMeterTextWidgetClass;

/*+ An opaque reference to the ProcMeter Text Widget Record type. +*/
typedef struct _ProcMeterTextRec      *ProcMeterTextWidget;

/* The resource names */

#define XtNtext      "text"
#define XtNtextFont  "textFont"

/* Public functions */

void ProcMeterTextWidgetChangeData(Widget pmw,char *data);

#endif /* PMGRAPH_H */
