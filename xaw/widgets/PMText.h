/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMText.h,v 1.2 2000-12-16 17:03:10 amb Exp $

  ProcMeter Text Widget Public include file (for ProcMeter 3.3).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,2000 Andrew M. Bishop
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

void ProcMeterTextChangeData(Widget pmw,char *data);

#endif /* PMTEXT_H */
