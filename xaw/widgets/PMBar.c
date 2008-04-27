/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMBar.c,v 1.5 2008-04-27 15:21:30 amb Exp $

  ProcMeter Bar Widget Source file (for ProcMeter3 3.5b).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996-2008 Andrew M. Bishop
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
#include "PMBarP.h"

#include "procmeter.h"

static void Initialize(ProcMeterBarWidget request,ProcMeterBarWidget new);
static void Destroy(ProcMeterBarWidget pmw);
static Boolean SetValues(ProcMeterBarWidget current,ProcMeterBarWidget request,ProcMeterBarWidget new);
static void Resize(ProcMeterBarWidget pmw);
static void Redisplay(ProcMeterBarWidget pmw,XEvent *event,Region region);
static void BarResize(ProcMeterBarWidget pmw);
static void BarUpdate(ProcMeterBarWidget pmw,Boolean all);

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
  NULL,
  NULL
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
 int i;
 XGCValues values;

 /* The grid parts. */

 new->procmeter_bar.grid_units=XtNewString(request->procmeter_bar.grid_units);

 values.foreground=new->procmeter_bar.grid_pixel;
 values.background=new->core.background_pixel;
 new->procmeter_bar.grid_gc=XtGetGC((Widget)new,GCForeground|GCBackground,&values);

 if(request->procmeter_bar.grid_min<0)
   {
    new->procmeter_bar.grid_min=-request->procmeter_bar.grid_min;
    new->procmeter_bar.grid_drawn=0;
   }
 else if(request->procmeter_bar.grid_min>0)
   {
    new->procmeter_bar.grid_min=request->procmeter_bar.grid_min;
    new->procmeter_bar.grid_drawn=1;
   }
 else /* if(request->procmeter_bar.grid_min==0) */
   {
    new->procmeter_bar.grid_min=1;
    new->procmeter_bar.grid_drawn=1;
   }

 if(request->procmeter_bar.grid_max<0)
    new->procmeter_bar.grid_max=0;
 else
    new->procmeter_bar.grid_max=request->procmeter_bar.grid_max;

 if(new->procmeter_bar.grid_max && new->procmeter_bar.grid_max<new->procmeter_bar.grid_min)
    new->procmeter_bar.grid_max=new->procmeter_bar.grid_min;

 new->procmeter_bar.grid_num=new->procmeter_bar.grid_min;

 /* The data parts. */

 for(i=0;i<sizeof(new->procmeter_bar.data)/sizeof(new->procmeter_bar.data[0]);i++)
    new->procmeter_bar.data[i]=0;

 new->procmeter_bar.data_index=0;

 new->procmeter_bar.data_sum=0;

 /* The rest of the sizing. */

 BarResize(new);
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a ProcMeter Bar Widget.

  ProcMeterBarWidget pmw The Widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void Destroy(ProcMeterBarWidget pmw)
{
 XtReleaseGC((Widget)pmw,pmw->procmeter_bar.grid_gc);
 XtFree((XtPointer)pmw->procmeter_bar.grid_units);
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
      {
       new->procmeter_bar.grid_min=-request->procmeter_bar.grid_min;
       new->procmeter_bar.grid_drawn=0;
      }
    else if(request->procmeter_bar.grid_min>0)
      {
       new->procmeter_bar.grid_min=request->procmeter_bar.grid_min;
       new->procmeter_bar.grid_drawn=1;
      }
    else /* if(request->procmeter_bar.grid_min==0) */
      {
       new->procmeter_bar.grid_min=1;
       new->procmeter_bar.grid_drawn=1;
      }

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
    else
       new->procmeter_bar.grid_max=request->procmeter_bar.grid_max;

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

  ProcMeterBarWidget pmw The Widget that is resized.
  ++++++++++++++++++++++++++++++++++++++*/

static void Resize(ProcMeterBarWidget pmw)
{
 BarResize(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Widget.

  ProcMeterBarWidget pmw The Widget to redisplay.

  XEvent *event The event that caused the redisplay.

  Region region The region that was exposed.
  ++++++++++++++++++++++++++++++++++++++*/

static void Redisplay(ProcMeterBarWidget pmw,XEvent *event,Region region)
{
 if(pmw->core.visible)
    BarUpdate(pmw,True);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform all of the sizing on the Widget when it is created/resized.

  ProcMeterBarWidget pmw The Widget to resize.
  ++++++++++++++++++++++++++++++++++++++*/

static void BarResize(ProcMeterBarWidget pmw)
{
 (*procMeterGenericClassRec.procmeter_generic_class.resize)((ProcMeterGenericWidget)pmw);

 pmw->procmeter_generic.label_x=2;

 /* The grid parts. */

 pmw->procmeter_bar.grid_units_x=pmw->core.width-XTextWidth(pmw->procmeter_generic.label_font,pmw->procmeter_bar.grid_units,(int)strlen(pmw->procmeter_bar.grid_units));

 pmw->procmeter_bar.grid_maxvis=pmw->core.width/3;

 if(pmw->procmeter_generic.label_pos==ProcMeterLabelTop)
    pmw->procmeter_generic.body_start=pmw->procmeter_generic.label_height;
 else
    pmw->procmeter_generic.body_start=0;

 if(pmw->procmeter_bar.grid_num>pmw->procmeter_bar.grid_maxvis && pmw->procmeter_bar.grid_drawn)
    pmw->procmeter_bar.grid_drawn=-1;
 if(pmw->procmeter_bar.grid_num<=pmw->procmeter_bar.grid_maxvis && pmw->procmeter_bar.grid_drawn)
    pmw->procmeter_bar.grid_drawn=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Update the display.

  ProcMeterBarWidget pmw The Widget to update.

  Boolean all Indicates if it all is to be updated including the generic parts.
  ++++++++++++++++++++++++++++++++++++++*/

static void BarUpdate(ProcMeterBarWidget pmw,Boolean all)
{
 if(pmw->core.visible)
   {
    int i;
    int scale=PROCMETER_GRAPH_SCALE*pmw->procmeter_bar.grid_num;
    Position pos;
    Position top_average_bottom,bottom_average_top,average_size;

    if(all)
      {
       (*procMeterGenericClassRec.procmeter_generic_class.update)((ProcMeterGenericWidget)pmw);

       if(pmw->procmeter_generic.label_pos!=ProcMeterLabelNone)
          XDrawString(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.label_gc,
                      pmw->procmeter_bar.grid_units_x,pmw->procmeter_generic.label_y,
                      pmw->procmeter_bar.grid_units,(int)strlen(pmw->procmeter_bar.grid_units));

      }
    else
       XClearArea(XtDisplay(pmw),XtWindow(pmw),
                  0              ,pmw->procmeter_generic.body_start,
                  pmw->core.width,pmw->procmeter_generic.body_height,False);

    pos=pmw->procmeter_bar.data_sum*pmw->core.width/(scale*2);

    top_average_bottom=pmw->procmeter_generic.body_start+2*(pmw->procmeter_generic.body_height>>3);
    bottom_average_top=pmw->procmeter_generic.body_start+pmw->procmeter_generic.body_height-2*(pmw->procmeter_generic.body_height>>3);
    average_size=pmw->procmeter_generic.body_height>>3;

    XFillRectangle(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.body_gc,
                   pos-average_size,top_average_bottom-average_size,
                   average_size    ,average_size);

    XFillRectangle(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.body_gc,
                   pos-average_size,bottom_average_top,
                   average_size    ,average_size);

    pos=pmw->procmeter_bar.data[pmw->procmeter_bar.data_index]*pmw->core.width/scale;

    XFillRectangle(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.body_gc,
                   0  ,top_average_bottom+1,
                   pos,bottom_average_top-top_average_bottom-2);

    if(pmw->procmeter_bar.grid_drawn==1)
       for(i=1;i<pmw->procmeter_bar.grid_num;i++)
         {
          pos=i*pmw->core.width/pmw->procmeter_bar.grid_num;
          XDrawLine(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_bar.grid_gc,
                    pos,pmw->procmeter_generic.body_start,
                    pos,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start);
         }
    else
       if(pmw->procmeter_bar.grid_drawn==-1)
         {
          pos=pmw->procmeter_bar.grid_maxvis*pmw->core.width/pmw->procmeter_bar.grid_num;
          XDrawLine(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_bar.grid_gc,
                    pos,pmw->procmeter_generic.body_start,
                    pos,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start);
         }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Add a data point to the ProcMeter Bar Widget.

  Widget w The ProcMeter Bar Widget.

  unsigned short datum The data point to add.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterBarAddDatum(Widget w,unsigned short datum)
{
 ProcMeterBarWidget pmw=(ProcMeterBarWidget)w;
 int new_grid_num;
 unsigned short old_datum;

 pmw->procmeter_bar.data_index++;
 if(pmw->procmeter_bar.data_index==8)
    pmw->procmeter_bar.data_index=0;

 old_datum=pmw->procmeter_bar.data[pmw->procmeter_bar.data_index];
 pmw->procmeter_bar.data[pmw->procmeter_bar.data_index]=datum;

 pmw->procmeter_bar.data_sum=(pmw->procmeter_bar.data_sum>>1)+datum-(old_datum>>8);

 if((pmw->procmeter_bar.data_sum/2)>datum)
    new_grid_num=((pmw->procmeter_bar.data_sum/2)+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;
 else
    new_grid_num=(datum+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;

 if(new_grid_num<pmw->procmeter_bar.grid_min)
    new_grid_num=pmw->procmeter_bar.grid_min;
 if(pmw->procmeter_bar.grid_max && new_grid_num>pmw->procmeter_bar.grid_max)
    new_grid_num=pmw->procmeter_bar.grid_max;

 if(new_grid_num!=pmw->procmeter_bar.grid_num)
   {
    pmw->procmeter_bar.grid_num=new_grid_num;

    if(pmw->procmeter_bar.grid_num>pmw->procmeter_bar.grid_maxvis && pmw->procmeter_bar.grid_drawn)
       pmw->procmeter_bar.grid_drawn=-1;
    if(pmw->procmeter_bar.grid_num<=pmw->procmeter_bar.grid_maxvis && pmw->procmeter_bar.grid_drawn)
       pmw->procmeter_bar.grid_drawn=1;
   }

 BarUpdate(pmw,False);
}
