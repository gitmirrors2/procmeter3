/***************************************
  ProcMeter Graph Widget Source file (for ProcMeter3 3.6a).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996-2012,19 Andrew M. Bishop
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
static void destroy(GtkWidget *widget);
static void realize(GtkWidget *widget);
static void get_preferred_width(GtkWidget *widget,gint *minimal_width,gint *natural_width);
static void get_preferred_height(GtkWidget *widget,gint *minimal_height,gint *natural_height);
static void size_allocate(GtkWidget *widget,GtkAllocation *allocation);
static gboolean draw(GtkWidget *widget,cairo_t *cr);

static void GraphResize(ProcMeterGraph *pmw);
static void GraphUpdate(ProcMeterGraph *pmw,gboolean all);

static char *empty_string="";

static ProcMeterGenericClass *parent_class=NULL;


/*++++++++++++++++++++++++++++++++++++++
  Returns the type of a Widget.

  GType gtk_procmetergraph_get_type Returns a unique pointer to the Widget type.
  ++++++++++++++++++++++++++++++++++++++*/

GType gtk_procmetergraph_get_type(void)
{
 static GType pmw_type = 0;

 if(!pmw_type)
   {
    GTypeInfo pmw_info={sizeof(ProcMeterGraphClass),
                        NULL,
                        NULL,
                        (GClassInitFunc) procmetergraph_class_init,
                        NULL,
                        NULL,
                        sizeof(ProcMeterGraph),
                        0,
                        (GInstanceInitFunc) procmetergraph_init,
                        NULL};

    pmw_type=g_type_register_static(gtk_procmetergeneric_get_type(),"ProcMeterGraph",&pmw_info,0);
   }

 return(pmw_type);
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise the Widget class

  ProcMeterGraphClass *class The class of widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmetergraph_class_init(ProcMeterGraphClass *class)
{
 GtkWidgetClass *widget_class;

 g_return_if_fail(class!=NULL);

 widget_class=(GtkWidgetClass*)class;

 class->resize=GraphResize;
 class->update=GraphUpdate;

 parent_class=g_type_class_ref(gtk_procmetergeneric_get_type());

 widget_class->destroy=destroy;
 widget_class->realize=realize;
 widget_class->draw=draw;
 widget_class->get_preferred_width=get_preferred_width;
 widget_class->get_preferred_height=get_preferred_height;
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

 gtk_style_context_get_color(gtk_widget_get_style_context(&pmw->generic.widget),GTK_STATE_FLAG_NORMAL,&pmw->grid_color);

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

 pmw=g_object_new(gtk_procmetergraph_get_type(),NULL);

 return(GTK_WIDGET(pmw));
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a Widget

  GtkWidget *widget The widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void destroy(GtkWidget *widget)
{
 ProcMeterGraph *pmw;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGRAPH(widget));

 pmw=GTK_PROCMETERGRAPH(widget);

 if(pmw->grid_units!=empty_string)
   {
    free(pmw->grid_units);
    pmw->grid_units=empty_string;
   }
 if(pmw->data)
   {
    free(pmw->data);
    pmw->data=NULL;
   }

 if(GTK_WIDGET_CLASS(parent_class)->destroy)
    (*GTK_WIDGET_CLASS(parent_class)->destroy)(widget);
}


/*++++++++++++++++++++++++++++++++++++++
  Realize a widget.

  GtkWidget *widget The widget to realize.
  ++++++++++++++++++++++++++++++++++++++*/

static void realize(GtkWidget *widget)
{
 ProcMeterGraph *pmw;
 GdkWindowAttr attributes;
 GtkAllocation allocation;
 gint attributes_mask;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGRAPH(widget));

 gtk_widget_set_realized(widget,TRUE);
 pmw=GTK_PROCMETERGRAPH(widget);

 gtk_widget_get_allocation(widget,&allocation);

 attributes.x=allocation.x;
 attributes.y=allocation.y;
 attributes.width=allocation.width;
 attributes.height=allocation.height;
 attributes.wclass=GDK_INPUT_OUTPUT;
 attributes.window_type=GDK_WINDOW_CHILD;
 attributes.event_mask=gtk_widget_get_events(widget)|GDK_EXPOSURE_MASK|GDK_BUTTON_PRESS_MASK;
 attributes.visual=gtk_widget_get_visual(widget);

 attributes_mask=GDK_WA_X|GDK_WA_Y|GDK_WA_VISUAL;
 gtk_widget_set_window(widget,gdk_window_new(gtk_widget_get_parent_window(widget),&attributes,attributes_mask));

 gdk_window_set_user_data(gtk_widget_get_window(widget),widget);

 GraphUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Return the preferred width of the widget

  GtkWidget *widget The widget whose width is requested.

  gint *minimal_width Returns the minimal width.

  gint *natural_width Returns the preferred width.
  ++++++++++++++++++++++++++++++++++++++*/

static void get_preferred_width(GtkWidget *widget,gint *minimal_width,gint *natural_width)
{
 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGRAPH(widget));
 g_return_if_fail(minimal_width!=NULL);
 g_return_if_fail(natural_width!=NULL);

 *minimal_width=*natural_width=50; /* arbitrary */
}


/*++++++++++++++++++++++++++++++++++++++
  Return the preferred height of the widget

  GtkWidget *widget The widget whose height is requested.

  gint *minimal_height Returns the minimal height.

  gint *natural_height Returns the preferred height.
  ++++++++++++++++++++++++++++++++++++++*/

static void get_preferred_height(GtkWidget *widget,gint *minimal_height,gint *natural_height)
{
 ProcMeterGraph *pmw;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGRAPH(widget));
 g_return_if_fail(minimal_height!=NULL);
 g_return_if_fail(natural_height!=NULL);

 pmw=GTK_PROCMETERGRAPH(widget);

 *minimal_height=*natural_height=20+pmw->generic.label_height;
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

 gtk_widget_set_allocation(widget,allocation);
 if(gtk_widget_get_realized(widget))
   {
    ProcMeterGraph *pmw=GTK_PROCMETERGRAPH(widget);

    gdk_window_move_resize(gtk_widget_get_window(widget),
                           allocation->x,allocation->y,
                           allocation->width,allocation->height);

    GraphResize(pmw);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Graph Widget.

  gboolean draw Returns false

  GtkWidget *widget The Widget to redisplay.

  cairo_t *cr A cairo object describing the position to draw.
  ++++++++++++++++++++++++++++++++++++++*/

static gboolean draw(GtkWidget *widget,cairo_t *cr)
{
 ProcMeterGraph *pmw;

 g_return_val_if_fail(widget!=NULL,FALSE);
 g_return_val_if_fail(GTK_IS_PROCMETERGRAPH(widget),FALSE);

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
 int allocation_width;
 cairo_t *cr;
 PangoLayout *layout;
 int width,height;

 g_return_if_fail(pmw!=NULL);

 if(parent_class->resize)
    parent_class->resize(&pmw->generic);

 allocation_width=gtk_widget_get_allocated_width(&pmw->generic.widget);

 if(pmw->data_num!=allocation_width)
   {
    int i,old_num=pmw->data_num;
    gushort* old_data=pmw->data;

    pmw->data_num=allocation_width;
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

 cr=gdk_cairo_create(gtk_widget_get_root_window(&pmw->generic.widget));

 layout=pango_cairo_create_layout(cr);
 if(pmw->generic.label_font)
    pango_layout_set_font_description(layout,pmw->generic.label_font);
 else
    pango_layout_set_font_description(layout,gtk_style_context_get_font(gtk_widget_get_style_context(&pmw->generic.widget),GTK_STATE_FLAG_NORMAL));

 pango_layout_set_text(layout,pmw->grid_units,-1);
 pango_layout_get_pixel_size(layout,&width,&height);

 cairo_destroy(cr);

 pmw->grid_units_x=allocation_width-width-2;

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

 if(gtk_widget_get_visible(&pmw->generic.widget))
   {
    int allocation_width;
    cairo_t *cr;
    int i;
    int scale=PROCMETER_GRAPH_SCALE*pmw->grid_num;
    gushort val;
    gshort pos;

    cr=gdk_cairo_create(gtk_widget_get_window(&pmw->generic.widget));
    cairo_set_line_width(cr,1.0);

    allocation_width=gtk_widget_get_allocated_width(&pmw->generic.widget);

    if(all)
      {
       if(parent_class->update)
          parent_class->update(&pmw->generic);

       if(pmw->generic.label_pos!=ProcMeterLabelNone)
         {
          PangoLayout *layout;

          cairo_set_source_rgb(cr,pmw->generic.label_color.red,pmw->generic.label_color.green,pmw->generic.label_color.blue);

          layout=pango_cairo_create_layout(cr);
          if(pmw->generic.label_font)
             pango_layout_set_font_description(layout,pmw->generic.label_font);
          else
             pango_layout_set_font_description(layout,gtk_style_context_get_font(gtk_widget_get_style_context(&pmw->generic.widget),GTK_STATE_FLAG_NORMAL));

          cairo_move_to(cr,pmw->grid_units_x,pmw->generic.label_y);
          pango_layout_set_text(layout,pmw->grid_units,-1);
          pango_cairo_show_layout(cr,layout);
         }

       cairo_set_source_rgb(cr,pmw->generic.body_fg_color.red,pmw->generic.body_fg_color.green,pmw->generic.body_fg_color.blue);

       for(i=0;i<pmw->data_num;i++)
         {
          val=pmw->data[(i+pmw->data_index)%pmw->data_num];
          pos=val*pmw->generic.body_height/scale;

          if(pmw->line_solid)
            {
             cairo_move_to(cr,i+0.5,pmw->generic.body_height+pmw->generic.body_start);
             cairo_line_to(cr,i+0.5,pmw->generic.body_height+pmw->generic.body_start-pos);
             cairo_stroke(cr);
            }
          else if(i)
            {
             gushort oldval=pmw->data[(i-1+pmw->data_index)%pmw->data_num];
             gshort oldpos=oldval*pmw->generic.body_height/scale;

             cairo_move_to(cr,i+0.5,pmw->generic.body_height+pmw->generic.body_start-oldpos);
             cairo_line_to(cr,i+0.5,pmw->generic.body_height+pmw->generic.body_start-pos);
             cairo_stroke(cr);
            }
         }

       cairo_set_source_rgb(cr,pmw->grid_color.red,pmw->grid_color.green,pmw->grid_color.blue);

       if(pmw->grid_drawn==1)
          for(i=1;i<pmw->grid_num;i++)
            {
             pos=i*pmw->generic.body_height/pmw->grid_num;
             cairo_move_to(cr,0               ,pmw->generic.body_height+pmw->generic.body_start-pos+0.5);
             cairo_line_to(cr,allocation_width,pmw->generic.body_height+pmw->generic.body_start-pos+0.5);
             cairo_stroke(cr);
            }
       else
          if(pmw->grid_drawn==-1)
            {
             pos=pmw->grid_maxvis*pmw->generic.body_height/pmw->grid_num;
             cairo_move_to(cr,0               ,pmw->generic.body_height+pmw->generic.body_start-pos+0.5);
             cairo_line_to(cr,allocation_width,pmw->generic.body_height+pmw->generic.body_start-pos+0.5);
             cairo_stroke(cr);
            }
      }
    else
      {
       val=pmw->data[(pmw->data_num-1+pmw->data_index)%pmw->data_num];
       pos=val*pmw->generic.body_height/scale;

       cairo_save(cr);

       cairo_push_group(cr);
       gdk_cairo_set_source_window(cr,gtk_widget_get_window(&pmw->generic.widget),-1,0);
       cairo_rectangle(cr,0,pmw->generic.body_start,allocation_width,pmw->generic.body_height);
       cairo_clip(cr);
       cairo_paint(cr);
       cairo_pop_group_to_source(cr);
       cairo_paint(cr);

       cairo_restore(cr);

       cairo_set_source_rgb(cr,pmw->generic.body_bg_color.red,pmw->generic.body_bg_color.green,pmw->generic.body_bg_color.blue);

       cairo_move_to(cr,allocation_width-1+0.5,pmw->generic.body_start);
       cairo_line_to(cr,allocation_width-1+0.5,pmw->generic.body_start+pmw->generic.body_height);
       cairo_stroke(cr);

       cairo_set_source_rgb(cr,pmw->generic.body_fg_color.red,pmw->generic.body_fg_color.green,pmw->generic.body_fg_color.blue);

       if(pmw->line_solid)
         {
          cairo_move_to(cr,(pmw->data_num-1)+0.5,pmw->generic.body_height+pmw->generic.body_start);
          cairo_line_to(cr,(pmw->data_num-1)+0.5,pmw->generic.body_height+pmw->generic.body_start-pos);
          cairo_stroke(cr);
         }
       else
         {
          gushort oldval=pmw->data[(pmw->data_num-2+pmw->data_index)%pmw->data_num];
          gshort oldpos=oldval*pmw->generic.body_height/scale;

          cairo_move_to(cr,(pmw->data_num-1)+0.5,pmw->generic.body_height+pmw->generic.body_start-oldpos);
          cairo_line_to(cr,(pmw->data_num-1)+0.5,pmw->generic.body_height+pmw->generic.body_start-pos);
          cairo_stroke(cr);
         }

       cairo_set_source_rgb(cr,pmw->grid_color.red,pmw->grid_color.green,pmw->grid_color.blue);

       if(pmw->grid_drawn==1)
          for(i=1;i<pmw->grid_num;i++)
            {
             pos=i*pmw->generic.body_height/pmw->grid_num;
             cairo_move_to(cr,allocation_width-1,pmw->generic.body_height+pmw->generic.body_start-pos+0.5);
             cairo_line_to(cr,allocation_width  ,pmw->generic.body_height+pmw->generic.body_start-pos+0.5);
             cairo_stroke(cr);
            }
       else
          if(pmw->grid_drawn==-1)
            {
             pos=pmw->grid_maxvis*pmw->generic.body_height/pmw->grid_num;
             cairo_move_to(cr,allocation_width-1,pmw->generic.body_height+pmw->generic.body_start-pos+0.5);
             cairo_line_to(cr,allocation_width  ,pmw->generic.body_height+pmw->generic.body_start-pos+0.5);
             cairo_stroke(cr);
            }
      }

    cairo_destroy(cr);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Set the grid colour of the Widget.

  ProcMeterGraph *pmw The widget to set.

  GdkRGBA *grid_color The grid.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGraphSetGridColour(ProcMeterGraph *pmw,GdkRGBA *grid_color)
{
 pmw->grid_color=*grid_color;

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
