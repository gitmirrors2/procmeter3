/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/window.c,v 1.3 1998-10-24 09:02:51 amb Exp $

  ProcMeter - A system monitoring program for Linux (v3.0a).

  X Windows interface.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1997,98 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/SmeBSB.h>

#include "widgets/PMGeneric.h"
#include "widgets/PMGraph.h"
#include "widgets/PMText.h"

#include "procmeter.h"
#include "procmeterp.h"
#include "xwindow.h"

#define MINHEIGHT 30
#define MINWIDTH  60

static void SleepCallback(XtPointer p,XtIntervalId i);
static void ResizePaneCallback(Widget w,XtPointer va,XEvent* e,Boolean* vb);
static void ResizePane(void);
static void CloseCallback(Widget w,XtPointer va,XEvent* e,Boolean* vb);


/*+ The application context. +*/
XtAppContext app_context;

/*+ The display that the meter is on. +*/
Display* display=NULL;

/*+ If the meters are aligned vertically. +*/
int vertical=1;

/*+ A flag that is set to true when we are told to quit. +*/
extern int quit;

/*+ Set to true when we are sleeping waiting for a timeout. +*/
static int sleeping;

/*+ The pane that contains all of the outputs. +*/
static Widget pane;

/*+ A list of the outputs that are currently visible. +*/
static Output *displayed=NULL;
static int ndisplayed=0;

/*+ A flag to indicate that we are still initialising and not to resize. +*/
static int initialising=1;


/*++++++++++++++++++++++++++++++++++++++
  Start the X-Windows part.

  int *argc The number of command line arguments.

  char **argv The actual command line arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void StartX(int *argc,char **argv)
{
 Atom close_atom;
 Widget toplevel;
 static char procmeter_version[16]="ProcMeter V" PROCMETER_VERSION;
 char *string;

 if((string=GetProcMeterRC("resources","horizontal")) &&
    *StringToBoolean(string))
    vertical=0;

 /* Initialise the display */

 toplevel=XtVaAppInitialize(&app_context,"ProcMeter3",
                            NULL,(Cardinal)0,argc,argv,NULL,
                            XtNtitle,procmeter_version,
                            XtNiconName,procmeter_version,
                            NULL);

 display=XtDisplay(toplevel);

 /* Create the bitmaps */

 CreateBitmaps(toplevel);

 /* Create the menu widgets */

 CreateMenus(toplevel);

 /* Create the pane widget */

 pane=XtVaCreateManagedWidget("pane",panedWidgetClass,toplevel,
                              XtNwidth, vertical?100:200,
                              XtNheight,vertical?200:100,
                              XtNinternalBorderWidth,2,
                              XtNorientation,vertical?XtorientVertical:XtorientHorizontal,
                              NULL);

 XtAddEventHandler(pane,StructureNotifyMask,False,(XtEventHandler)ResizePaneCallback,NULL);

 AddMenuToOutput(pane,NULL,NULL);

 /* Show the widgets */

 XtRealizeWidget(toplevel);
 XFlush(display);

 /* Put an action on the close button */

 close_atom=XInternAtom(display,"WM_DELETE_WINDOW",False);

 XSetWMProtocols(display,XtWindow(toplevel),&close_atom,1);

 XtAddEventHandler(toplevel,0,True,CloseCallback,NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Stop the X-Windows part.
  ++++++++++++++++++++++++++++++++++++++*/

void StopX(void)
{
 DestroyMenus();

 XCloseDisplay(display);
}


/*++++++++++++++++++++++++++++++++++++++
  Sleep for the specified interval in seconds.

  time_t until The time to sleep until.
  ++++++++++++++++++++++++++++++++++++++*/

void SleepX(time_t until)
{
 XtIntervalId id;
 struct timeval now;
 int delay;

 gettimeofday(&now,NULL);

 delay=1000*(until-now.tv_sec)-now.tv_usec/1000;

 if(delay>0)
   {
    id=XtAppAddTimeOut(app_context,(unsigned)delay,(XtTimerCallbackProc)SleepCallback,NULL);
    sleeping=1;

    while(sleeping)
      {
       struct timeval now2;

       XtAppProcessEvent(app_context,XtIMAll);

       gettimeofday(&now2,NULL);

       if(now2.tv_sec<now.tv_sec)       /* Ooops, we went back in time. Let's cancel timer */
         {
          XtRemoveTimeOut(id);
          sleeping=0;
         }
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Perform the updates.

  time_t now The current time.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateX(time_t now)
{
 Module *module;
 Output *output;
 ProcMeterOutput *last=NULL;

 for(module=Modules;*module;module++)
    for(output=(*module)->outputs;*output;output++)
       if((*output)->output_widget &&
          (((*output)->output->interval && !(now%(*output)->output->interval)) ||
          (*output)->first))
         {
          if(last!=(*output)->output)
             if((*module)->Update(now,(*output)->output)==-1)
                fprintf(stderr,"ProcMeter: Error updating %s.%s\n",(*module)->module->name,(*output)->output->name);

          if((*output)->first)
             (*output)->first--;

          if(!(*output)->first)
            {
             if((*output)->type==PROCMETER_GRAPH)
               {
                long value=(*output)->output->graph_value;
                if(value<0)
                   value=0;
                if(value>65535)
                   value=65535;
                ProcMeterGraphWidgetAddDatum((*output)->output_widget,value);
               }
             else if((*output)->type==PROCMETER_TEXT)
                ProcMeterTextWidgetChangeData((*output)->output_widget,(*output)->output->text_value);
            }

          last=(*output)->output;
         }
}


/*++++++++++++++++++++++++++++++++++++++
  Add the default outputs at startup.
  ++++++++++++++++++++++++++++++++++++++*/

void AddDefaultOutputs(int argc,char **argv)
{
 Output *outputp=NULL;
 Module *modulep=NULL;
 char *string;
 int arg;

 for(arg=1;arg<argc;arg++)
   {
    int found=0;

    for(modulep=Modules;*modulep;modulep++)
       if(!strncmp((*modulep)->module->name,argv[arg],strlen((*modulep)->module->name)) &&
          argv[arg][strlen((*modulep)->module->name)]=='.')
         {
          for(outputp=(*modulep)->outputs;*outputp;outputp++)
             if(!strncmp((*outputp)->output->name,&argv[arg][strlen((*modulep)->module->name)+1],strlen((*outputp)->output->name)) &&
                (argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]=='-' ||
                 argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]==0))
               {
                if(!argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1])
                   AddRemoveOutput(*outputp);
                else if(argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='g' &&
                        (*outputp)->type==PROCMETER_GRAPH)
                   AddRemoveOutput(*outputp);
                else if(argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='t' &&
                        (*outputp)->type==PROCMETER_TEXT)
                   AddRemoveOutput(*outputp);
                found=1;
               }

          if(found)
             break;
         }

    if(!*modulep)
       fprintf(stderr,"ProcMeter: Unrecognised output '%s'\n",argv[arg]);
   }

 for(modulep=Modules;*modulep;modulep++)
    for(outputp=(*modulep)->outputs;*outputp;outputp++)
       if(!(*outputp)->output_widget)
          if(((*outputp)->type==PROCMETER_GRAPH &&
              (string=GetProcMeterRC2((*modulep)->module->name,(*outputp)->output->name,"startup-graph")) &&
              *StringToBoolean(string)) ||
             ((*outputp)->type==PROCMETER_TEXT &&
              (string=GetProcMeterRC2((*modulep)->module->name,(*outputp)->output->name,"startup-text")) &&
              *StringToBoolean(string)))
             AddRemoveOutput(*outputp);

 initialising=0;

 ResizePane();
}

/*++++++++++++++++++++++++++++++++++++++
  Add or remove an output

  Output output The output to be added or removed.
  ++++++++++++++++++++++++++++++++++++++*/

void AddRemoveOutput(Output output)
{
 XawPanedSetRefigureMode(pane,False);

 if(output->output_widget)
   {
    int i,found=0;

    XtDestroyWidget(output->output_widget);

    XtVaSetValues(output->menu_item_widget,XtNleftBitmap,None,NULL);

    output->output_widget=NULL;

    for(i=0;i<ndisplayed;i++)
       if(displayed[i]==output)
          found=1;
       else if(found)
          displayed[i-1]=displayed[i];
    ndisplayed--;
   }
 else
   {
    Widget w;
    char *string;
    XtPointer resource;
    Arg args[16];
    int nargs=0;
    Output *outputp=NULL;
    Module *modulep,module=NULL;

    for(modulep=Modules;*modulep;modulep++)
      {
       for(outputp=(*modulep)->outputs;*outputp;outputp++)
          if(output==*outputp)
            {
             module=*modulep;
             break;
            }
       if(module)
          break;
      }

    /* Resources */

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"foreground")) ||
        (string=GetProcMeterRC(module->module->name,"foreground")) ||
        (string=GetProcMeterRC("resources","foreground"))) &&
       (resource=(XtPointer)StringToPixel(string)))
      {XtSetArg(args[nargs],XtNforeground,*(Pixel*)resource);nargs++;}

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"background")) ||
        (string=GetProcMeterRC(module->module->name,"background")) ||
        (string=GetProcMeterRC("resources","background"))) &&
       (resource=(XtPointer)StringToPixel(string)))
      {XtSetArg(args[nargs],XtNbackground,*(Pixel*)resource);nargs++;}

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"label-font")) ||
        (string=GetProcMeterRC(module->module->name,"label-font")) ||
        (string=GetProcMeterRC("resources","label-font"))) &&
       (resource=StringToFont(string)))
      {XtSetArg(args[nargs],XtNlabelFont,resource);nargs++;}

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"label-foreground")) ||
        (string=GetProcMeterRC(module->module->name,"label-foreground")) ||
        (string=GetProcMeterRC("resources","label-foreground")) ||
        (string=GetProcMeterRC2(module->module->name,output->output->name,"foreground")) ||
        (string=GetProcMeterRC(module->module->name,"foreground")) ||
        (string=GetProcMeterRC("resources","foreground"))) &&
       (resource=(XtPointer)StringToPixel(string)))
      {XtSetArg(args[nargs],XtNlabelForeground,*(Pixel*)resource);nargs++;}

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"label-position")) ||
        (string=GetProcMeterRC(module->module->name,"label-position")) ||
        (string=GetProcMeterRC("resources","label-position"))) &&
       (resource=(XtPointer)StringToLabelPosition(string)))
      {XtSetArg(args[nargs],XtNlabelPosition,*(int*)resource);nargs++;}

    if(output->type==PROCMETER_GRAPH)
      {
       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-foreground")) ||
           (string=GetProcMeterRC(module->module->name,"grid-foreground")) ||
           (string=GetProcMeterRC("resources","grid-foreground"))) &&
          (resource=(XtPointer)StringToPixel(string)))
         {XtSetArg(args[nargs],XtNgridForeground,*(Pixel*)resource);nargs++;}

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"graph-solid")) ||
           (string=GetProcMeterRC(module->module->name,"graph-solid")) ||
           (string=GetProcMeterRC("resources","graph-solid"))) &&
          (resource=(XtPointer)StringToBoolean(string)))
         {XtSetArg(args[nargs],XtNsolid,*(Boolean*)resource);nargs++;}

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-min")) ||
           (string=GetProcMeterRC(module->module->name,"grid-min")) ||
           (string=GetProcMeterRC("resources","grid-min"))) &&
          (resource=(XtPointer)StringToInt(string)))
         {XtSetArg(args[nargs],XtNgridMin,*(int*)resource);nargs++;}

       if(vertical)
         {XtSetArg(args[nargs],XtNmin,MINHEIGHT);nargs++;}
       else
         {XtSetArg(args[nargs],XtNmin,MINWIDTH);nargs++;}

       XtSetArg(args[nargs],XtNlabel,output->output->name);nargs++;
       XtSetArg(args[nargs],XtNgridUnits,output->output->graph_units);nargs++;
       XtSetArg(args[nargs],XtNallowResize,True);nargs++;
       XtSetArg(args[nargs],XtNshowGrip,False);nargs++;

       w=XtCreateManagedWidget(output->output->name,procMeterGraphWidgetClass,pane,
                               args,nargs);
      }
    else if(output->type==PROCMETER_TEXT)
      {
       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"text-font")) ||
           (string=GetProcMeterRC(module->module->name,"text-font")) ||
           (string=GetProcMeterRC("resources","text-font"))) &&
          (resource=StringToFont(string)))
         {XtSetArg(args[nargs],XtNtextFont,resource);nargs++;}

       XtSetArg(args[nargs],XtNlabel,output->output->name);nargs++;
       XtSetArg(args[nargs],XtNallowResize,True);nargs++;
       XtSetArg(args[nargs],XtNskipAdjust,True);nargs++;
       XtSetArg(args[nargs],XtNshowGrip,False);nargs++;

       w=XtCreateManagedWidget(output->output->name,procMeterTextWidgetClass,pane,
                               args,nargs);
      }
    else
       return;

    AddMenuToOutput(w,module,output);

    XtVaSetValues(output->menu_item_widget,XtNleftBitmap,CircleBitmap,NULL);

    output->output_widget=w;
    output->first=2;

    displayed=(Output*)realloc((void*)displayed,sizeof(Output)*(ndisplayed+1));
    displayed[ndisplayed]=output;
    ndisplayed++;
   }

 XawPanedSetRefigureMode(pane,True);

 ResizePane();
}


/*++++++++++++++++++++++++++++++++++++++
  The function called by the timeout to terminate the sleep.

  XtPointer p Not used.

  XtIntervalId i Not used.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void SleepCallback(XtPointer p,XtIntervalId i)
{
 sleeping=0;
}


/*++++++++++++++++++++++++++++++++++++++
  A callback that is activated by a resize event on the parent pane.

  Widget w The widget that caused the callback.

  XtPointer va Not used.

  XEvent* e The event that requires action.

  Boolean* vb Not used.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void ResizePaneCallback(Widget w,XtPointer va,XEvent* e,Boolean* vb)
{
 ResizePane();
}


/*++++++++++++++++++++++++++++++++++++++
  Resize the pane.
  ++++++++++++++++++++++++++++++++++++++*/

static void ResizePane(void)
{
 Dimension psize,size;
 int rsize,gsize;
 int i,ngraphs=0;

 if(initialising)
    return;

 if(!ndisplayed)
    return;

 XtVaGetValues(pane,vertical?XtNheight:XtNwidth,&psize,NULL);
 rsize=psize;
 gsize=psize;

 for(i=0;i<ndisplayed;i++)
   {
    int min_size,max_size;

    XawPanedGetMinMax(displayed[i]->output_widget,&min_size,&max_size);

    if(displayed[i]->type==PROCMETER_GRAPH)
       ngraphs++;
    else
       gsize-=min_size;

    rsize-=min_size;

    if(i)
       rsize-=2,gsize-=2;       /* separator between panes */
   }

 if(rsize<0)
   {
    XtVaSetValues(XtParent(pane),vertical?XtNheight:XtNwidth,psize-rsize,NULL);
    return;
   }

 if(ngraphs)
   {
    size=gsize/ngraphs;

    for(i=0;i<ndisplayed;i++)
       if(displayed[i]->type==PROCMETER_GRAPH)
          XtVaSetValues(displayed[i]->output_widget,vertical?XtNheight:XtNwidth,size,NULL);
   }
 else
    XtVaSetValues(XtParent(pane),vertical?XtNheight:XtNwidth,psize-rsize,NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  A callback that is activated by a close window event on the toplevel window.

  Widget w The widget that caused the callback.

  XtPointer va Not used.

  XEvent* e The event that requires action.

  Boolean* vb Not used.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void CloseCallback(Widget w,XtPointer va,XEvent* e,Boolean* vb)
{
 XClientMessageEvent *cev=(XClientMessageEvent*)e;
 Atom atom_type=XInternAtom(display,"WM_PROTOCOLS",False);

 if(atom_type==cev->message_type)
   {
    Atom atom_proto=XInternAtom(display,"WM_DELETE_WINDOW",False);

    if(cev->format==32 && atom_proto==(Atom)cev->data.l[0])
       quit=1;
   }
}
