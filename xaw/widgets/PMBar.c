/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMBar.c,v 1.1 1999-09-30 17:41:57 amb Exp $

  ProcMeter Bar Widget Source file (for ProcMeter3 3.2).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,99 Andrew M. Bishop
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
#include <X11/Xaw/Paned.h>

#include "PMGenericP.h"
#include "PMBarP.h"

#include "procmeter.h"

static void Initialize(ProcMeterBarWidget request,ProcMeterBarWidget new);
static void Destroy(ProcMeterBarWidget w);
static Boolean SetValues(ProcMeterBarWidget current,ProcMeterBarWidget request,ProcMeterBarWidget new);
static void Resize(ProcMeterBarWidget w);
static void Redisplay(ProcMeterBarWidget w,XEvent *event,Region region);
static void BarResize(ProcMeterBarWidget w);
static void BarUpdate(ProcMeterBarWidget w);

static XtResource resources[]=
{
 /* The grid parts. */

 {XtNgridUnits, XtCLabel, XtRString, sizeof(XtPointer),
  XtOffset(ProcMeterBarWidget,procmeter_bar.grid_units), XtRString, "" },
 {XtNgridForeground, XtCForeground, XtRPixel, sizeof(Pixel),
  XtOffset(ProcMeterBarWidget,procmeter_bar.grid_pixel),XtRString,XtDefaultBackground},
 {XtNgridMin, XtCGridMin, XtRInt, sizeof(int),
  XtOffset(ProcMeterBarWidget,procmeter_bar.grid_min), XtRString, "1" },
 {XtNgridMax, XtCGridMax, XtRInt, sizeof(int),
  XtOffset(ProcMeterBarWidget,procmeter_bar.grid_max), XtRString, "0" }
};

/*+ The actual ProcMeter Bar Widget Class Record. +*/
ProcMeterBarClassRec procMeterBarClassRec=
{
 {
  (WidgetClass) &procMeterGenericClassRec,
  "ProcMeterBar",
  sizeof(ProcMeterBarRec),
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
  0
 },
 {
  0
 }
};

/*+ The actual ProcMeter Bar Widget Class Record masquerading as a WidgetClass type. +*/
WidgetClass procMeterBarWidgetClass=(WidgetClass)&procMeterBarClassRec;


/*++++++++++++++++++++++++++++++++++++++
  Initialise a new ProcMeter Bar Widget.

  ProcMeterBarWidget request The requested parameters.

  ProcMeterBarWidget new The new parameters that are to be filled in.
  ++++++++++++++++++++++++++++++++++++++*/

static void Initialize(ProcMeterBarWidget request,ProcMeterBarWidget new)
{
 XGCValues values;

 /* The grid parts. */

 new->procmeter_bar.grid_units=XtNewString(request->procmeter_bar.grid_units);

 values.foreground=new->procmeter_bar.grid_pixel;
 values.background=new->core.background_pixel;
 new->procmeter_bar.grid_gc=XtGetGC((Widget)new,GCForeground|GCBackground,&values);

 if(request->procmeter_bar.grid_min<0)
    new->procmeter_bar.grid_min=-request->procmeter_bar.grid_min,
    new->procmeter_bar.grid_drawn=0;
 else
    new->procmeter_bar.grid_drawn=1;
 if(request->procmeter_bar.grid_min==0)
    new->procmeter_bar.grid_min=1;

 if(request->procmeter_bar.grid_max<0)
    new->procmeter_bar.grid_max=0;

 if(new->procmeter_bar.grid_max && new->procmeter_bar.grid_max<new->procmeter_bar.grid_min)
    new->procmeter_bar.grid_max=new->procmeter_bar.grid_min;

 new->procmeter_bar.grid_num=new->procmeter_bar.grid_min;

 /* The data parts. */

 new->procmeter_bar.data=0;

 /* The rest of the sizing. */

 BarResize(new);
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a ProcMeter Bar Widget.

  ProcMeterBarWidget w The Widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void Destroy(ProcMeterBarWidget w)
{
 XtReleaseGC((Widget)w,w->procmeter_bar.grid_gc);
 XtFree((XtPointer)w->procmeter_bar.grid_units);
}


/*++++++++++++++++++++++++++++++++++++++
  The setvalues procedure that is used to set the values internal to the Widget.

  Boolean SetValues Returns True if the Widget is to be redrawn.

  ProcMeterBarWidget current The current Widget values.

  ProcMeterBarWidget request The requested Widget values.

  ProcMeterBarWidget new The new Widget values to be set up.
  ++++++++++++++++++++++++++++++++++++++*/

static Boolean SetValues(ProcMeterBarWidget current,ProcMeterBarWidget request,ProcMeterBarWidget new)
{
 Boolean redraw=False;

 /* The grid parts. */

 if(request->procmeter_bar.grid_units!=current->procmeter_bar.grid_units)
   {
    XtFree((XtPointer)new->procmeter_bar.grid_units);
    new->procmeter_bar.grid_units=XtNewString(request->procmeter_bar.grid_units);

    redraw=True;
   }

 if(request->procmeter_bar.grid_pixel!=current->procmeter_bar.grid_pixel)
   {
    XGCValues xgcv;

    XGetGCValues(XtDisplay(new),new->procmeter_bar.grid_gc,GCForeground|GCBackground,&xgcv);
    XtReleaseGC((Widget)new,new->procmeter_bar.grid_gc);
    xgcv.foreground=request->procmeter_bar.grid_pixel;
    xgcv.background=request->core.background_pixel;
    new->procmeter_bar.grid_gc=XtGetGC((Widget)new,GCForeground|GCBackground,&xgcv);

    redraw=True;
   }

 if(request->procmeter_bar.grid_min!=current->procmeter_bar.grid_min)
   {
    if(request->procmeter_bar.grid_min<0)
       new->procmeter_bar.grid_min=-request->procmeter_bar.grid_min,
       new->procmeter_bar.grid_drawn=0;
    else
       new->procmeter_bar.grid_drawn=1;
    if(request->procmeter_bar.grid_min==0)
       new->procmeter_bar.grid_min=1;

    if(request->procmeter_bar.grid_min>request->procmeter_bar.grid_max && request->procmeter_bar.grid_max)
       new->procmeter_bar.grid_min=request->procmeter_bar.grid_max;

    if(new->procmeter_bar.grid_min>=new->procmeter_bar.grid_num)
       new->procmeter_bar.grid_num=new->procmeter_bar.grid_min;

    redraw=True;
   }

 if(request->procmeter_bar.grid_max!=current->procmeter_bar.grid_max)
   {
    if(request->procmeter_bar.grid_max<0)
       new->procmeter_bar.grid_max=0;

    if(request->procmeter_bar.grid_max && request->procmeter_bar.grid_max<new->procmeter_bar.grid_min)
       new->procmeter_bar.grid_max=new->procmeter_bar.grid_min;

    redraw=True;
   }

 if(redraw)
    BarResize(new);

 return(redraw);
}


/*++++++++++++++++++++++++++++++++++++++
  Resize the ProcMeter Bar Widget.

  ProcMeterBarWidget w The Widget that is resized.
  ++++++++++++++++++++++++++++++++++++++*/

static void Resize(ProcMeterBarWidget w)
{
 BarResize(w);
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Widget.

  ProcMeterBarWidget w The Widget to redisplay.

  XEvent *event The event that caused the redisplay.

  Region region The region that was exposed.
  ++++++++++++++++++++++++++++++++++++++*/

static void Redisplay(ProcMeterBarWidget w,XEvent *event,Region region)
{
 if(w->core.visible)
    BarUpdate(w);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform all of the sizing on the Widget when it is created/resized.

  ProcMeterBarWidget w The Widget to resize.
  ++++++++++++++++++++++++++++++++++++++*/

static void BarResize(ProcMeterBarWidget w)
{
 ProcMeterGenericResize((ProcMeterGenericWidget)w);

 w->procmeter_generic.label_x=2;

 /* The grid parts. */

 w->procmeter_bar.grid_units_x=w->core.width-XTextWidth(w->procmeter_generic.label_font,w->procmeter_bar.grid_units,(int)strlen(w->procmeter_bar.grid_units));

 w->procmeter_bar.grid_maxvis=w->core.width/3;

 if(w->procmeter_generic.label_pos==ProcMeterLabelTop)
    w->procmeter_generic.body_start=w->procmeter_generic.label_height;
 else
    w->procmeter_generic.body_start=0;

 if(w->procmeter_bar.grid_num>w->procmeter_bar.grid_maxvis && w->procmeter_bar.grid_drawn)
    w->procmeter_bar.grid_drawn=-1;
 if(w->procmeter_bar.grid_num<=w->procmeter_bar.grid_maxvis && w->procmeter_bar.grid_drawn)
    w->procmeter_bar.grid_drawn=1;

 if(XtIsSubclass(XtParent(w),panedWidgetClass))
    XawPanedSetMinMax((Widget)w,20,20);
}


/*++++++++++++++++++++++++++++++++++++++
  Update the display.

  ProcMeterBarWidget w The Widget to update.
  ++++++++++++++++++++++++++++++++++++++*/

static void BarUpdate(ProcMeterBarWidget w)
{
 if(w->core.visible)
   {
    int i;
    int scale=PROCMETER_GRAPH_SCALE*w->procmeter_bar.grid_num;
    Position pos;

    ProcMeterGenericUpdate((ProcMeterGenericWidget)w);

    if(w->procmeter_generic.label_pos!=ProcMeterLabelNone)
       XDrawString(XtDisplay(w),XtWindow(w),w->procmeter_generic.label_gc,
                   w->procmeter_bar.grid_units_x,w->procmeter_generic.label_y,
                   w->procmeter_bar.grid_units,(int)strlen(w->procmeter_bar.grid_units));

    pos=w->procmeter_bar.data*w->core.width/scale;

    XFillRectangle(XtDisplay(w),XtWindow(w),w->procmeter_generic.body_gc,
                   0  ,2+w->procmeter_generic.body_start,
                   pos,w->procmeter_generic.body_height-4);

    if(w->procmeter_bar.grid_drawn==1)
       for(i=1;i<w->procmeter_bar.grid_num;i++)
         {
          pos=i*w->core.width/w->procmeter_bar.grid_num;
          XDrawLine(XtDisplay(w),XtWindow(w),w->procmeter_bar.grid_gc,
                    pos,w->procmeter_generic.body_start,
                    pos,w->procmeter_generic.body_height+w->procmeter_generic.body_start);
         }
    else
       if(w->procmeter_bar.grid_drawn==-1)
         {
          pos=w->procmeter_bar.grid_maxvis*w->core.width/w->procmeter_bar.grid_num;
          XDrawLine(XtDisplay(w),XtWindow(w),w->procmeter_bar.grid_gc,
                    pos,w->procmeter_generic.body_start,
                    pos,w->procmeter_generic.body_height+w->procmeter_generic.body_start);
         }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Add a data point to the ProcMeter Bar Widget.

  Widget pmw The ProcMeter Bar Widget.

  unsigned short datum The data point to add.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterBarWidgetAddDatum(Widget pmw,unsigned short datum)
{
 ProcMeterBarWidget w=(ProcMeterBarWidget)pmw;
 int new_grid_num;

 w->procmeter_bar.data=datum;

 new_grid_num=(datum+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;

 if(new_grid_num<w->procmeter_bar.grid_min)
    new_grid_num=w->procmeter_bar.grid_min;
 if(w->procmeter_bar.grid_max && new_grid_num>w->procmeter_bar.grid_max)
    new_grid_num=w->procmeter_bar.grid_max;

 if(new_grid_num!=w->procmeter_bar.grid_num)
   {
    w->procmeter_bar.grid_num=new_grid_num;

    if(w->procmeter_bar.grid_num>w->procmeter_bar.grid_maxvis && w->procmeter_bar.grid_drawn)
       w->procmeter_bar.grid_drawn=-1;
    if(w->procmeter_bar.grid_num<=w->procmeter_bar.grid_maxvis && w->procmeter_bar.grid_drawn)
       w->procmeter_bar.grid_drawn=1;
   }

 BarUpdate(w);
}
