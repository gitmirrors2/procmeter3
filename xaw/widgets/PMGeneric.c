/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMGeneric.c,v 1.4 2001-01-04 19:26:46 amb Exp $

  ProcMeter Generic Widget Source file (for ProcMeter 3.3).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,2000 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>

#include "PMGenericP.h"

static void Initialize(ProcMeterGenericWidget request,ProcMeterGenericWidget new);
static void Destroy(ProcMeterGenericWidget pmw);
static Boolean SetValues(ProcMeterGenericWidget current,ProcMeterGenericWidget request,ProcMeterGenericWidget new);
static void Resize(ProcMeterGenericWidget pmw);
static void Redisplay(ProcMeterGenericWidget pmw,XEvent *event,Region region);
static void GenericResize(ProcMeterGenericWidget pmw);
static void GenericUpdate(ProcMeterGenericWidget pmw);

static XtResource resources[]=
{
 /* The body parts. */

 {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
  XtOffset(ProcMeterGenericWidget,procmeter_generic.body_pixel),XtRString,XtDefaultForeground},

 /* The label parts. */

 {XtNlabel, XtCLabel, XtRString, sizeof(XtPointer),
  XtOffset(ProcMeterGenericWidget,procmeter_generic.label_string), XtRString, "" },
 {XtNlabelForeground, XtCForeground, XtRPixel, sizeof(Pixel),
  XtOffset(ProcMeterGenericWidget,procmeter_generic.label_pixel),XtRString,XtDefaultForeground},
 {XtNlabelPosition, XtCLabelPosition, XtRInt, sizeof(int),
  XtOffset(ProcMeterGenericWidget,procmeter_generic.label_pos), XtRString, "-1" },
 {XtNlabelFont, XtCFont, XtRFontStruct, sizeof(XFontStruct*),
  XtOffset(ProcMeterGenericWidget,procmeter_generic.label_font), XtRString, "-*-*-*-r-normal-sans-8-*-*-*-p-*-*-*"}
};

/*+ The actual ProcMeter Generic Widget Class Record. +*/
ProcMeterGenericClassRec procMeterGenericClassRec=
{
 {
  (WidgetClass) &widgetClassRec,
  "ProcMeterGeneric",
  sizeof(ProcMeterGenericRec),
  NULL,
  NULL,
  FALSE,
  (XtInitProc)Initialize,
  NULL,
  XtInheritRealize,
  NULL,
  0,
  resources,
  XtNumber(resources),
  NULLQUARK,
  TRUE,
  XtExposeCompressMaximal|XtExposeGraphicsExpose,
  TRUE,
  TRUE,
  (XtWidgetProc)Destroy,
  (XtWidgetProc)Resize,
  (XtExposeProc)Redisplay,
  (XtSetValuesFunc)SetValues,
  NULL,
  XtInheritSetValuesAlmost,
  NULL,
  NULL,
  XtVersion,
  NULL,
  XtInheritTranslations,
  NULL,
  NULL,
  NULL,
 },
 {
  GenericResize,
  GenericUpdate
 }
};

/*+ The actual ProcMeter Generic Widget Class Record masquerading as a WidgetClass type. +*/
WidgetClass procMeterGenericWidgetClass=(WidgetClass)&procMeterGenericClassRec;


/*++++++++++++++++++++++++++++++++++++++
  Initialise a new ProcMeter Generic Widget.

  ProcMeterGenericWidget request The requested parameters.

  ProcMeterGenericWidget new The new parameters that are to be filled in.
  ++++++++++++++++++++++++++++++++++++++*/

static void Initialize(ProcMeterGenericWidget request,ProcMeterGenericWidget new)
{
 XGCValues values;

 /* The core widget parts. */

 if(request->core.width ==0)
    new->core.width=100;
 if(request->core.height==0)
    new->core.height=100;

 /* The body parts. */

 values.foreground=new->procmeter_generic.body_pixel;
 values.background=new->core.background_pixel;
 new->procmeter_generic.body_gc=XtGetGC((Widget)new,GCForeground|GCBackground,&values);

 /* The label parts. */

 if((new->procmeter_generic.label_pos!=ProcMeterLabelTop) &&
    (new->procmeter_generic.label_pos!=ProcMeterLabelNone) &&
    (new->procmeter_generic.label_pos!=ProcMeterLabelBottom))
    new->procmeter_generic.label_pos=ProcMeterLabelNone;

 new->procmeter_generic.label_string=XtNewString(request->procmeter_generic.label_string);

 if(!new->procmeter_generic.label_font)
    new->procmeter_generic.label_font=XLoadQueryFont(XtDisplay(new),"-*-*-*-r-normal-sans-8-*-*-*-p-*-*-*");

 values.font=new->procmeter_generic.label_font->fid;
 values.foreground=new->procmeter_generic.label_pixel;
 values.background=new->core.background_pixel;
 new->procmeter_generic.label_gc=XtGetGC((Widget)new,GCForeground|GCBackground|GCFont,&values);

 /* The rest of the sizing. */

 GenericResize(new);
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a ProcMeter Generic Widget.

  ProcMeterGenericWidget pmw The Widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void Destroy(ProcMeterGenericWidget pmw)
{
 XtFree((XtPointer)pmw->procmeter_generic.label_string);
 XtReleaseGC((Widget)pmw,pmw->procmeter_generic.body_gc);
 XtReleaseGC((Widget)pmw,pmw->procmeter_generic.label_gc);
}


/*++++++++++++++++++++++++++++++++++++++
  The setvalues procedure that is used to set the values internal to the Widget.

  Boolean SetValues Returns True if the Widget is to be redrawn.

  ProcMeterGenericWidget current The current Widget values.

  ProcMeterGenericWidget request The requested Widget values.

  ProcMeterGenericWidget new The new Widget values to be set up.
  ++++++++++++++++++++++++++++++++++++++*/

static Boolean SetValues(ProcMeterGenericWidget current,ProcMeterGenericWidget request,ProcMeterGenericWidget new)
{
 Boolean redraw=False;

 /* The body parts. */

 if(request->procmeter_generic.body_pixel!=current->procmeter_generic.body_pixel)
   {
    XGCValues xgcv;

    XGetGCValues(XtDisplay(new),new->procmeter_generic.body_gc,GCForeground|GCBackground,&xgcv);
    XtReleaseGC((Widget)new,new->procmeter_generic.body_gc);
    xgcv.foreground=request->procmeter_generic.body_pixel;
    xgcv.background=request->core.background_pixel;
    new->procmeter_generic.body_gc=XtGetGC((Widget)new,GCForeground|GCBackground,&xgcv);

    redraw=True;
   }

 /* The label parts. */

 if(request->procmeter_generic.label_pos!=current->procmeter_generic.label_pos)
   {
    if((request->procmeter_generic.label_pos!=ProcMeterLabelTop) &&
       (request->procmeter_generic.label_pos!=ProcMeterLabelNone) &&
       (request->procmeter_generic.label_pos!=ProcMeterLabelBottom))
       new->procmeter_generic.label_pos=ProcMeterLabelNone;

    redraw=True;
   }

 if(request->procmeter_generic.label_string!=current->procmeter_generic.label_string)
   {
    XtFree((XtPointer)new->procmeter_generic.label_string);
    new->procmeter_generic.label_string=XtNewString(request->procmeter_generic.label_string);

    redraw=True;
   }

 if((request->procmeter_generic.label_font !=current->procmeter_generic.label_font)||
    (request->procmeter_generic.label_pixel!=current->procmeter_generic.label_pixel))
   {
    XGCValues xgcv;

    XGetGCValues(XtDisplay(new),new->procmeter_generic.label_gc,GCForeground|GCBackground|GCFont,&xgcv);
    XtReleaseGC((Widget)new,new->procmeter_generic.label_gc);
    xgcv.font=request->procmeter_generic.label_font->fid;
    xgcv.foreground=request->procmeter_generic.label_pixel;
    xgcv.background=request->core.background_pixel;
    new->procmeter_generic.label_gc=XtGetGC((Widget)new,GCForeground|GCBackground|GCFont,&xgcv);

    redraw=True;
   }

 /* Resize if needed */

 if(redraw)
    GenericResize(new);

 return(redraw);
}


/*++++++++++++++++++++++++++++++++++++++
  Resize the ProcMeter Generic Widget.

  ProcMeterGenericWidget pmw The Widget that is resized.
  ++++++++++++++++++++++++++++++++++++++*/

static void Resize(ProcMeterGenericWidget pmw)
{
 GenericResize(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Generic Widget.

  ProcMeterGenericWidget pmw The Widget to redisplay.

  XEvent *event The event that caused the redisplay.

  Region region The region that was exposed.
  ++++++++++++++++++++++++++++++++++++++*/

static void Redisplay(ProcMeterGenericWidget pmw,XEvent *event,Region region)
{
 if(pmw->core.visible)
    GenericUpdate(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform all of the sizing on the Widget when it is created/resized.

  ProcMeterGenericWidget pmw The Widget to resize.
  ++++++++++++++++++++++++++++++++++++++*/

static void GenericResize(ProcMeterGenericWidget pmw)
{
 /* The label parts. */

 if(pmw->procmeter_generic.label_pos)
   {
    pmw->procmeter_generic.label_height=pmw->procmeter_generic.label_font->ascent+pmw->procmeter_generic.label_font->descent+2;
    pmw->procmeter_generic.label_x=(pmw->core.width-XTextWidth(pmw->procmeter_generic.label_font,pmw->procmeter_generic.label_string,(int)strlen(pmw->procmeter_generic.label_string)))/2;
    if(pmw->procmeter_generic.label_pos==ProcMeterLabelTop)
       pmw->procmeter_generic.label_y=pmw->procmeter_generic.label_height-1-pmw->procmeter_generic.label_font->descent;
    else
       pmw->procmeter_generic.label_y=pmw->core.height-pmw->procmeter_generic.label_font->descent;
   }
 else
   {
    pmw->procmeter_generic.label_height=0;
    pmw->procmeter_generic.label_x=0;
    pmw->procmeter_generic.label_y=0;
   }

 /* The body parts. */

 pmw->procmeter_generic.body_height=pmw->core.height-pmw->procmeter_generic.label_height;

 if(pmw->procmeter_generic.label_pos==ProcMeterLabelTop)
    pmw->procmeter_generic.body_start=pmw->procmeter_generic.label_height;
 else
    pmw->procmeter_generic.body_start=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Update the display of the generic part of the widget.

  ProcMeterGenericWidget pmw The Widget to update.
  ++++++++++++++++++++++++++++++++++++++*/

static void GenericUpdate(ProcMeterGenericWidget pmw)
{
 if(pmw->core.visible)
   {
    XClearWindow(XtDisplay(pmw),XtWindow(pmw));

    if(pmw->procmeter_generic.label_pos)
      {
       XDrawString(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.label_gc,
                   pmw->procmeter_generic.label_x,pmw->procmeter_generic.label_y,
                   pmw->procmeter_generic.label_string,(int)strlen(pmw->procmeter_generic.label_string));

       if(pmw->procmeter_generic.label_pos==ProcMeterLabelTop)
          XDrawLine(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.label_gc,
                    0              ,pmw->procmeter_generic.label_height-1,
                    pmw->core.width,pmw->procmeter_generic.label_height-1);
       else
          XDrawLine(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.label_gc,
                    0              ,pmw->procmeter_generic.body_height,
                    pmw->core.width,pmw->procmeter_generic.body_height);
      }
   }
}
