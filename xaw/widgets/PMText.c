/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/PMText.c,v 1.4 2001-01-04 19:26:46 amb Exp $

  ProcMeter Text Widget Source file (for ProcMeter 3.3).
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
#include <X11/Xaw/Paned.h>

#include "PMGenericP.h"
#include "PMTextP.h"

static void Initialize(ProcMeterTextWidget request,ProcMeterTextWidget new);
static void Destroy(ProcMeterTextWidget pmw);
static Boolean SetValues(ProcMeterTextWidget current,ProcMeterTextWidget request,ProcMeterTextWidget new);
static void Resize(ProcMeterTextWidget pmw);
static void Redisplay(ProcMeterTextWidget pmw,XEvent *event,Region region);
static void TextResize(ProcMeterTextWidget pmw);
static void TextUpdate(ProcMeterTextWidget pmw,Boolean all);

static XtResource resources[]=
{
 /* The text parts. */

 {XtNtext, XtCLabel, XtRString, sizeof(XtPointer),
  XtOffset(ProcMeterTextWidget,procmeter_text.text_string), XtRString, "" },
 {XtNtextFont, XtCFont, XtRFontStruct, sizeof(XFontStruct*),
  XtOffset(ProcMeterTextWidget,procmeter_text.text_font), XtRString, "-*-*-*-r-normal-sans-12-*-*-*-p-*-*-*"}
};

/*+ The actual ProcMeter Text Widget Class Record. +*/
ProcMeterTextClassRec procMeterTextClassRec=
{
 {
  (WidgetClass) &procMeterGenericClassRec,
  "ProcMeterText",
  sizeof(ProcMeterTextRec),
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

/*+ The actual ProcMeter Text Widget Class Record masquerading as a WidgetClass type. +*/
WidgetClass procMeterTextWidgetClass=(WidgetClass)&procMeterTextClassRec;


/*++++++++++++++++++++++++++++++++++++++
  Initialise a new ProcMeter Text Widget.

  ProcMeterTextWidget request The requested parameters.

  ProcMeterTextWidget new The new parameters that are to be filled in.
  ++++++++++++++++++++++++++++++++++++++*/

static void Initialize(ProcMeterTextWidget request,ProcMeterTextWidget new)
{
 /* The text parts. */

 if(!new->procmeter_text.text_font)
    new->procmeter_text.text_font=XLoadQueryFont(XtDisplay(new),"-*-*-*-r-normal-sans-12-*-*-*-p-*-*-*");

 new->procmeter_text.text_string=XtNewString(request->procmeter_text.text_string);
 XSetFont(XtDisplay(new),new->procmeter_generic.body_gc,new->procmeter_text.text_font->fid);

 /* The rest of the sizing. */

 TextResize(new);
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a ProcMeter Text Widget.

  ProcMeterTextWidget pmw The Widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void Destroy(ProcMeterTextWidget pmw)
{
 XtFree((XtPointer)pmw->procmeter_text.text_string);
}


/*++++++++++++++++++++++++++++++++++++++
  The setvalues procedure that is used to set the values internal to the Widget.

  Boolean SetValues Returns True if the Widget is to be redrawn.

  ProcMeterTextWidget current The current Widget values.

  ProcMeterTextWidget request The requested Widget values.

  ProcMeterTextWidget new The new Widget values to be set up.
  ++++++++++++++++++++++++++++++++++++++*/

static Boolean SetValues(ProcMeterTextWidget current,ProcMeterTextWidget request,ProcMeterTextWidget new)
{
 Boolean redraw=False;

 /* The text parts. */

 if(request->procmeter_text.text_string!=current->procmeter_text.text_string)
   {
    XtFree((XtPointer)new->procmeter_text.text_string);
    new->procmeter_text.text_string=XtNewString(request->procmeter_text.text_string);

    redraw=True;
   }

 if(request->procmeter_text.text_font!=current->procmeter_text.text_font)
   {
    XSetFont(XtDisplay(new),new->procmeter_generic.body_gc,new->procmeter_text.text_font->fid);

    redraw=True;
   }

 if(redraw)
    TextResize(new);

 return(redraw);
}


/*++++++++++++++++++++++++++++++++++++++
  Resize the ProcMeter Text Widget.

  ProcMeterTextWidget pmw The Widget that is resized.
  ++++++++++++++++++++++++++++++++++++++*/

static void Resize(ProcMeterTextWidget pmw)
{
 TextResize(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Widget.

  ProcMeterTextWidget pmw The Widget to redisplay.

  XEvent *event The event that caused the redisplay.

  Region region The region that was exposed.
  ++++++++++++++++++++++++++++++++++++++*/

static void Redisplay(ProcMeterTextWidget pmw,XEvent *event,Region region)
{
 if(pmw->core.visible)
    TextUpdate(pmw,True);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform all of the sizing on the Widget when it is created/resized.

  ProcMeterTextWidget pmw The Widget to resize.
  ++++++++++++++++++++++++++++++++++++++*/

static void TextResize(ProcMeterTextWidget pmw)
{
 Dimension text_width,text_height;

 (*procMeterGenericClassRec.procmeter_generic_class.resize)((ProcMeterGenericWidget)pmw);

 /* The text parts. */

 text_width=XTextWidth(pmw->procmeter_text.text_font,pmw->procmeter_text.text_string,(int)strlen(pmw->procmeter_text.text_string));
 text_height=pmw->procmeter_text.text_font->ascent+pmw->procmeter_text.text_font->descent+2;

 pmw->procmeter_text.text_x=(pmw->core.width-text_width)/2;
 pmw->procmeter_text.text_y=pmw->procmeter_generic.body_start+1+pmw->procmeter_text.text_font->ascent;

 if(XtIsSubclass(XtParent(pmw),panedWidgetClass))
   {
    XtOrientation orient;

    XtVaGetValues(XtParent(pmw),XtNorientation,&orient,NULL);

    if(orient==XtorientVertical)
       XawPanedSetMinMax((Widget)pmw,text_height+pmw->procmeter_generic.label_height,
                                     text_height+pmw->procmeter_generic.label_height);
    else
      {
       text_width=XTextWidth(pmw->procmeter_text.text_font,"NNNNNNNNNNNNNNN",15);
       XawPanedSetMinMax((Widget)pmw,text_width,text_width);
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Update the display.

  ProcMeterTextWidget pmw The Widget to update.

  Boolean all Indicates if the whole widget is to be updated.
  ++++++++++++++++++++++++++++++++++++++*/

static void TextUpdate(ProcMeterTextWidget pmw,Boolean all)
{
 if(pmw->core.visible)
   {
    if(all)
       (*procMeterGenericClassRec.procmeter_generic_class.update)((ProcMeterGenericWidget)pmw);
    else
       XClearArea(XtDisplay(pmw),XtWindow(pmw),
                  0              ,pmw->procmeter_generic.body_start,
                  pmw->core.width,pmw->procmeter_generic.body_height,False);

    XDrawString(XtDisplay(pmw),XtWindow(pmw),pmw->procmeter_generic.body_gc,
                pmw->procmeter_text.text_x     ,pmw->procmeter_text.text_y,
                pmw->procmeter_text.text_string,(int)strlen(pmw->procmeter_text.text_string));
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Change the data displayed in the ProcMeter Text Widget.

  Widget w The ProcMeter Text Widget.

  char *data The new string to display.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterTextChangeData(Widget w,char *data)
{
 ProcMeterTextWidget pmw=(ProcMeterTextWidget)w;

 XtFree((XtPointer)pmw->procmeter_text.text_string);
 pmw->procmeter_text.text_string=XtNewString(data);

 pmw->procmeter_text.text_x=(pmw->core.width-XTextWidth(pmw->procmeter_text.text_font,pmw->procmeter_text.text_string,(int)strlen(pmw->procmeter_text.text_string)))/2;

 TextUpdate(pmw,False);
}
