/***************************************
  $Header: /home/amb/CVS/procmeter3/gtk1/window.c,v 1.1 2000-12-16 16:36:35 amb Exp $

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
#include <string.h>

#include <sys/time.h>

#include <gtk/gtk.h>

#include "widgets/PMGeneric.h"
#include "widgets/PMGraph.h"
#include "widgets/PMText.h"
#include "widgets/PMBar.h"

#include "procmeter.h"
#include "procmeterp.h"
#include "window.h"


#define MINHEIGHT 30
#define MINWIDTH  60

static gint SleepCallback(gpointer p);
static void ResizePaneCallback(GtkWidget *w,GdkEventConfigure *event);
static void ResizePane(void);
static gint CloseCallback(GtkWidget *w,GdkEvent *event,gpointer data);


/*+ The toplevel widget. +*/
GtkWidget *toplevel;

/*+ The pane that contains all of the outputs. +*/
GtkWidget *pane;

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
 static char procmeter_version[16]="ProcMeter V" PROCMETER_VERSION;
 char *string;
 int i;

 if((string=GetProcMeterRC("resources","horizontal")) &&
    StringToBoolean(string))
    vertical=0;

 /* Initialise the display */

 gtk_init(argc,&argv);

 toplevel=gtk_window_new(GTK_WINDOW_TOPLEVEL);
 gtk_window_set_title(GTK_WINDOW(toplevel),procmeter_version);
 gtk_window_set_policy(GTK_WINDOW(toplevel),TRUE,TRUE,TRUE);

 /* Create the bitmaps */

 CreateBitmaps(toplevel);

 /* Create the menu widgets */

 CreateMenus(toplevel);

 /* Create the pane widget */

 if(vertical)
    pane=gtk_vbox_new(FALSE,0);
 else
    pane=gtk_hbox_new(FALSE,0);

 gtk_box_set_spacing(GTK_BOX(pane),2);

 gtk_container_add(GTK_CONTAINER(toplevel),pane);
 gtk_widget_show(GTK_WIDGET(pane));

 gtk_signal_connect(GTK_OBJECT(toplevel),"configure_event",
                    GTK_SIGNAL_FUNC(ResizePaneCallback),NULL);

 AddMenuToOutput(pane,NULL);

 /* Parse the -geometry flag */

 for(i=1;i<*argc-1;i++)
    if(!strcmp(argv[i],"-geometry"))
      {
       int x,y,w,h;

       if(sscanf(argv[i+1],"%dx%d%d%d",&w,&h,&x,&y)==4)
         {
          gtk_widget_set_usize(GTK_WIDGET(toplevel),w,h);

          if(x<0) x=gdk_screen_width()-w+x;
          if(y<0) y=gdk_screen_height()-h+y;

          gtk_widget_set_uposition(GTK_WIDGET(toplevel),x,y);

          break;
         }
       else if(sscanf(argv[i+1],"%dx%d",&w,&h)==2)
         {
          gtk_widget_set_usize(GTK_WIDGET(toplevel),w,h);

          break;
         }
      }

 if(i<*argc-1)
   {
    for(i+=2;i<*argc;i++)
       argv[i-2]=argv[i];
    *argc-=2;
   }

 /* Show the widgets */

 gtk_widget_show(GTK_WIDGET(toplevel));

 /* Put an action on the window manager close button */

 gtk_signal_connect(GTK_OBJECT(toplevel),"delete_event",
                    GTK_SIGNAL_FUNC(CloseCallback),NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Stop the X-Windows part.
  ++++++++++++++++++++++++++++++++++++++*/

void StopX(void)
{
 DestroyMenus();

 gtk_main_quit();
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
    gint id=gtk_timeout_add(delay,(GtkFunction)SleepCallback,NULL);
    sleeping=1;

    while(sleeping)
      {
       struct timeval now2;

       gtk_main_iteration();

       gettimeofday(&now2,NULL);

       if(now2.tv_sec<now.tv_sec)       /* Ooops, we went back in time. Let's cancel timer */
         {
          gtk_timeout_remove(id);
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
 if(output->output_widget)
   {
    int i,found=0;

    gtk_widget_destroy(GTK_WIDGET(output->output_widget));

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
    GtkWidget *w=NULL;
    char *string,str[16];
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

    if(output->type==PROCMETER_GRAPH)
      {
       w=gtk_procmetergraph_new();
       gtk_box_pack_start(GTK_BOX(pane),w,TRUE,TRUE,0);

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-foreground")) ||
           (string=GetProcMeterRC(module->module->name,"grid-foreground")) ||
           (string=GetProcMeterRC("resources","grid-foreground"))))
          ProcMeterGraphSetGridColour(GTK_PROCMETERGRAPH(w),StringToPixel(string));

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"graph-solid")) ||
           (string=GetProcMeterRC(module->module->name,"graph-solid")) ||
           (string=GetProcMeterRC("resources","graph-solid"))))
          ProcMeterGraphSetSolid(GTK_PROCMETERGRAPH(w),StringToBoolean(string));

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-min")) ||
           (string=GetProcMeterRC(module->module->name,"grid-min")) ||
           (string=GetProcMeterRC("resources","grid-min"))))
          ProcMeterGraphSetGridMin(GTK_PROCMETERGRAPH(w),StringToInt(string));
 
       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-max")) ||
           (string=GetProcMeterRC(module->module->name,"grid-max")) ||
           (string=GetProcMeterRC("resources","grid-max"))))
          ProcMeterGraphSetGridMax(GTK_PROCMETERGRAPH(w),StringToInt(string));

//       if(vertical)
//         {XtSetArg(args[nargs],XtNmin,MINHEIGHT);nargs++;}
//       else
//         {XtSetArg(args[nargs],XtNmin,MINWIDTH);nargs++;}

       sprintf(str,output->output->graph_units,output->output->graph_scale);
       ProcMeterGraphSetGridUnits(GTK_PROCMETERGRAPH(w),str);
      }
    else if(output->type==PROCMETER_TEXT)
      {
       w=gtk_procmetertext_new();
       gtk_box_pack_start(GTK_BOX(pane),w,FALSE,TRUE,0);

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"text-font")) ||
           (string=GetProcMeterRC(module->module->name,"text-font")) ||
           (string=GetProcMeterRC("resources","text-font"))))
          ProcMeterTextSetFont(GTK_PROCMETERTEXT(w),StringToFont(string));
      }
    else if(output->type==PROCMETER_BAR)
      {
       w=gtk_procmeterbar_new();
       gtk_box_pack_start(GTK_BOX(pane),w,TRUE,TRUE,0);

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-foreground")) ||
           (string=GetProcMeterRC(module->module->name,"grid-foreground")) ||
           (string=GetProcMeterRC("resources","grid-foreground"))))
          ProcMeterBarSetGridColour(GTK_PROCMETERBAR(w),StringToPixel(string));

       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-min")) ||
           (string=GetProcMeterRC(module->module->name,"grid-min")) ||
           (string=GetProcMeterRC("resources","grid-min"))))
          ProcMeterBarSetGridMin(GTK_PROCMETERBAR(w),StringToInt(string));
 
       if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-max")) ||
           (string=GetProcMeterRC(module->module->name,"grid-max")) ||
           (string=GetProcMeterRC("resources","grid-max"))))
          ProcMeterBarSetGridMax(GTK_PROCMETERBAR(w),StringToInt(string));

       sprintf(str,output->output->graph_units,output->output->graph_scale);
       ProcMeterBarSetGridUnits(GTK_PROCMETERBAR(w),str);
      }

    /* Generic */

    ProcMeterGenericSetLabel(GTK_PROCMETERGENERIC(w),(gchar*)output->label);

    /* Resources */

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"foreground")) ||
        (string=GetProcMeterRC(module->module->name,"foreground")) ||
        (string=GetProcMeterRC("resources","foreground"))))
       ProcMeterGenericSetForegroundColour(GTK_PROCMETERGENERIC(w),StringToPixel(string));

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"background")) ||
        (string=GetProcMeterRC(module->module->name,"background")) ||
        (string=GetProcMeterRC("resources","background"))))
       ProcMeterGenericSetBackgroundColour(GTK_PROCMETERGENERIC(w),StringToPixel(string));

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"label-font")) ||
        (string=GetProcMeterRC(module->module->name,"label-font")) ||
        (string=GetProcMeterRC("resources","label-font"))))
       ProcMeterGenericSetLabelFont(GTK_PROCMETERGENERIC(w),StringToFont(string));

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"label-foreground")) ||
        (string=GetProcMeterRC(module->module->name,"label-foreground")) ||
        (string=GetProcMeterRC("resources","label-foreground")) ||
        (string=GetProcMeterRC2(module->module->name,output->output->name,"foreground")) ||
        (string=GetProcMeterRC(module->module->name,"foreground")) ||
        (string=GetProcMeterRC("resources","foreground"))))
       ProcMeterGenericSetLabelColour(GTK_PROCMETERGENERIC(w),StringToPixel(string));

    if(((string=GetProcMeterRC2(module->module->name,output->output->name,"label-position")) ||
        (string=GetProcMeterRC(module->module->name,"label-position")) ||
        (string=GetProcMeterRC("resources","label-position"))))
       ProcMeterGenericSetLabelPosition(GTK_PROCMETERGENERIC(w),StringToLabelPosition(string));

    AddMenuToOutput(w,module);

    gtk_widget_show(GTK_WIDGET(w));

    gtk_signal_handler_block_by_data(GTK_OBJECT(output->menu_item_widget),output);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(output->menu_item_widget),TRUE);
    gtk_signal_handler_unblock_by_data(GTK_OBJECT(output->menu_item_widget),output);

    output->output_widget=w;
    output->first=2;

    displayed=(Output*)realloc((void*)displayed,sizeof(Output)*(ndisplayed+1));
    displayed[ndisplayed]=output;
    ndisplayed++;
   }

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

 gtk_box_reorder_child(GTK_BOX(pane),GTK_WIDGET(output1->output_widget),i2);

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

  gint SleepCallback Returns true to repeat the timer.

  gpointer p Not used.

  This function is only ever called from the gtk event loop.
  ++++++++++++++++++++++++++++++++++++++*/

static gint SleepCallback(gpointer p)
{
 sleeping=0;
 return(FALSE);
}


/*++++++++++++++++++++++++++++++++++++++
  A callback that is activated by a resize event on the parent pane.

  GtkWidget *w The widget that caused the callback.

  GdkEventConfigure *event Not used.

  This function is only ever called from the gtk event loop.
  ++++++++++++++++++++++++++++++++++++++*/

static void ResizePaneCallback(GtkWidget *w,GdkEventConfigure *event)
{
 ResizePane();
}


/*++++++++++++++++++++++++++++++++++++++
  Resize the pane.
  ++++++++++++++++++++++++++++++++++++++*/

static void ResizePane(void)
{
 gushort psize,size;
 gint width,height;
 int gsize,msize;
 int i,ngraphs=0;

 if(initialising)
    return;

 if(!ndisplayed)
    return;

 gdk_window_get_size(GTK_WIDGET(toplevel)->window,&width,&height);
 if(vertical)
    psize=height;
 else
    psize=width;

 msize=0;
 gsize=psize;

 for(i=0;i<ndisplayed;i++)
   {
    GtkRequisition request;
    int min_size=0;

    gtk_widget_size_request(GTK_WIDGET(displayed[i]->output_widget),&request);

    if(vertical)
       min_size=request.height;
    else
       min_size=request.width;

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
    if(vertical)
       height=msize;
    else
       width=msize;

    gtk_widget_set_usize(GTK_WIDGET(pane),-1,-1);
    gtk_widget_set_usize(GTK_WIDGET(pane),width,height);
    gtk_window_set_default_size(GTK_WINDOW(toplevel),width,height);

    gtk_container_resize_children(GTK_CONTAINER(pane));

    return;
   }

 for(i=0;i<ndisplayed;i++)
   {
    if(displayed[i]->type==PROCMETER_GRAPH || displayed[i]->type==PROCMETER_BAR)
      {
       size=gsize/ngraphs;
       gsize-=size;
       ngraphs--;

       if(vertical)
          height=size;
       else
          width=size;
      }
    else
      {
       GtkRequisition request;

       gtk_widget_size_request(GTK_WIDGET(displayed[i]->output_widget),&request);

       if(vertical)
          height=request.height;
       else
          width=request.width;
      }

    gtk_widget_set_usize(GTK_WIDGET(displayed[i]->output_widget),-1,-1);
    gtk_widget_set_usize(GTK_WIDGET(displayed[i]->output_widget),width,height);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  A callback that is activated by a close window event on the toplevel window.

  gint CloseCallback returns a value that indicates if the toplevel window is not to be destroyed.

  GtkWidget *w The widget that caused the event.

  GdkEvent *event The event information.

  gpointer data Not used.

  This function is only ever called from the GTK event handler.
  ++++++++++++++++++++++++++++++++++++++*/

static gint CloseCallback(GtkWidget *w,GdkEvent *event,gpointer data)
{
 quit=1;

 return(TRUE);
}
