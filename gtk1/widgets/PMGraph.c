/***************************************
  $Header: /home/amb/CVS/procmeter3/gtk1/widgets/PMGraph.c,v 1.5 2008-04-27 15:21:30 amb Exp $

  ProcMeter Graph Widget Source file (for ProcMeter3 3.5b).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996-2008 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdlib.h>
#include <string.h>

#include "PMGeneric.h"
#include "PMGraph.h"

#include "procmeter.h"

static void procmetergraph_class_init(ProcMeterGraphClass *class);
static void procmetergraph_init(ProcMeterGraph *pmw);
static void destroy(GtkObject *object);
static void realize(GtkWidget *widget);
static void size_request(GtkWidget *widget,GtkRequisition *requisition);
static void size_allocate(GtkWidget *widget,GtkAllocation *allocation);
static gint expose(GtkWidget *widget,GdkEventExpose *event);

static void GraphResize(ProcMeterGraph *pmw);
static void GraphUpdate(ProcMeterGraph *pmw,gboolean all);

static char *empty_string="";

static ProcMeterGenericClass *parent_class=NULL;


/*++++++++++++++++++++++++++++++++++++++
  Returns the type of a Widget.

  guint gtk_procmetergraph_get_type Returns a unique pointer to the Widget type.
  ++++++++++++++++++++++++++++++++++++++*/

guint gtk_procmetergraph_get_type(void)
{
 static guint pmw_type = 0;

 if(!pmw_type)
   {
    GtkTypeInfo pmw_info={"ProcMeterGraph",
                          sizeof(ProcMeterGraph),
                          sizeof(ProcMeterGraphClass),
                          (GtkClassInitFunc) procmetergraph_class_init,
                          (GtkObjectInitFunc) procmetergraph_init,
                          (GtkArgSetFunc) NULL,
                          (GtkArgGetFunc) NULL};

    pmw_type=gtk_type_unique(gtk_procmetergeneric_get_type(),&pmw_info);
   }

 return(pmw_type);
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise the Widget class

  ProcMeterGraphClass *class The class of widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmetergraph_class_init(ProcMeterGraphClass *class)
{
 GtkObjectClass *object_class;
 GtkWidgetClass *widget_class;

 g_return_if_fail(class!=NULL);

 object_class=(GtkObjectClass*)class;
 widget_class=(GtkWidgetClass*)class;

 class->resize=GraphResize;
 class->update=GraphUpdate;

 parent_class=gtk_type_class(gtk_procmetergeneric_get_type());

 object_class->destroy=destroy;

 widget_class->realize=realize;
 widget_class->expose_event=expose;
 widget_class->size_request=size_request;
 widget_class->size_allocate=size_allocate;
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise an instance of the Widget

  ProcMeterGraph *pmw The Widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmetergraph_init(ProcMeterGraph *pmw)
{
 g_return_if_fail(pmw!=NULL);

 /* The grid parts. */

 pmw->grid_units=empty_string;

 pmw->grid_gc=NULL;

 pmw->grid_drawn=1;
 pmw->grid_min=1;

 pmw->grid_max=0;

 pmw->grid_num=pmw->grid_min;

 /* The data parts. */

 pmw->data_num=10;
 pmw->data=(gushort*)calloc(pmw->data_num,sizeof(gushort));
 pmw->data_max=0;
 pmw->data_index=0;

 /* The rest of the sizing. */

 GraphResize(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Create a new Widget.

  GtkWidget* gtk_procmetergraph_new Returns the new widgets.
  ++++++++++++++++++++++++++++++++++++++*/

GtkWidget* gtk_procmetergraph_new(void)
{
 ProcMeterGraph *pmw;

 pmw=gtk_type_new(gtk_procmetergraph_get_type());

 return(GTK_WIDGET(pmw));
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a Widget

  GtkObject *object The widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void destroy(GtkObject *object)
{
 ProcMeterGraph *pmw;

 g_return_if_fail(object!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGRAPH(object));

 pmw=GTK_PROCMETERGRAPH(object);

 if(pmw->grid_gc)
    gdk_gc_destroy(pmw->grid_gc);
 if(pmw->grid_units!=empty_string)
    free(pmw->grid_units);
 free(pmw->data);

 if(GTK_OBJECT_CLASS(parent_class)->destroy)
    (*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}


/*++++++++++++++++++++++++++++++++++++++
  Realize a widget.

  GtkWidget *widget The widget to realize.
  ++++++++++++++++++++++++++++++++++++++*/

static void realize(GtkWidget *widget)
{
 ProcMeterGraph *pmw;
 GdkWindowAttr attributes;
 gint attributes_mask;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGRAPH(widget));

 GTK_WIDGET_SET_FLAGS(widget,GTK_REALIZED);
 pmw=GTK_PROCMETERGRAPH(widget);

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

 GraphUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Choose the size that the widget wants to be.

  GtkWidget *widget The widget to be resized.

  GtkRequisition *requisition Returns the request for the size.
  ++++++++++++++++++++++++++++++++++++++*/

static void size_request(GtkWidget *widget,GtkRequisition *requisition)
{
 ProcMeterGraph *pmw;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGRAPH(widget));
 g_return_if_fail(requisition!=NULL);

 pmw=GTK_PROCMETERGRAPH(widget);

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
 g_return_if_fail(GTK_IS_PROCMETERGRAPH(widget));
 g_return_if_fail(allocation!=NULL);

 widget->allocation=*allocation;
 if(GTK_WIDGET_REALIZED(widget))
   {
    ProcMeterGraph *pmw=GTK_PROCMETERGRAPH(widget);

    gdk_window_move_resize(widget->window,
                           allocation->x,allocation->y,
                           allocation->width,allocation->height);

    GraphResize(pmw);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Graph Widget.

  gint expose Returns false

  GtkWidget *widget The Widget to redisplay.

  GdkEventExpose *event The event that caused the redisplay.
  ++++++++++++++++++++++++++++++++++++++*/

static gint expose(GtkWidget *widget,GdkEventExpose *event)
{
 ProcMeterGraph *pmw;

 g_return_val_if_fail(widget!=NULL,FALSE);
 g_return_val_if_fail(GTK_IS_PROCMETERGRAPH(widget),FALSE);
 g_return_val_if_fail(event!=NULL,FALSE);

 if(event->count>0)
    return(FALSE);

 pmw=GTK_PROCMETERGRAPH(widget);

 GraphUpdate(pmw,TRUE);

 return(FALSE);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform all of the sizing on the Widget when it is created/resized.

  ProcMeterGraph *pmw The Widget to resize.
  ++++++++++++++++++++++++++++++++++++++*/

static void GraphResize(ProcMeterGraph *pmw)
{
 GdkFont *label_font;

 g_return_if_fail(pmw!=NULL);

 (parent_class->resize)(&pmw->generic);

 if(pmw->data_num!=pmw->generic.widget.allocation.width)
   {
    int i,old_num=pmw->data_num;
    gushort* old_data=pmw->data;

    pmw->data_num=pmw->generic.widget.allocation.width;
    pmw->data=(gushort*)calloc(pmw->data_num,sizeof(gushort));

    if(pmw->data_num<old_num)
       i=pmw->data_num;
    else
       i=old_num;

    for(;i>0;i--)
       pmw->data[(-i+pmw->data_num)%pmw->data_num]=old_data[(pmw->data_index-i+old_num)%old_num];

    pmw->data_index=0;

    free(old_data);

    for(i=pmw->data_max=0;i<pmw->data_num;i++)
       if(pmw->data[i]>pmw->data_max)
          pmw->data_max=pmw->data[i];

    pmw->grid_num=(pmw->data_max+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;

    if(pmw->grid_num<pmw->grid_min)
       pmw->grid_num=pmw->grid_min;
    if(pmw->grid_max && pmw->grid_num>pmw->grid_max)
       pmw->grid_num=pmw->grid_max;
   }

 pmw->generic.label_x=2;

 /* The grid parts. */

 label_font=pmw->generic.label_font?pmw->generic.label_font:pmw->generic.widget.style->font;

 pmw->grid_units_x=pmw->generic.widget.allocation.width-gdk_string_width(label_font,pmw->grid_units);

 pmw->grid_maxvis=pmw->generic.body_height/3;

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

  ProcMeterGraph *pmw The Widget to update.

  gboolean all Indicates if the whole widget is to be updated.
  ++++++++++++++++++++++++++++++++++++++*/

static void GraphUpdate(ProcMeterGraph *pmw,gboolean all)
{
 g_return_if_fail(pmw!=NULL);

 if(GTK_WIDGET_VISIBLE(&pmw->generic.widget))
   {
    GdkGC *grid_gc=pmw->grid_gc?pmw->grid_gc:pmw->generic.widget.style->fg_gc[GTK_STATE_NORMAL];
    GdkGC *body_gc=pmw->generic.body_gc?pmw->generic.body_gc:pmw->generic.widget.style->fg_gc[GTK_STATE_NORMAL];
    GdkFont *label_font=pmw->generic.label_font?pmw->generic.label_font:pmw->generic.widget.style->font;

    int i;
    int scale=PROCMETER_GRAPH_SCALE*pmw->grid_num;
    gushort val;
    gshort pos;

    if(all)
      {
       (parent_class->update)(&pmw->generic);

       if(pmw->generic.label_pos!=ProcMeterLabelNone)
          gdk_draw_string(pmw->generic.widget.window,label_font,pmw->generic.label_gc,
                          pmw->grid_units_x,pmw->generic.label_y,
                          pmw->grid_units);

       for(i=0;i<pmw->data_num;i++)
         {
          val=pmw->data[(i+pmw->data_index)%pmw->data_num];
          pos=val*pmw->generic.body_height/scale;

          if(pmw->line_solid)
             gdk_draw_line(pmw->generic.widget.window,body_gc,
                           i,pmw->generic.body_height+pmw->generic.body_start,
                           i,pmw->generic.body_height+pmw->generic.body_start-pos);
          else if(i)
            {
             gushort oldval=pmw->data[(i-1+pmw->data_index)%pmw->data_num];
             gshort oldpos=oldval*pmw->generic.body_height/scale;

             gdk_draw_line(pmw->generic.widget.window,body_gc,
                           i,pmw->generic.body_height+pmw->generic.body_start-oldpos,
                           i,pmw->generic.body_height+pmw->generic.body_start-pos);
            }
         }

       if(pmw->grid_drawn==1)
          for(i=1;i<pmw->grid_num;i++)
            {
             pos=i*pmw->generic.body_height/pmw->grid_num;
             gdk_draw_line(pmw->generic.widget.window,grid_gc,
                           0                                   ,pmw->generic.body_height+pmw->generic.body_start-pos,
                           pmw->generic.widget.allocation.width,pmw->generic.body_height+pmw->generic.body_start-pos);
            }
       else
          if(pmw->grid_drawn==-1)
            {
             pos=pmw->grid_maxvis*pmw->generic.body_height/pmw->grid_num;
             gdk_draw_line(pmw->generic.widget.window,grid_gc,
                           0                                   ,pmw->generic.body_height+pmw->generic.body_start-pos,
                           pmw->generic.widget.allocation.width,pmw->generic.body_height+pmw->generic.body_start-pos);
            }
      }
    else
      {
       val=pmw->data[(pmw->data_num-1+pmw->data_index)%pmw->data_num];
       pos=val*pmw->generic.body_height/scale;

       gdk_window_copy_area(pmw->generic.widget.window,grid_gc,
                            0,pmw->generic.body_start,
                            pmw->generic.widget.window,
                            1,pmw->generic.body_start,
                            pmw->generic.widget.allocation.width-1,pmw->generic.body_height);

       gdk_window_clear_area(pmw->generic.widget.window,
                             pmw->generic.widget.allocation.width-1,pmw->generic.body_start,
                             1,pmw->generic.body_height);

       if(pmw->line_solid)
          gdk_draw_line(pmw->generic.widget.window,body_gc,
                        (pmw->data_num-1),pmw->generic.body_height+pmw->generic.body_start,
                        (pmw->data_num-1),pmw->generic.body_height+pmw->generic.body_start-pos);
       else
         {
          gushort oldval=pmw->data[(pmw->data_num-2+pmw->data_index)%pmw->data_num];
          gshort oldpos=oldval*pmw->generic.body_height/scale;

          gdk_draw_line(pmw->generic.widget.window,body_gc,
                        (pmw->data_num-1),pmw->generic.body_height+pmw->generic.body_start-oldpos,
                        (pmw->data_num-1),pmw->generic.body_height+pmw->generic.body_start-pos);
         }

       if(pmw->grid_drawn==1)
          for(i=1;i<pmw->grid_num;i++)
            {
             pos=i*pmw->generic.body_height/pmw->grid_num;
             gdk_draw_point(pmw->generic.widget.window,grid_gc,
                            pmw->generic.widget.allocation.width-1,pmw->generic.body_height+pmw->generic.body_start-pos);
            }
       else
          if(pmw->grid_drawn==-1)
            {
             pos=pmw->grid_maxvis*pmw->generic.body_height/pmw->grid_num;
             gdk_draw_point(pmw->generic.widget.window,grid_gc,
                            pmw->generic.widget.allocation.width-1,pmw->generic.body_height+pmw->generic.body_start-pos);
            }
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Set the grid colour of the Widget.

  ProcMeterGraph *pmw The widget to set.

  GdkColor grid_color The grid.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGraphSetGridColour(ProcMeterGraph *pmw,GdkColor grid_color)
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

 GraphUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the minimum number of grid lines in the Widget.

  ProcMeterGraph *pmw The Widget to set.

  gint grid_min The minimum number of lines.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGraphSetGridMin(ProcMeterGraph *pmw,gint grid_min)
{
 if(grid_min<0)
   {
    pmw->grid_min=-grid_min;
    pmw->grid_drawn=0;
   }
 else if(grid_min>0)
   {
    pmw->grid_min=grid_min;
    pmw->grid_drawn=1;
   }
 else /* if(grid_min==0) */
   {
    pmw->grid_min=1;
    pmw->grid_drawn=1;
   }

 if(grid_min>pmw->grid_max && pmw->grid_max)
    pmw->grid_min=pmw->grid_max;

 if(pmw->grid_min>=pmw->grid_num)
    pmw->grid_num=pmw->grid_min;

 GraphResize(pmw);

 GraphUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the maximum number of grid lines in the Widget.

  ProcMeterGraph *pmw The Widget to set.

  gint grid_max The maximum number of lines.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGraphSetGridMax(ProcMeterGraph *pmw,gint grid_max)
{
 if(grid_max<0)
    pmw->grid_max=0;
 else
    pmw->grid_max=grid_max;

 if(grid_max && grid_max<pmw->grid_min)
    pmw->grid_max=pmw->grid_min;

 GraphResize(pmw);

 GraphUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the graph to solid or line.

  ProcMeterGraph *pmw The Widget to set.

  gboolean solid The solidity of the graph.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGraphSetSolid(ProcMeterGraph *pmw,gboolean solid)
{
 pmw->line_solid=solid;

 GraphUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the grid units for the widget

  ProcMeterGraph *pmw The widget to set.

  gchar *units The grid units.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGraphSetGridUnits(ProcMeterGraph *pmw,gchar *units)
{
 if(pmw->grid_units!=empty_string)
    free(pmw->grid_units);
 pmw->grid_units=(char*)malloc(strlen(units)+1);
 strcpy(pmw->grid_units,units);

 GraphResize(pmw);

 GraphUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a data point to the ProcMeter Graph Widget.

  ProcMeterGraph *pmw The ProcMeter Graph Widget.

  gushort datum The data point to add.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGraphAddDatum(ProcMeterGraph *pmw,gushort datum)
{
 gushort old_datum,new_data_max=pmw->data_max;
 int i;

 old_datum=pmw->data[pmw->data_index];
 pmw->data[pmw->data_index]=datum;

 pmw->data_index=(pmw->data_index+1)%pmw->data_num;

 if(datum>new_data_max)
    new_data_max=datum;
 else
    if(old_datum==new_data_max)
       for(i=new_data_max=0;i<pmw->data_num;i++)
          if(pmw->data[i]>new_data_max)
             new_data_max=pmw->data[i];

 if(new_data_max!=pmw->data_max)
   {
    int new_grid_num=(new_data_max+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;

    if(new_grid_num<pmw->grid_min)
       new_grid_num=pmw->grid_min;
    if(pmw->grid_max && new_grid_num>pmw->grid_max)
       new_grid_num=pmw->grid_max;

    pmw->data_max=new_data_max;

    if(new_grid_num!=pmw->grid_num)
      {
       pmw->grid_num=new_grid_num;

       if(pmw->grid_num>pmw->grid_maxvis && pmw->grid_drawn)
          pmw->grid_drawn=-1;
       if(pmw->grid_num<=pmw->grid_maxvis && pmw->grid_drawn)
          pmw->grid_drawn=1;

       GraphUpdate(pmw,TRUE);
      }
   }

 GraphUpdate(pmw,FALSE);
}
