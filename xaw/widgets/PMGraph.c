/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMGraph.c,v 1.7 2000-12-16 17:02:43 amb Exp $

  ProcMeter Graph Widget Source file (for ProcMeter3 3.3).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,99,2000 Andrew M. Bishop
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
#include "PMGraphP.h"

#include "procmeter.h"

static void Initialize(ProcMeterGraphWidget request,ProcMeterGraphWidget new);
static void Destroy(ProcMeterGraphWidget w);
static Boolean SetValues(ProcMeterGraphWidget current,ProcMeterGraphWidget request,ProcMeterGraphWidget new);
static void Resize(ProcMeterGraphWidget w);
static void Redisplay(ProcMeterGraphWidget w,XEvent *event,Region region);
static void GraphResize(ProcMeterGraphWidget w);
static void GraphUpdate(ProcMeterGraphWidget w,Boolean all);

static XtResource resources[]=
{
 /* The line style */

 {XtNsolid, XtCSolid, XtRBoolean, sizeof(Boolean),
  XtOffset(ProcMeterGraphWidget,procmeter_graph.line_solid), XtRString, "TRUE" },

 /* The grid parts. */

 {XtNgridUnits, XtCLabel, XtRString, sizeof(XtPointer),
  XtOffset(ProcMeterGraphWidget,procmeter_graph.grid_units), XtRString, "" },
 {XtNgridForeground, XtCForeground, XtRPixel, sizeof(Pixel),
  XtOffset(ProcMeterGraphWidget,procmeter_graph.grid_pixel),XtRString,XtDefaultBackground},
 {XtNgridMin, XtCGridMin, XtRInt, sizeof(int),
  XtOffset(ProcMeterGraphWidget,procmeter_graph.grid_min), XtRString, "1" },
 {XtNgridMax, XtCGridMax, XtRInt, sizeof(int),
  XtOffset(ProcMeterGraphWidget,procmeter_graph.grid_max), XtRString, "0" }
};

/*+ The actual ProcMeter Graph Widget Class Record. +*/
ProcMeterGraphClassRec procMeterGraphClassRec=
{
 {
  (WidgetClass) &procMeterGenericClassRec,
  "ProcMeterGraph",
  sizeof(ProcMeterGraphRec),
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

/*+ The actual ProcMeter Graph Widget Class Record masquerading as a WidgetClass type. +*/
WidgetClass procMeterGraphWidgetClass=(WidgetClass)&procMeterGraphClassRec;


/*++++++++++++++++++++++++++++++++++++++
  Initialise a new ProcMeter Graph Widget.

  ProcMeterGraphWidget request The requested parameters.

  ProcMeterGraphWidget new The new parameters that are to be filled in.
  ++++++++++++++++++++++++++++++++++++++*/

static void Initialize(ProcMeterGraphWidget request,ProcMeterGraphWidget new)
{
 XGCValues values;

 /* The grid parts. */

 new->procmeter_graph.grid_units=XtNewString(request->procmeter_graph.grid_units);

 values.foreground=new->procmeter_graph.grid_pixel;
 values.background=new->core.background_pixel;
 new->procmeter_graph.grid_gc=XtGetGC((Widget)new,GCForeground|GCBackground,&values);

 if(request->procmeter_graph.grid_min<0)
   {
    new->procmeter_graph.grid_min=-request->procmeter_graph.grid_min;
    new->procmeter_graph.grid_drawn=0;
   }
 else
    new->procmeter_graph.grid_drawn=1;
 if(request->procmeter_graph.grid_min==0)
    new->procmeter_graph.grid_min=1;

 if(request->procmeter_graph.grid_max<0)
    new->procmeter_graph.grid_max=0;

 if(new->procmeter_graph.grid_max && new->procmeter_graph.grid_max<new->procmeter_graph.grid_min)
    new->procmeter_graph.grid_max=new->procmeter_graph.grid_min;

 new->procmeter_graph.grid_num=new->procmeter_graph.grid_min;

 /* The data parts. */

 new->procmeter_graph.data_num=new->core.width;
 new->procmeter_graph.data=(unsigned short*)XtCalloc(new->procmeter_graph.data_num,sizeof(unsigned short));
 new->procmeter_graph.data_max=0;
 new->procmeter_graph.data_index=0;

 /* The rest of the sizing. */

 GraphResize(new);
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a ProcMeter Graph Widget.

  ProcMeterGraphWidget w The Widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void Destroy(ProcMeterGraphWidget w)
{
 XtReleaseGC((Widget)w,w->procmeter_graph.grid_gc);
 XtFree((XtPointer)w->procmeter_graph.grid_units);
 XtFree((XtPointer)w->procmeter_graph.data);
}


/*++++++++++++++++++++++++++++++++++++++
  The setvalues procedure that is used to set the values internal to the Widget.

  Boolean SetValues Returns True if the Widget is to be redrawn.

  ProcMeterGraphWidget current The current Widget values.

  ProcMeterGraphWidget request The requested Widget values.

  ProcMeterGraphWidget new The new Widget values to be set up.
  ++++++++++++++++++++++++++++++++++++++*/

static Boolean SetValues(ProcMeterGraphWidget current,ProcMeterGraphWidget request,ProcMeterGraphWidget new)
{
 Boolean redraw=False;

 /* The line style */

 if(request->procmeter_graph.line_solid!=current->procmeter_graph.line_solid)
    redraw=True;

 /* The grid parts. */

 if(request->procmeter_graph.grid_units!=current->procmeter_graph.grid_units)
   {
    XtFree((XtPointer)new->procmeter_graph.grid_units);
    new->procmeter_graph.grid_units=XtNewString(request->procmeter_graph.grid_units);

    redraw=True;
   }

 if(request->procmeter_graph.grid_pixel!=current->procmeter_graph.grid_pixel)
   {
    XGCValues xgcv;

    XGetGCValues(XtDisplay(new),new->procmeter_graph.grid_gc,GCForeground|GCBackground,&xgcv);
    XtReleaseGC((Widget)new,new->procmeter_graph.grid_gc);
    xgcv.foreground=request->procmeter_graph.grid_pixel;
    xgcv.background=request->core.background_pixel;
    new->procmeter_graph.grid_gc=XtGetGC((Widget)new,GCForeground|GCBackground,&xgcv);

    redraw=True;
   }

 if(request->procmeter_graph.grid_min!=current->procmeter_graph.grid_min)
   {
    if(request->procmeter_graph.grid_min<0)
      {
       new->procmeter_graph.grid_min=-request->procmeter_graph.grid_min;
       new->procmeter_graph.grid_drawn=0;
      }
    else
       new->procmeter_graph.grid_drawn=1;
    if(request->procmeter_graph.grid_min==0)
       new->procmeter_graph.grid_min=1;

    if(request->procmeter_graph.grid_min>request->procmeter_graph.grid_max && request->procmeter_graph.grid_max)
       new->procmeter_graph.grid_min=request->procmeter_graph.grid_max;

    if(new->procmeter_graph.grid_min>=new->procmeter_graph.grid_num)
       new->procmeter_graph.grid_num=new->procmeter_graph.grid_min;

    redraw=True;
   }

 if(request->procmeter_graph.grid_max!=current->procmeter_graph.grid_max)
   {
    if(request->procmeter_graph.grid_max<0)
       new->procmeter_graph.grid_max=0;

    if(request->procmeter_graph.grid_max && request->procmeter_graph.grid_max<new->procmeter_graph.grid_min)
       new->procmeter_graph.grid_max=new->procmeter_graph.grid_min;

    redraw=True;
   }

 if(redraw)
    GraphResize(new);

 return(redraw);
}


/*++++++++++++++++++++++++++++++++++++++
  Resize the ProcMeter Graph Widget.

  ProcMeterGraphWidget w The Widget that is resized.
  ++++++++++++++++++++++++++++++++++++++*/

static void Resize(ProcMeterGraphWidget w)
{
 if(w->procmeter_graph.data_num!=w->core.width)
   {
    int i,old_num=w->procmeter_graph.data_num;
    unsigned short* old_data=w->procmeter_graph.data;

    w->procmeter_graph.data_num=w->core.width;
    w->procmeter_graph.data=(unsigned short*)XtCalloc(w->procmeter_graph.data_num,sizeof(unsigned short));

    if(w->procmeter_graph.data_num<old_num)
       i=w->procmeter_graph.data_num;
    else
       i=old_num;

    for(;i>0;i--)
       w->procmeter_graph.data[(-i+w->procmeter_graph.data_num)%w->procmeter_graph.data_num]=old_data[(w->procmeter_graph.data_index-i+old_num)%old_num];

    w->procmeter_graph.data_index=0;

    XtFree((XtPointer)old_data);

    for(i=w->procmeter_graph.data_max=0;i<w->procmeter_graph.data_num;i++)
       if(w->procmeter_graph.data[i]>w->procmeter_graph.data_max)
          w->procmeter_graph.data_max=w->procmeter_graph.data[i];

    w->procmeter_graph.grid_num=(w->procmeter_graph.data_max+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;

    if(w->procmeter_graph.grid_num<w->procmeter_graph.grid_min)
       w->procmeter_graph.grid_num=w->procmeter_graph.grid_min;
    if(w->procmeter_graph.grid_max && w->procmeter_graph.grid_num>w->procmeter_graph.grid_max)
       w->procmeter_graph.grid_num=w->procmeter_graph.grid_max;
   }

 GraphResize(w);
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Widget.

  ProcMeterGraphWidget w The Widget to redisplay.

  XEvent *event The event that caused the redisplay.

  Region region The region that was exposed.
  ++++++++++++++++++++++++++++++++++++++*/

static void Redisplay(ProcMeterGraphWidget w,XEvent *event,Region region)
{
 if(w->core.visible)
    GraphUpdate(w,True);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform all of the sizing on the Widget when it is created/resized.

  ProcMeterGraphWidget w The Widget to resize.
  ++++++++++++++++++++++++++++++++++++++*/

static void GraphResize(ProcMeterGraphWidget w)
{
 ProcMeterGenericResize((ProcMeterGenericWidget)w);

 w->procmeter_generic.label_x=2;

 /* The grid parts. */

 w->procmeter_graph.grid_units_x=w->core.width-XTextWidth(w->procmeter_generic.label_font,w->procmeter_graph.grid_units,(int)strlen(w->procmeter_graph.grid_units));

 w->procmeter_graph.grid_maxvis=w->procmeter_generic.body_height/3;

 if(w->procmeter_generic.label_pos==ProcMeterLabelTop)
    w->procmeter_generic.body_start=w->procmeter_generic.label_height;
 else
    w->procmeter_generic.body_start=0;

 if(w->procmeter_graph.grid_num>w->procmeter_graph.grid_maxvis && w->procmeter_graph.grid_drawn)
    w->procmeter_graph.grid_drawn=-1;
 if(w->procmeter_graph.grid_num<=w->procmeter_graph.grid_maxvis && w->procmeter_graph.grid_drawn)
    w->procmeter_graph.grid_drawn=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Update the display.

  ProcMeterGraphWidget w The Widget to update.

  Boolean all Indicates if the whole widget is to be updated.
  ++++++++++++++++++++++++++++++++++++++*/

static void GraphUpdate(ProcMeterGraphWidget w,Boolean all)
{
 if(w->core.visible)
   {
    int i;
    int scale=PROCMETER_GRAPH_SCALE*w->procmeter_graph.grid_num;
    unsigned short val;
    Position pos;

    if(all)
      {
       ProcMeterGenericUpdate((ProcMeterGenericWidget)w);

       if(w->procmeter_generic.label_pos!=ProcMeterLabelNone)
          XDrawString(XtDisplay(w),XtWindow(w),w->procmeter_generic.label_gc,
                      w->procmeter_graph.grid_units_x,w->procmeter_generic.label_y,
                      w->procmeter_graph.grid_units,(int)strlen(w->procmeter_graph.grid_units));

       for(i=0;i<w->procmeter_graph.data_num;i++)
         {
          val=w->procmeter_graph.data[(i+w->procmeter_graph.data_index)%w->procmeter_graph.data_num];
          pos=val*w->procmeter_generic.body_height/scale;

          if(w->procmeter_graph.line_solid)
             XDrawLine(XtDisplay(w),XtWindow(w),w->procmeter_generic.body_gc,
                       i,w->procmeter_generic.body_height+w->procmeter_generic.body_start,
                       i,w->procmeter_generic.body_height+w->procmeter_generic.body_start-pos);
          else if(i)
            {
             unsigned short oldval=w->procmeter_graph.data[(i-1+w->procmeter_graph.data_index)%w->procmeter_graph.data_num];
             Position oldpos=oldval*w->procmeter_generic.body_height/scale;

             XDrawLine(XtDisplay(w),XtWindow(w),w->procmeter_generic.body_gc,
                       i,w->procmeter_generic.body_height+w->procmeter_generic.body_start-oldpos,
                       i,w->procmeter_generic.body_height+w->procmeter_generic.body_start-pos);
            }
         }

       if(w->procmeter_graph.grid_drawn==1)
          for(i=1;i<w->procmeter_graph.grid_num;i++)
            {
             pos=i*w->procmeter_generic.body_height/w->procmeter_graph.grid_num;
             XDrawLine(XtDisplay(w),XtWindow(w),w->procmeter_graph.grid_gc,
                       0            ,w->procmeter_generic.body_height+w->procmeter_generic.body_start-pos,
                       w->core.width,w->procmeter_generic.body_height+w->procmeter_generic.body_start-pos);
            }
       else
          if(w->procmeter_graph.grid_drawn==-1)
            {
             pos=w->procmeter_graph.grid_maxvis*w->procmeter_generic.body_height/w->procmeter_graph.grid_num;
             XDrawLine(XtDisplay(w),XtWindow(w),w->procmeter_graph.grid_gc,
                       0            ,w->procmeter_generic.body_height+w->procmeter_generic.body_start-pos,
                       w->core.width,w->procmeter_generic.body_height+w->procmeter_generic.body_start-pos);
            }
      }
    else
      {
       val=w->procmeter_graph.data[(w->procmeter_graph.data_num-1+w->procmeter_graph.data_index)%w->procmeter_graph.data_num];
       pos=val*w->procmeter_generic.body_height/scale;

       XCopyArea(XtDisplay(w),XtWindow(w),XtWindow(w),w->procmeter_graph.grid_gc,
                 1,w->procmeter_generic.body_start,(unsigned)(w->core.width-1),w->procmeter_generic.body_height,
                 0,w->procmeter_generic.body_start);

       XClearArea(XtDisplay(w),XtWindow(w),
                  w->core.width-1,w->procmeter_generic.body_start,1,w->procmeter_generic.body_height,
                  False);

       if(w->procmeter_graph.line_solid)
          XDrawLine(XtDisplay(w),XtWindow(w),w->procmeter_generic.body_gc,
                    (signed)(w->procmeter_graph.data_num-1),w->procmeter_generic.body_height+w->procmeter_generic.body_start,
                    (signed)(w->procmeter_graph.data_num-1),w->procmeter_generic.body_height+w->procmeter_generic.body_start-pos);
       else
         {
          unsigned short oldval=w->procmeter_graph.data[(w->procmeter_graph.data_num-2+w->procmeter_graph.data_index)%w->procmeter_graph.data_num];
          Position oldpos=oldval*w->procmeter_generic.body_height/scale;

          XDrawLine(XtDisplay(w),XtWindow(w),w->procmeter_generic.body_gc,
                    (signed)(w->procmeter_graph.data_num-1),w->procmeter_generic.body_height+w->procmeter_generic.body_start-oldpos,
                    (signed)(w->procmeter_graph.data_num-1),w->procmeter_generic.body_height+w->procmeter_generic.body_start-pos);
         }

       if(w->procmeter_graph.grid_drawn==1)
          for(i=1;i<w->procmeter_graph.grid_num;i++)
            {
             pos=i*w->procmeter_generic.body_height/w->procmeter_graph.grid_num;
             XDrawPoint(XtDisplay(w),XtWindow(w),w->procmeter_graph.grid_gc,
                        w->core.width-1,w->procmeter_generic.body_height+w->procmeter_generic.body_start-pos);
            }
       else
          if(w->procmeter_graph.grid_drawn==-1)
            {
             pos=w->procmeter_graph.grid_maxvis*w->procmeter_generic.body_height/w->procmeter_graph.grid_num;
             XDrawPoint(XtDisplay(w),XtWindow(w),w->procmeter_graph.grid_gc,
                        w->core.width-1,w->procmeter_generic.body_height+w->procmeter_generic.body_start-pos);
            }
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Add a data point to the ProcMeter Graph Widget.

  Widget pmw The ProcMeter Graph Widget.

  unsigned short datum The data point to add.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGraphAddDatum(Widget pmw,unsigned short datum)
{
 ProcMeterGraphWidget w=(ProcMeterGraphWidget)pmw;
 unsigned short old_datum,new_data_max=w->procmeter_graph.data_max;
 int i;

 old_datum=w->procmeter_graph.data[w->procmeter_graph.data_index];
 w->procmeter_graph.data[w->procmeter_graph.data_index]=datum;

 w->procmeter_graph.data_index=(w->procmeter_graph.data_index+1)%w->procmeter_graph.data_num;

 if(datum>new_data_max)
    new_data_max=datum;
 else
    if(old_datum==new_data_max)
       for(i=new_data_max=0;i<w->procmeter_graph.data_num;i++)
          if(w->procmeter_graph.data[i]>new_data_max)
             new_data_max=w->procmeter_graph.data[i];

 if(new_data_max!=w->procmeter_graph.data_max)
   {
    int new_grid_num=(new_data_max+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;

    if(new_grid_num<w->procmeter_graph.grid_min)
       new_grid_num=w->procmeter_graph.grid_min;
    if(w->procmeter_graph.grid_max && new_grid_num>w->procmeter_graph.grid_max)
       new_grid_num=w->procmeter_graph.grid_max;

    w->procmeter_graph.data_max=new_data_max;

    if(new_grid_num!=w->procmeter_graph.grid_num)
      {
       w->procmeter_graph.grid_num=new_grid_num;

       if(w->procmeter_graph.grid_num>w->procmeter_graph.grid_maxvis && w->procmeter_graph.grid_drawn)
          w->procmeter_graph.grid_drawn=-1;
       if(w->procmeter_graph.grid_num<=w->procmeter_graph.grid_maxvis && w->procmeter_graph.grid_drawn)
          w->procmeter_graph.grid_drawn=1;

       GraphUpdate(w,True);
      }
   }

 GraphUpdate(w,False);
}
