/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMGenericP.h,v 1.1 1998-09-19 15:28:40 amb Exp $

  ProcMeter Generic Widget Private header file (for ProcMeter 3.0).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMGENERICP_H
#define PMGENERICP_H    /*+ To stop multiple inclusions. +*/

#include "PMGeneric.h"

/*+ The Class Part of the Widget, shared among all instances of the ProcMeter Generic Widget. +*/
typedef struct _ProcMeterGenericClassPart
{
 int unused;                    /*+ Not used. +*/
}
ProcMeterGenericClassPart;

/*+ The complete Class Record for the ProcMeter Generic Widget, includes the Core Widget Class Part. +*/
typedef struct _ProcMeterGenericClassRec
{
 CoreClassPart core_class;                          /*+ The Core Widget Class Part. +*/
 ProcMeterGenericClassPart procmeter_generic_class; /*+ The ProcMeter Generic Widget Class Part. +*/
}
ProcMeterGenericClassRec;

/*+ The actual Class Record for the ProcMeter Generic Widget. +*/
extern ProcMeterGenericClassRec procMeterGenericClassRec;

/*+ The ProcMeter Generic Widget Part that is used in each of the ProcMeter Generic Widgets. +*/
typedef struct _ProcMeterGenericPart
{
 Pixel           body_pixel;       /*+ The body colour (Set & Get via Xt). +*/
 GC              body_gc;          /*+ The graphics context for the body. +*/
 Dimension       body_height;      /*+ The height of the body part. +*/
 Dimension       body_start;       /*+ The start position of the body part. +*/

 char*           label_string;     /*+ The label for the Widget (Set & Get via Xt). +*/
 Pixel           label_pixel;      /*+ The label colour (Set & Get via Xt). +*/
 GC              label_gc;         /*+ The graphics context for the label. +*/
 int             label_pos;        /*+ The position of the label (Set & Get via Xt). +*/
 XFontStruct*    label_font;       /*+ The font for the label (Set & Get via Xt). +*/
 Dimension       label_height;     /*+ The height of the label. +*/
 Dimension       label_x,label_y;  /*+ The position of the label. +*/
}
ProcMeterGenericPart;

/*+ The complete Widget Record that is used per ProcMeter Generic Widget. +*/
typedef struct _ProcMeterGenericRec
{
 CorePart  core;                         /*+ The Core Widget Part. +*/
 ProcMeterGenericPart procmeter_generic; /*+ The ProcMeter Generic Widget Part. +*/
}
ProcMeterGenericRec;

/* Generic functions */

void ProcMeterGenericResize(ProcMeterGenericWidget w);
void ProcMeterGenericUpdate(ProcMeterGenericWidget w);

#endif /* PMGENERICP_H */
