/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMGraph.c,v 1.8 2001-01-04 19:26:46 amb Exp $

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
static void Destroy(ProcMeterGraphWidget pmw);
static Boolean SetValues(ProcMeterGraphWidget current,ProcMeterGraphWidget request,ProcMeterGraphWidget new);
static void Resize(ProcMeterGraphWidget pmw);
static void Redisplay(ProcMeterGraphWidget pmw,XEvent *event,Region region);
static void GraphResize(ProcMeterGraphWidget pmw);
static void GraphUpdate(ProcMeterGraphWidget pmw,Boolean all);

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
  NULL,
  NULL
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

  ProcMeterGraphWidget pmw The Widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void Destroy(ProcMeterGraphWidget pmw)
{
 XtReleaseGC((Widget)pmw,pmw->procmeter_graph.grid_gc);
 XtFree((XtPointer)pmw->procmeter_graph.grid_units);
 XtFree((XtPointer)pmw->procmeter_graph.data);
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

  ProcMeterGraphWidget pmw The Widget that is resized.
  ++++++++++++++++++++++++++++++++++++++*/

static void Resize(ProcMeterGraphWidget pmw)
{
 if(pmw->procmeter_graph.data_num!=pmw->core.width)
   {
    int i,old_num=pmw->procmeter_graph.data_num;
    unsigned short* old_data=pmw->procmeter_graph.data;

    pmw->procmeter_graph.data_num=pmw->core.width;
    pmw->procmeter_graph.data=(unsigned short*)XtCalloc(pmw->procmeter_graph.data_num,sizeof(unsigned short));

    if(pmw->procmeter_graph.data_num<old_num)
       i=pmw->procmeter_graph.data_num;
    else
       i=old_num;

    for(;i>0;i--)
       pmw->procmeter_graph.data[(-i+pmw->procmeter_graph.data_num)%pmw->procmeter_graph.data_num]=old_data[(pmw->procmeter_graph.data_index-i+old_num)%old_num];

    pmw->procmeter_graph.data_index=0;

    XtFree((XtPointer)old_data);

    for(i=pmw->procmeter_graph.data_max=0;i<pmw->procmeter_graph.data_num;i++)
       if(pmw->procmeter_graph.data[i]>pmw->procmeter_graph.data_max)
          pmw->procmeter_graph.data_max=pmw->procmeter_graph.data[i];

    pmw->procmeter_graph.grid_num=(pmw->procmeter_graph.data_max+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;

    if(pmw->procmeter_graph.grid_num<pmw->procmeter_graph.grid_min)
       pmw->procmeter_graph.grid_num=pmw->procmeter_graph.grid_min;
    if(pmw->procmeter_graph.grid_max && pmw->procmeter_graph.grid_num>pmw->procmeter_graph.grid_max)
       pmw->procmeter_graph.grid_num=pmw->procmeter_graph.grid_max;
   }

 GraphResize(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Widget.

  ProcMeterGraphWidget pmw The Widget to redisplay.

  XEvent *event The event that caused the redisplay.

  Region region The region that was exposed.
  ++++++++++++++++++++++++++++++++++++++*/

static void Redisplay(ProcMeterGraphWidget pmw,XEvent *event,Region region)
{
 if(pmw->core.visible)
    GraphUpdate(pmw,True);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform all of the sizing on the Widget when it is created/resized.

  ProcMeterGraphWidget pmw The Widget to resize.
  ++++++++++++++++++++++++++++++++++++++*/

static void GraphResize(ProcMeterGraphWidget pmw)
{
 (*procMeterGenericClassRec.procmeter_generic_class.resize)((ProcMeterGenericWidget)pmw);

 pmw->procmeter_generic.label_x=2;

 /* The grid parts. */

 pmw->procmeter_graph.grid_units_x=pmw->core.width-XTextWidth(pmw->procmeter_generic.label_font,pmw->procmeter_graph.grid_units,(int)strlen(pmw->procmeter_graph.grid_units));

 pmw->procmeter_graph.grid_maxvis=pmw->procmeter_generic.body_height/3;

 if(pmw->procmeter_generic.label_pos==ProcMeterLabelTop)
    pmw->procmeter_generic.body_start=pmw->procmeter_generic.label_height;
 else
    pmw->procmeter_generic.body_start=0;

 if(pmw->procmeter_graph.grid_num>pmw->procmeter_graph.grid_maxvis && pmw->procmeter_graph.grid_drawn)
    pmw->procmeter_graph.grid_drawn=-1;
 if(pmw->procmeter_graph.grid_num<=pmw->procmeter_graph.grid_maxvis && pmw->procmeter_graph.grid_drawn)
    pmw->procmeter_graph.grid_drawn=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Update the display.

  ProcMeterGraphWidget pmw The Widget to update.

  Boolean all Indicates if the whole widget is to be updated.
  ++++++++++++++++++++++++++++++++++++++*/

static void GraphUpdate(ProcMeterGraphWidget pmw,Boolean all)
{
 if(pmw->core.visible)
   {
    int i;
    int scale=PROCMETER_GRAPH_SCALE*pmw->procmeter_graph.grid_num;
    unsigned short val;
    Position pos;

    if(all)
      {
       (*procMeterGenericClassRec.procmeter_generic_class.update)((ProcMeterGenericWidget)pmw);

       if(pmw->procmeter_generic.label_pos!=ProcMeterLabelNone)
          XDrawString(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.label_gc,
                      pmw->procmeter_graph.grid_units_x,pmw->procmeter_generic.label_y,
                      pmw->procmeter_graph.grid_units,(int)strlen(pmw->procmeter_graph.grid_units));

       for(i=0;i<pmw->procmeter_graph.data_num;i++)
         {
          val=pmw->procmeter_graph.data[(i+pmw->procmeter_graph.data_index)%pmw->procmeter_graph.data_num];
          pos=val*pmw->procmeter_generic.body_height/scale;

          if(pmw->procmeter_graph.line_solid)
             XDrawLine(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.body_gc,
                       i,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start,
                       i,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-pos);
          else if(i)
            {
             unsigned short oldval=pmw->procmeter_graph.data[(i-1+pmw->procmeter_graph.data_index)%pmw->procmeter_graph.data_num];
             Position oldpos=oldval*pmw->procmeter_generic.body_height/scale;

             XDrawLine(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.body_gc,
                       i,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-oldpos,
                       i,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-pos);
            }
         }

       if(pmw->procmeter_graph.grid_drawn==1)
          for(i=1;i<pmw->procmeter_graph.grid_num;i++)
            {
             pos=i*pmw->procmeter_generic.body_height/pmw->procmeter_graph.grid_num;
             XDrawLine(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_graph.grid_gc,
                       0              ,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-pos,
                       pmw->core.width,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-pos);
            }
       else
          if(pmw->procmeter_graph.grid_drawn==-1)
            {
             pos=pmw->procmeter_graph.grid_maxvis*pmw->procmeter_generic.body_height/pmw->procmeter_graph.grid_num;
             XDrawLine(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_graph.grid_gc,
                       0              ,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-pos,
                       pmw->core.width,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-pos);
            }
      }
    else
      {
       val=pmw->procmeter_graph.data[(pmw->procmeter_graph.data_num-1+pmw->procmeter_graph.data_index)%pmw->procmeter_graph.data_num];
       pos=val*pmw->procmeter_generic.body_height/scale;

       XCopyArea(XtDisplay(pmw),XtWindow(pmw),XtWindow(pmw),pmw->procmeter_graph.grid_gc,
                 1,pmw->procmeter_generic.body_start,(unsigned)(pmw->core.width-1),pmw->procmeter_generic.body_height,
                 0,pmw->procmeter_generic.body_start);

       XClearArea(XtDisplay(pmw),XtWindow(pmw),
                  pmw->core.width-1,pmw->procmeter_generic.body_start,1,pmw->procmeter_generic.body_height,
                  False);

       if(pmw->procmeter_graph.line_solid)
          XDrawLine(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.body_gc,
                    (signed)(pmw->procmeter_graph.data_num-1),pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start,
                    (signed)(pmw->procmeter_graph.data_num-1),pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-pos);
       else
         {
          unsigned short oldval=pmw->procmeter_graph.data[(pmw->procmeter_graph.data_num-2+pmw->procmeter_graph.data_index)%pmw->procmeter_graph.data_num];
          Position oldpos=oldval*pmw->procmeter_generic.body_height/scale;

          XDrawLine(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.body_gc,
                    (signed)(pmw->procmeter_graph.data_num-1),pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-oldpos,
                    (signed)(pmw->procmeter_graph.data_num-1),pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-pos);
         }

       if(pmw->procmeter_graph.grid_drawn==1)
          for(i=1;i<pmw->procmeter_graph.grid_num;i++)
            {
             pos=i*pmw->procmeter_generic.body_height/pmw->procmeter_graph.grid_num;
             XDrawPoint(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_graph.grid_gc,
                        pmw->core.width-1,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-pos);
            }
       else
          if(pmw->procmeter_graph.grid_drawn==-1)
            {
             pos=pmw->procmeter_graph.grid_maxvis*pmw->procmeter_generic.body_height/pmw->procmeter_graph.grid_num;
             XDrawPoint(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_graph.grid_gc,
                        pmw->core.width-1,pmw->procmeter_generic.body_height+pmw->procmeter_generic.body_start-pos);
            }
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Add a data point to the ProcMeter Graph Widget.

  Widget w The ProcMeter Graph Widget.

  unsigned short datum The data point to add.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGraphAddDatum(Widget w,unsigned short datum)
{
 ProcMeterGraphWidget pmw=(ProcMeterGraphWidget)w;
 unsigned short old_datum,new_data_max=pmw->procmeter_graph.data_max;
 int i;

 old_datum=pmw->procmeter_graph.data[pmw->procmeter_graph.data_index];
 pmw->procmeter_graph.data[pmw->procmeter_graph.data_index]=datum;

 pmw->procmeter_graph.data_index=(pmw->procmeter_graph.data_index+1)%pmw->procmeter_graph.data_num;

 if(datum>new_data_max)
    new_data_max=datum;
 else
    if(old_datum==new_data_max)
       for(i=new_data_max=0;i<pmw->procmeter_graph.data_num;i++)
          if(pmw->procmeter_graph.data[i]>new_data_max)
             new_data_max=pmw->procmeter_graph.data[i];

 if(new_data_max!=pmw->procmeter_graph.data_max)
   {
    int new_grid_num=(new_data_max+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;

    if(new_grid_num<pmw->procmeter_graph.grid_min)
       new_grid_num=pmw->procmeter_graph.grid_min;
    if(pmw->procmeter_graph.grid_max && new_grid_num>pmw->procmeter_graph.grid_max)
       new_grid_num=pmw->procmeter_graph.grid_max;

    pmw->procmeter_graph.data_max=new_data_max;

    if(new_grid_num!=pmw->procmeter_graph.grid_num)
      {
       pmw->procmeter_graph.grid_num=new_grid_num;

       if(pmw->procmeter_graph.grid_num>pmw->procmeter_graph.grid_maxvis && pmw->procmeter_graph.grid_drawn)
          pmw->procmeter_graph.grid_drawn=-1;
       if(pmw->procmeter_graph.grid_num<=pmw->procmeter_graph.grid_maxvis && pmw->procmeter_graph.grid_drawn)
          pmw->procmeter_graph.grid_drawn=1;

       GraphUpdate(pmw,True);
      }
   }

 GraphUpdate(pmw,False);
}
