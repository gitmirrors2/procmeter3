/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/window.c,v 1.15 2008-05-05 12:48:23 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.5b.

  X Windows interface.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1997-2008 Andrew M. Bishop
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
#include "widgets/PMBar.h"

#include "procmeter.h"
#include "procmeterp.h"
#include "window.h"


#define MINHEIGHT 30
#define MINWIDTH  60

static void SleepCallback(XtPointer p,XtIntervalId i);
static void ResizePaneCallback(Widget w,XtPointer va,XEvent* e,Boolean* vb);
static void CloseCallback(Widget w,XtPointer va,XEvent* e,Boolean* vb);
static void add_EWMH(char *name);


/*+ The application context. +*/
XtAppContext app_context;

/*+ The display that the meter is on. +*/
Display* display=NULL;

/*+ The toplevel widget. +*/
Widget toplevel;

/*+ The pane that contains all of the outputs. +*/
Widget pane;

/*+ If the meters are aligned vertically. +*/
int vertical=1;

/*+ A flag that is set to true when we are told to quit. +*/
extern int quit;

/*+ Set to true when we are sleeping waiting for a timeout. +*/
static int sleeping;

/*+ A list of the outputs that are currently visible. +*/
static Output *displayed=NULL;
static int ndisplayed=0;

/*+ A flag to indicate that we are still initialising and not to resize. +*/
static int initialising=1;


/*++++++++++++++++++++++++++++++++++++++
  Start the X-Windows & Athena part.

  int *argc The number of command line arguments.

  char **argv The actual command line arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void Start(int *argc,char **argv)
{
 Atom close_atom;
 static char procmeter_version[]="ProcMeter V" PROCMETER_VERSION;
 char *string;
 int i,j=0;

 if((string=GetProcMeterRC("resources","horizontal")) &&
    StringToBoolean(string))
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

 if((string=GetProcMeterRC("resources","background")))
    XtVaSetValues(pane,XtNbackground,StringToPixel(string),NULL);

 XtAddEventHandler(pane,StructureNotifyMask,False,(XtEventHandler)ResizePaneCallback,NULL);

 AddMenuToOutput(pane,NULL);

 /* Show the widgets */

 XtRealizeWidget(toplevel);
 XFlush(display);

 /* Parse the -w flag */

 for(i=1;i<*argc;i++)
    if((!strcmp(argv[i],"-w")) && (i+1<=*argc))
      {
       char *token;

       i++;j+=2;

       token=strtok(argv[i],",");
       while(token)
         {
          if(!strcmp(token,"above"))
             add_EWMH("_NET_WM_STATE_ABOVE");
          else if(!strcmp(token,"below"))
             add_EWMH("_NET_WM_STATE_BELOW");
          else if(!strcmp(token,"skip_taskbar"))
             add_EWMH("_NET_WM_STATE_SKIP_TASKBAR");
          else if(!strcmp(token,"skip_pager"))
             add_EWMH("_NET_WM_STATE_SKIP_PAGER");
          else if(!strcmp(token,"sticky"))
             add_EWMH("_NET_WM_STATE_STICKY");
          else
             fprintf(stderr,"ProcMeter3: Cannot parse -w option: '%s'\n",token);

          token = strtok(NULL,",");
         }
      }

 if(j>0)
   {
    for(i=j;i<*argc;i++)
      argv[i-j]=argv[i];
    *argc-=j;
   }

 /* Put an action on the close button */

 close_atom=XInternAtom(display,"WM_DELETE_WINDOW",False);

 XSetWMProtocols(display,XtWindow(toplevel),&close_atom,1);

 XtAddEventHandler(toplevel,0,True,CloseCallback,NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Stop the X-Windows & Athena part.
  ++++++++++++++++++++++++++++++++++++++*/

void Stop(void)
{
 DestroyMenus();

 XCloseDisplay(display);
}


/*++++++++++++++++++++++++++++++++++++++
  Sleep for the specified interval in seconds.

  time_t until The time to sleep until.
  ++++++++++++++++++++++++++++++++++++++*/

void Sleep(time_t until)
{
 struct timeval now;
 int delay;

 /* Before we sleep the first time, resize the window. */

 if(initialising)
   {
    Resize();
    initialising=0;
   }

 /* Sleep */

 gettimeofday(&now,NULL);

 delay=1000*(until-now.tv_sec)-now.tv_usec/1000;

 if(delay>0)
   {
    XtIntervalId id=XtAppAddTimeOut(app_context,(unsigned)delay,(XtTimerCallbackProc)SleepCallback,NULL);
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
    Widget w=NULL;
    char *string,str[PROCMETER_NAME_LEN+1];
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
        (string=GetProcMeterRC("resources","foreground"))))
      {XtSetArg(args[nargs],XtNforeground,StringToPixel(string));nargs++;}

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"background")) ||
        (string=GetProcMeterRC(module->module->name,"background")) ||
        (string=GetProcMeterRC("resources","background"))))
      {XtSetArg(args[nargs],XtNbackground,StringToPixel(string));nargs++;}

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"label-font")) ||
        (string=GetProcMeterRC(module->module->name,"label-font")) ||
        (string=GetProcMeterRC("resources","label-font"))))
      {XtSetArg(args[nargs],XtNlabelFont,StringToFont(string));nargs++;}

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"label-foreground")) ||
        (string=GetProcMeterRC(module->module->name,"label-foreground")) ||
        (string=GetProcMeterRC("resources","label-foreground")) ||
        (string=GetProcMeterRC2(module->module->name,output->output->name,"foreground")) ||
        (string=GetProcMeterRC(module->module->name,"foreground")) ||
        (string=GetProcMeterRC("resources","foreground"))))
      {XtSetArg(args[nargs],XtNlabelForeground,StringToPixel(string));nargs++;}

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"label-position")) ||
        (string=GetProcMeterRC(module->module->name,"label-position")) ||
        (string=GetProcMeterRC("resources","label-position"))))
      {XtSetArg(args[nargs],XtNlabelPosition,StringToLabelPosition(string));nargs++;}

    if(output->type==PROCMETER_GRAPH)
      {
       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-foreground")) ||
           (string=GetProcMeterRC(module->module->name,"grid-foreground")) ||
           (string=GetProcMeterRC("resources","grid-foreground"))))
         {XtSetArg(args[nargs],XtNgridForeground,StringToPixel(string));nargs++;}

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"graph-solid")) ||
           (string=GetProcMeterRC(module->module->name,"graph-solid")) ||
           (string=GetProcMeterRC("resources","graph-solid"))))
         {XtSetArg(args[nargs],XtNsolid,StringToBoolean(string));nargs++;}

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-min")) ||
           (string=GetProcMeterRC(module->module->name,"grid-min")) ||
           (string=GetProcMeterRC("resources","grid-min"))))
         {XtSetArg(args[nargs],XtNgridMin,StringToInt(string));nargs++;}

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-max")) ||
           (string=GetProcMeterRC(module->module->name,"grid-max")) ||
           (string=GetProcMeterRC("resources","grid-max"))))
         {XtSetArg(args[nargs],XtNgridMax,StringToInt(string));nargs++;}

       if(vertical)
         {XtSetArg(args[nargs],XtNmin,MINHEIGHT);nargs++;}
       else
         {XtSetArg(args[nargs],XtNmin,MINWIDTH);nargs++;}

       XtSetArg(args[nargs],XtNlabel,output->label);nargs++;
       sprintf(str,output->output->graph_units,output->output->graph_scale);
       XtSetArg(args[nargs],XtNgridUnits,str);nargs++;
       XtSetArg(args[nargs],XtNallowResize,True);nargs++;
       XtSetArg(args[nargs],XtNshowGrip,False);nargs++;

       w=XtCreateManagedWidget(output->output->name,procMeterGraphWidgetClass,pane,
                               args,nargs);
      }
    else if(output->type==PROCMETER_TEXT)
      {
       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"text-font")) ||
           (string=GetProcMeterRC(module->module->name,"text-font")) ||
           (string=GetProcMeterRC("resources","text-font"))))
         {XtSetArg(args[nargs],XtNtextFont,StringToFont(string));nargs++;}

       XtSetArg(args[nargs],XtNlabel,output->label);nargs++;
       XtSetArg(args[nargs],XtNallowResize,True);nargs++;
       XtSetArg(args[nargs],XtNskipAdjust,True);nargs++;
       XtSetArg(args[nargs],XtNshowGrip,False);nargs++;

       w=XtCreateManagedWidget(output->output->name,procMeterTextWidgetClass,pane,
                               args,nargs);
      }
    else if(output->type==PROCMETER_BAR)
      {
       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-foreground")) ||
           (string=GetProcMeterRC(module->module->name,"grid-foreground")) ||
           (string=GetProcMeterRC("resources","grid-foreground"))))
         {XtSetArg(args[nargs],XtNgridForeground,StringToPixel(string));nargs++;}

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-min")) ||
           (string=GetProcMeterRC(module->module->name,"grid-min")) ||
           (string=GetProcMeterRC("resources","grid-min"))))
         {XtSetArg(args[nargs],XtNgridMin,StringToInt(string));nargs++;}

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-max")) ||
           (string=GetProcMeterRC(module->module->name,"grid-max")) ||
           (string=GetProcMeterRC("resources","grid-max"))))
         {XtSetArg(args[nargs],XtNgridMax,StringToInt(string));nargs++;}

       XtSetArg(args[nargs],XtNmin,MINHEIGHT);nargs++;

       XtSetArg(args[nargs],XtNlabel,output->label);nargs++;
       sprintf(str,output->output->graph_units,output->output->graph_scale);
       XtSetArg(args[nargs],XtNgridUnits,str);nargs++;
       XtSetArg(args[nargs],XtNallowResize,True);nargs++;
       XtSetArg(args[nargs],XtNshowGrip,False);nargs++;

       w=XtCreateManagedWidget(output->output->name,procMeterBarWidgetClass,pane,
                               args,nargs);
      }

    AddMenuToOutput(w,module);

    XtVaSetValues(output->menu_item_widget,XtNleftBitmap,CircleBitmap,NULL);

    output->output_widget=w;
    output->first=2;

    displayed=(Output*)realloc((void*)displayed,sizeof(Output)*(ndisplayed+1));
    displayed[ndisplayed]=output;
    ndisplayed++;
   }

 XawPanedSetRefigureMode(pane,True);

 Resize();
}


/*++++++++++++++++++++++++++++++++++++++
  Update a graph output.

  Output output The output to update.

  short value The new value.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateGraph(Output output,short value)
{
 ProcMeterGraphAddDatum(output->output_widget,value);
}


/*++++++++++++++++++++++++++++++++++++++
  Update a text output.

  Output output The output to update.

  char *value The new value.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateText(Output output,char *value)
{
 ProcMeterTextChangeData(output->output_widget,value);
}


/*++++++++++++++++++++++++++++++++++++++
  Update a bar output.

  Output output The output to update.

  short value The new value.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateBar(Output output,short value)
{
 ProcMeterBarAddDatum(output->output_widget,value);
}


/*++++++++++++++++++++++++++++++++++++++
  Move an output.

  Output output1 The output to be moved.

  Output output2 The one that the output is to be moved above or below.

  int direction The direction to move the output (up=1 or down=2).
  ++++++++++++++++++++++++++++++++++++++*/

void MoveOutput(Output output1,Output output2,int direction)
{
 int i,i1=-1,i2=-1;

 for(i=0;i<ndisplayed;i++)
   {
    if(displayed[i]==output1)
       i1=i;
    if(displayed[i]==output2)
       i2=i;
   }

 if(i1==-1 || i2==-1 || i1==i2 || (direction==2 && i1==(i2+1)) || (direction==1 && i1==(i2-1)))
    return;

 XawPanedSetRefigureMode(pane,False);

 XtUnmanageChild(output1->output_widget);

 for(i=i2;i<ndisplayed;i++)
   {
    if(i==i2)
      {
       XtManageChild(output1->output_widget);
       if(direction==2)
          continue;
      }
    if(i!=i1)
      {
       XtUnmanageChild(displayed[i]->output_widget);
       XtManageChild(displayed[i]->output_widget);
      }
   }

 XawPanedSetRefigureMode(pane,True);

 Resize();

 if(direction==1 && i2>i1)
   {
    for(i=i1;i<i2;i++)
       displayed[i]=displayed[i+1];
    displayed[i2-1]=output1;
   }
 else if(direction==1 && i1>i2)
   {
    for(i=i1;i>i2;i--)
       displayed[i]=displayed[i-1];
    displayed[i2]=output1;
   }
 else if(direction==2 && i2>i1)
   {
    for(i=i1;i<i2;i++)
       displayed[i]=displayed[i+1];
    displayed[i2]=output1;
   }
 else /* if(direction==2 && i1>i2) */
   {
    for(i=i1;i>i2;i--)
       displayed[i]=displayed[i-1];
    displayed[i2+1]=output1;
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Resize the pane.
  ++++++++++++++++++++++++++++++++++++++*/

void Resize(void)
{
 Dimension psize,size;
 int gsize,msize;
 int i,ngraphs=0;

 if(initialising)
    return;

 if(!ndisplayed)
    return;

 XtVaGetValues(pane,vertical?XtNheight:XtNwidth,&psize,NULL);

 msize=0;
 gsize=psize;

 for(i=0;i<ndisplayed;i++)
   {
    int min_size,max_size;

    XawPanedGetMinMax(displayed[i]->output_widget,&min_size,&max_size);

    if(displayed[i]->type==PROCMETER_GRAPH)
       ngraphs++;
    else if(displayed[i]->type==PROCMETER_TEXT)
       gsize-=min_size;
    else if(displayed[i]->type==PROCMETER_BAR)
       ngraphs++;

    msize+=min_size;

    if(i)
       msize+=2,gsize-=2;       /* separator between panes */
   }

 if(msize>psize || (ngraphs==0 && msize!=psize))
   {
    XtVaSetValues(XtParent(pane),vertical?XtNheight:XtNwidth,msize,NULL);
    return;
   }

 for(i=0;i<ndisplayed;i++)
   {
    if(displayed[i]->type==PROCMETER_GRAPH || displayed[i]->type==PROCMETER_BAR)
      {
       size=gsize/ngraphs;
       gsize-=size;
       ngraphs--;
      }
    else
      {
       continue;
      }

    XtVaSetValues(displayed[i]->output_widget,vertical?XtNheight:XtNwidth,size,NULL);
   }
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
 Resize();
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


/*++++++++++++++++++++++++++++++++++++++
  Adds an Extended Window Manager Hint to the window.

  char *name The name of the hint to add.
  ++++++++++++++++++++++++++++++++++++++*/

static void add_EWMH(char *name)
{
 XEvent event;

 event.xclient.type=ClientMessage;
 event.xclient.serial=0;
 event.xclient.send_event=True;
 event.xclient.message_type=XInternAtom(display,"_NET_WM_STATE",False);
 event.xclient.window=XtWindow(toplevel);
 event.xclient.format=32;
 event.xclient.data.l[0]=1; /* add */
 event.xclient.data.l[1]=XInternAtom(display,name,False);
 event.xclient.data.l[2]=0;
 event.xclient.data.l[3]=0;
 event.xclient.data.l[4]=0;

 XSendEvent(display,DefaultRootWindow(display),False,SubstructureRedirectMask|SubstructureNotifyMask,&event);
}
