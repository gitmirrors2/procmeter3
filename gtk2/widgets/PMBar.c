/***************************************
  $Header: /home/amb/CVS/procmeter3/gtk2/widgets/PMBar.c,v 1.2 2007-11-21 19:57:18 amb Exp $

  ProcMeter Bar Widget Source file (for ProcMeter3 3.5a).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,99,2000,01,02,03,07 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdlib.h>
#include <string.h>

#include "PMGeneric.h"
#include "PMBar.h"

#include "procmeter.h"

static void procmeterbar_class_init(ProcMeterBarClass *class);
static void procmeterbar_init(ProcMeterBar *pmw);
static void destroy(GtkObject *object);
static void realize(GtkWidget *widget);
static void size_request(GtkWidget *widget,GtkRequisition *requisition);
static void size_allocate(GtkWidget *widget,GtkAllocation *allocation);
static gint expose(GtkWidget *widget,GdkEventExpose *event);

static void BarResize(ProcMeterBar *pmw);
static void BarUpdate(ProcMeterBar *pmw,gboolean all);

static char *empty_string="";

static ProcMeterGenericClass *parent_class=NULL;


/*++++++++++++++++++++++++++++++++++++++
  Returns the type of a Widget.

  guint gtk_procmeterbar_get_type Returns a unique pointer to the Widget type.
  ++++++++++++++++++++++++++++++++++++++*/

guint gtk_procmeterbar_get_type(void)
{
 static guint pmw_type = 0;

 if(!pmw_type)
   {
    GtkTypeInfo pmw_info={"ProcMeterBar",
                          sizeof(ProcMeterBar),
                          sizeof(ProcMeterBarClass),
                          (GtkClassInitFunc) procmeterbar_class_init,
                          (GtkObjectInitFunc) procmeterbar_init,
                          (gpointer) NULL,
                          (gpointer) NULL,
                          (GtkClassInitFunc) NULL};

    pmw_type=gtk_type_unique(gtk_procmetergeneric_get_type(),&pmw_info);
   }

 return(pmw_type);
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise the Widget class

  ProcMeterBarClass *class The class of widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmeterbar_class_init(ProcMeterBarClass *class)
{
 GtkObjectClass *object_class;
 GtkWidgetClass *widget_class;

 g_return_if_fail(class!=NULL);

 object_class=(GtkObjectClass*)class;
 widget_class=(GtkWidgetClass*)class;

 class->resize=BarResize;
 class->update=BarUpdate;

 parent_class=gtk_type_class(gtk_procmetergeneric_get_type());

 object_class->destroy=destroy;

 widget_class->realize=realize;
 widget_class->expose_event=expose;
 widget_class->size_request=size_request;
 widget_class->size_allocate=size_allocate;
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise an instance of the Widget

  ProcMeterBar *pmw The Widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmeterbar_init(ProcMeterBar *pmw)
{
 int i;

 g_return_if_fail(pmw!=NULL);

 /* The grid parts. */

 pmw->grid_units=empty_string;

 pmw->grid_gc=NULL;

 pmw->grid_drawn=1;
 pmw->grid_min=1;

 pmw->grid_max=0;

 if(pmw->grid_max && pmw->grid_max<pmw->grid_min)
    pmw->grid_max=pmw->grid_min;

 pmw->grid_num=pmw->grid_min;

 /* The data parts. */

 for(i=0;i<sizeof(pmw->data)/sizeof(pmw->data[0]);i++)
    pmw->data[i]=0;

 pmw->data_index=0;

 pmw->data_sum=0;

 /* The rest of the sizing. */

 BarResize(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Create a new Widget.

  GtkWidget* gtk_procmeterbar_new Returns the new widgets.
  ++++++++++++++++++++++++++++++++++++++*/

GtkWidget* gtk_procmeterbar_new(void)
{
 ProcMeterBar *pmw;

 pmw=gtk_type_new(gtk_procmeterbar_get_type());

 return(GTK_WIDGET(pmw));
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a Widget

  GtkObject *object The widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void destroy(GtkObject *object)
{
 ProcMeterBar *pmw;

 g_return_if_fail(object!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERBAR(object));

 pmw=GTK_PROCMETERBAR(object);

 if(pmw->grid_gc)
   {
    gdk_gc_destroy(pmw->grid_gc);
    pmw->grid_gc=NULL;
   }
 if(pmw->grid_units!=empty_string)
   {
    free(pmw->grid_units);
    pmw->grid_units=empty_string;
   }

 if(GTK_OBJECT_CLASS(parent_class)->destroy)
    (*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}


/*++++++++++++++++++++++++++++++++++++++
  Realize a widget.

  GtkWidget *widget The widget to realize.
  ++++++++++++++++++++++++++++++++++++++*/

static void realize(GtkWidget *widget)
{
 ProcMeterBar *pmw;
 GdkWindowAttr attributes;
 gint attributes_mask;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERBAR(widget));

 GTK_WIDGET_SET_FLAGS(widget,GTK_REALIZED);
 pmw=GTK_PROCMETERBAR(widget);

 attributes.x=widget->allocation.x;
 attributes.y=widget->allocation.y;
 attributes.width=widget->allocation.width;
 attributes.height=widget->allocation.height;
 attributes.wclass=GDK_INPUT_OUTPUT;
 attributes.window_type=GDK_WINDOW_CHILD;
 attributes.event_mask=gtk_widget_get_events(widget)|GDK_EXPOSURE_MASK|GDK_BUTTON_PRESS_MASK;
 attributes.visual=gtk_widget_get_visual(widget);
 attributes.colormap=gtk_widget_get_colormap(widget);

 attributes_mask=GDK_WA_X|GDK_WA_Y|GDK_WA_VISUAL|GDK_WA_COLORMAP;
 widget->window=gdk_window_new(widget->parent->window,&attributes,attributes_mask);

 widget->style=gtk_style_attach(widget->style,widget->window);

 gdk_window_set_user_data(widget->window,widget);

 gtk_style_set_background(widget->style,widget->window,GTK_STATE_ACTIVE);

 if(pmw->generic.body_bg_set)
    gdk_window_set_background(widget->window,&pmw->generic.body_bg_color);

 BarUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Choose the size that the widget wants to be.

  GtkWidget *widget The widget to be resized.

  GtkRequisition *requisition Returns the request for the size.
  ++++++++++++++++++++++++++++++++++++++*/

static void size_request(GtkWidget *widget,GtkRequisition *requisition)
{
 ProcMeterBar *pmw;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERBAR(widget));
 g_return_if_fail(requisition!=NULL);

 pmw=GTK_PROCMETERBAR(widget);

 requisition->height=20+pmw->generic.label_height;
 requisition->width=50;
}


/*++++++++++++++++++++++++++++++++++++++
  Change to the size that has been specified by the container.

  GtkWidget *widget The widget that has been resized.

  GtkAllocation *allocation The size information.
  ++++++++++++++++++++++++++++++++++++++*/

static void size_allocate(GtkWidget *widget,GtkAllocation *allocation)
{
 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERBAR(widget));
 g_return_if_fail(allocation!=NULL);

 widget->allocation=*allocation;
 if(GTK_WIDGET_REALIZED(widget))
   {
    ProcMeterBar *pmw=GTK_PROCMETERBAR(widget);

    gdk_window_move_resize(widget->window,
                           allocation->x,allocation->y,
                           allocation->width,allocation->height);

    BarResize(pmw);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Bar Widget.

  gint expose Returns false

  GtkWidget *widget The Widget to redisplay.

  GdkEventExpose *event The event that caused the redisplay.
  ++++++++++++++++++++++++++++++++++++++*/

static gint expose(GtkWidget *widget,GdkEventExpose *event)
{
 ProcMeterBar *pmw;

 g_return_val_if_fail(widget!=NULL,FALSE);
 g_return_val_if_fail(GTK_IS_PROCMETERBAR(widget),FALSE);
 g_return_val_if_fail(event!=NULL,FALSE);

 if(event->count>0)
    return(FALSE);

 pmw=GTK_PROCMETERBAR(widget);

 BarUpdate(pmw,TRUE);

 return(FALSE);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform all of the sizing on the Widget when it is created/resized.

  ProcMeterBar *pmw The Widget to resize.
  ++++++++++++++++++++++++++++++++++++++*/

static void BarResize(ProcMeterBar *pmw)
{
 GdkFont *label_font;

 g_return_if_fail(pmw!=NULL);

 (parent_class->resize)(&pmw->generic);

 pmw->generic.label_x=2;

 /* The grid parts. */

 label_font=pmw->generic.label_font?pmw->generic.label_font:gtk_style_get_font(pmw->generic.widget.style);

 pmw->grid_units_x=pmw->generic.widget.allocation.width-gdk_string_width(label_font,pmw->grid_units);

 pmw->grid_maxvis=pmw->generic.widget.allocation.width/3;

 if(pmw->generic.label_pos==ProcMeterLabelTop)
    pmw->generic.body_start=pmw->generic.label_height;
 else
    pmw->generic.body_start=0;

 if(pmw->grid_num>pmw->grid_maxvis && pmw->grid_drawn)
    pmw->grid_drawn=-1;
 if(pmw->grid_num<=pmw->grid_maxvis && pmw->grid_drawn)
    pmw->grid_drawn=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Update the display.

  ProcMeterBar *pmw The Widget to update.

  gboolean all Indicates if it all is to be updated including the generic parts.
  ++++++++++++++++++++++++++++++++++++++*/

static void BarUpdate(ProcMeterBar *pmw,gboolean all)
{
 g_return_if_fail(pmw!=NULL);

 if(GTK_WIDGET_VISIBLE(&pmw->generic.widget))
   {
    GdkGC *grid_gc=pmw->grid_gc?pmw->grid_gc:pmw->generic.widget.style->fg_gc[GTK_STATE_NORMAL];
    GdkGC *body_gc=pmw->generic.body_gc?pmw->generic.body_gc:pmw->generic.widget.style->fg_gc[GTK_STATE_NORMAL];
    GdkFont *label_font=pmw->generic.label_font?pmw->generic.label_font:gtk_style_get_font(pmw->generic.widget.style);

    int i;
    int scale=PROCMETER_GRAPH_SCALE*pmw->grid_num;
    gshort pos;
    gshort top_average_bottom,bottom_average_top,average_size;

    if(all)
      {
       (parent_class->update)(&pmw->generic);

       if(pmw->generic.label_pos!=ProcMeterLabelNone)
          gdk_draw_string(pmw->generic.widget.window,label_font,pmw->generic.label_gc,
                          pmw->grid_units_x,pmw->generic.label_y,
                          pmw->grid_units);
      }
    else
       gdk_window_clear_area(pmw->generic.widget.window,
                             0,pmw->generic.body_start,
                             pmw->generic.widget.allocation.width,pmw->generic.body_height);


    pos=pmw->data_sum*pmw->generic.widget.allocation.width/(scale*2);

    top_average_bottom=pmw->generic.body_start+2*(pmw->generic.body_height>>3);
    bottom_average_top=pmw->generic.body_start+pmw->generic.body_height-2*(pmw->generic.body_height>>3);
    average_size=pmw->generic.body_height>>3;

    gdk_draw_rectangle(pmw->generic.widget.window,body_gc,1,
                       pos-average_size,top_average_bottom-average_size,
                       average_size    ,average_size);

    gdk_draw_rectangle(pmw->generic.widget.window,body_gc,1,
                       pos-average_size,bottom_average_top,
                       average_size    ,average_size);

    pos=pmw->data[pmw->data_index]*pmw->generic.widget.allocation.width/scale;

    gdk_draw_rectangle(pmw->generic.widget.window,body_gc,1,
                       0  ,top_average_bottom+1,
                       pos,bottom_average_top-top_average_bottom-2);

    if(pmw->grid_drawn==1)
       for(i=1;i<pmw->grid_num;i++)
         {
          pos=i*pmw->generic.widget.allocation.width/pmw->grid_num;
          gdk_draw_line(pmw->generic.widget.window,grid_gc,
                        pos,pmw->generic.body_start,
                        pos,pmw->generic.body_height+pmw->generic.body_start);
         }
    else
       if(pmw->grid_drawn==-1)
         {
          pos=pmw->grid_maxvis*pmw->generic.widget.allocation.width/pmw->grid_num;
          gdk_draw_line(pmw->generic.widget.window,grid_gc,
                        pos,pmw->generic.body_start,
                        pos,pmw->generic.body_height+pmw->generic.body_start);
         }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Set the grid colour of the Widget.

  ProcMeterBar *pmw The widget to set.

  GdkColor grid_color The grid.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterBarSetGridColour(ProcMeterBar *pmw,GdkColor grid_color)
{
 pmw->grid_color=grid_color;

 if(pmw->grid_gc)
    gdk_gc_set_foreground(pmw->grid_gc,&pmw->grid_color);
 else
   {
    GdkGCValues values;

    values.foreground=pmw->grid_color;
    pmw->grid_gc=gdk_gc_new_with_values(pmw->generic.widget.parent->window,&values,GDK_GC_FOREGROUND);
   }

 BarUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the minimum number of grid lines in the Widget.

  ProcMeterBar *pmw The Widget to set.

  gint grid_min The minimum number of lines.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterBarSetGridMin(ProcMeterBar *pmw,gint grid_min)
{
 if(grid_min<0)
   {
    pmw->grid_min=-grid_min;
    pmw->grid_drawn=0;
   }
 else
    pmw->grid_drawn=1;
 if(grid_min==0)
    pmw->grid_min=1;

 if(grid_min>pmw->grid_max && pmw->grid_max)
    pmw->grid_min=pmw->grid_max;

 if(pmw->grid_min>=pmw->grid_num)
    pmw->grid_num=pmw->grid_min;

 BarResize(pmw);

 BarUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the maximum number of grid lines in the Widget.

  ProcMeterBar *pmw The Widget to set.

  gint grid_max The maximum number of lines.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterBarSetGridMax(ProcMeterBar *pmw,gint grid_max)
{
 if(grid_max<0)
    pmw->grid_max=0;

 if(grid_max && grid_max<pmw->grid_min)
    pmw->grid_max=pmw->grid_min;

 BarResize(pmw);

 BarUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the grid units for the widget

  ProcMeterBar *pmw The widget to set.

  gchar *units The grid units.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterBarSetGridUnits(ProcMeterBar *pmw,gchar *units)
{
 if(pmw->grid_units!=empty_string)
    free(pmw->grid_units);
 pmw->grid_units=(char*)malloc(strlen(units)+1);
 strcpy(pmw->grid_units,units);

 BarResize(pmw);

 BarUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a data point to the ProcMeter Bar Widget.

  ProcMeterBar *pmw The ProcMeter Bar Widget.

  gushort datum The data point to add.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterBarAddDatum(ProcMeterBar *pmw,gushort datum)
{
 int new_grid_num;
 gushort old_datum;

 pmw->data_index++;
 if(pmw->data_index==8)
    pmw->data_index=0;

 old_datum=pmw->data[pmw->data_index];
 pmw->data[pmw->data_index]=datum;

 pmw->data_sum=(pmw->data_sum>>1)+datum-(old_datum>>8);

 if((pmw->data_sum/2)>datum)
    new_grid_num=((pmw->data_sum/2)+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;
 else
    new_grid_num=(datum+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;

 if(new_grid_num<pmw->grid_min)
    new_grid_num=pmw->grid_min;
 if(pmw->grid_max && new_grid_num>pmw->grid_max)
    new_grid_num=pmw->grid_max;

 if(new_grid_num!=pmw->grid_num)
   {
    pmw->grid_num=new_grid_num;

    if(pmw->grid_num>pmw->grid_maxvis && pmw->grid_drawn)
       pmw->grid_drawn=-1;
    if(pmw->grid_num<=pmw->grid_maxvis && pmw->grid_drawn)
       pmw->grid_drawn=1;
   }

 BarUpdate(pmw,TRUE);
}
