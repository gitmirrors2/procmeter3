/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/window.c,v 1.11 2000-12-16 16:51:15 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3.

  X Windows interface.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1997,98,99,2000 Andrew M. Bishop
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
static void ResizePane(void);
static void CloseCallback(Widget w,XtPointer va,XEvent* e,Boolean* vb);


/*+ The application context. +*/
XtAppContext app_context;

/*+ The display that the meter is on. +*/
Display* display=NULL;

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
 struct timeval now;
 int delay;

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
                ProcMeterGraphAddDatum((*output)->output_widget,value);
               }
             else if((*output)->type==PROCMETER_TEXT)
                ProcMeterTextChangeData((*output)->output_widget,(*output)->output->text_value);
             else if((*output)->type==PROCMETER_BAR)
               {
                long value=(*output)->output->graph_value;
                if(value<0)
                   value=0;
                if(value>65535)
                   value=65535;
                ProcMeterBarAddDatum((*output)->output_widget,value);
               }
            }

          last=(*output)->output;
         }
}


/*++++++++++++++++++++++++++++++++++++++
  Add the default outputs at startup.

  int argc The number of command line arguments.

  char **argv The command line arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void AddDefaultOutputs(int argc,char **argv)
{
 Output *outputp=NULL;
 Module *modulep=NULL;
 char *string;
 int arg;

 if((string=GetProcMeterRC("startup","order")))
   {
    char *s=string;

    while(*s && *s==' ')
       s++;

    while(*s)
      {
       int found=0;

       for(modulep=Modules;*modulep;modulep++)
         {
          if(!strncmp((*modulep)->module->name,s,strlen((*modulep)->module->name)) &&
             s[strlen((*modulep)->module->name)]=='.')
            {
             for(outputp=(*modulep)->outputs;*outputp;outputp++)
                if(!strncmp((*outputp)->output->name,&s[strlen((*modulep)->module->name)+1],strlen((*outputp)->output->name)) &&
                   (s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]=='-' ||
                    s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]==' ' ||
                    s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]==0))
                  {
                   if((s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]==' ' ||
                       s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]==0) &&
                      !(*outputp)->output_widget)
                      AddRemoveOutput(*outputp);
                   else if(s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='g' &&
                           (*outputp)->type==PROCMETER_GRAPH &&
                           !(*outputp)->output_widget)
                      AddRemoveOutput(*outputp);
                   else if(s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='t' &&
                           (*outputp)->type==PROCMETER_TEXT &&
                           !(*outputp)->output_widget)
                      AddRemoveOutput(*outputp);
                   else if(s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='b' &&
                           (*outputp)->type==PROCMETER_BAR &&
                           !(*outputp)->output_widget)
                      AddRemoveOutput(*outputp);
                   found=1;
                  }

             if(found)
                break;
            }
         }

       while(*s && *s!=' ')
          s++;
       while(*s && *s==' ')
          s++;
      }
   }

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
                if(!argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1] &&
                   !(*outputp)->output_widget)
                   AddRemoveOutput(*outputp);
                else if(argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='g' &&
                        (*outputp)->type==PROCMETER_GRAPH &&
                        !(*outputp)->output_widget)
                   AddRemoveOutput(*outputp);
                else if(argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='t' &&
                        (*outputp)->type==PROCMETER_TEXT &&
                        !(*outputp)->output_widget)
                   AddRemoveOutput(*outputp);
                else if(argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='b' &&
                        (*outputp)->type==PROCMETER_BAR &&
                        !(*outputp)->output_widget)
                   AddRemoveOutput(*outputp);
                found=1;
               }

          if(found)
             break;
         }

    if(!*modulep)
       fprintf(stderr,"ProcMeter: Unrecognised output '%s'\n",argv[arg]);
   }

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
    Widget w=NULL;
    char *string,str[16];
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

 ResizePane();
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

 ResizePane();

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
