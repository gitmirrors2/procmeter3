/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMTextP.h,v 1.1 1998-09-19 15:29:36 amb Exp $

  ProcMeter Text Widget Private header file (for ProcMeter 3.0).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMTEXTP_H
#define PMTEXTP_H    /*+ To stop multiple inclusions. +*/

#include "PMText.h"

/*+ The Class Part of the Widget, shared among all instances of the ProcMeter Text Widget. +*/
typedef struct _ProcMeterTextClassPart
{
 int unused;                    /*+ Not used. +*/
}
ProcMeterTextClassPart;

/*+ The complete Class Record for the ProcMeter Text Widget, includes the Core Widget Class Part. +*/
typedef struct _ProcMeterTextClassRec
{
 CoreClassPart core_class;                          /*+ The Core Widget Class Part. +*/
 ProcMeterGenericClassPart procmeter_generic_class; /*+ The ProcMeter Generic Widget Class Part. +*/
 ProcMeterTextClassPart procmeter_text_class;       /*+ The ProcMeter Text Widget Class Part. +*/
}
ProcMeterTextClassRec;

/*+ The actual Class Record for the ProcMeter Text Widget. +*/
extern ProcMeterTextClassRec procMeterTextClassRec;

/*+ The ProcMeter Text Widget Part that is used in each of the ProcMeter Text Widgets. +*/
typedef struct _ProcMeterPart
{
 char*           text_string;     /*+ The text for the Widget (Set & Get via Xt). +*/
 XFontStruct*    text_font;       /*+ The font for the text (Set & Get via Xt). +*/
 Dimension       text_x,text_y;   /*+ The position of the text. +*/
}
ProcMeterTextPart;

/*+ The complete Widget Record that is used per ProcMeter Text Widget. +*/
typedef struct _ProcMeterTextRec
{
 CorePart  core;                         /*+ The Core Widget Part. +*/
 ProcMeterGenericPart procmeter_generic; /*+ The ProcMeter Generic Widget Part. +*/
 ProcMeterTextPart procmeter_text;       /*+ The ProcMeter Text Widget Part. +*/
}
ProcMeterTextRec;

#endif /* PMTEXTP_H */
