/***************************************
  ProcMeter Bar Widget Source file (for ProcMeter3 3.6).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996-2012 Andrew M. Bishop
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
static void destroy(GtkWidget *widget);
static void realize(GtkWidget *widget);
static void get_preferred_width(GtkWidget *widget,gint *minimal_width,gint *natural_width);
static void get_preferred_height(GtkWidget *widget,gint *minimal_height,gint *natural_height);
static void size_allocate(GtkWidget *widget,GtkAllocation *allocation);
static gboolean draw(GtkWidget *widget,cairo_t *cr);

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
    GTypeInfo pmw_info={sizeof(ProcMeterBarClass),
                        NULL,
                        NULL,
                        (GClassInitFunc) procmeterbar_class_init,
                        NULL,
                        NULL,
                        sizeof(ProcMeterBar),
                        0,
                        (GInstanceInitFunc) procmeterbar_init,
                        NULL};

    pmw_type=g_type_register_static(gtk_procmetergeneric_get_type(),"ProcMeterBar",&pmw_info,0);
   }

 return(pmw_type);
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise the Widget class

  ProcMeterBarClass *class The class of widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmeterbar_class_init(ProcMeterBarClass *class)
{
 GtkWidgetClass *widget_class;

 g_return_if_fail(class!=NULL);

 widget_class=(GtkWidgetClass*)class;

 class->resize=BarResize;
 class->update=BarUpdate;

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

  ProcMeterBar *pmw The Widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmeterbar_init(ProcMeterBar *pmw)
{
 int i;

 g_return_if_fail(pmw!=NULL);

 /* The grid parts. */

 pmw->grid_units=empty_string;

 gtk_style_context_get_color(gtk_widget_get_style_context(&pmw->generic.widget),GTK_STATE_FLAG_NORMAL,&pmw->grid_color);

 pmw->grid_drawn=1;
 pmw->grid_min=1;

 pmw->grid_max=0;

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

 pmw=g_object_new(gtk_procmeterbar_get_type(),NULL);

 return(GTK_WIDGET(pmw));
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a Widget

  GtkWidget *widget The widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void destroy(GtkWidget *widget)
{
 ProcMeterBar *pmw;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERBAR(widget));

 pmw=GTK_PROCMETERBAR(widget);

 if(pmw->grid_units!=empty_string)
   {
    free(pmw->grid_units);
    pmw->grid_units=empty_string;
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
 ProcMeterBar *pmw;
 GdkWindowAttr attributes;
 GtkAllocation allocation;
 gint attributes_mask;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERBAR(widget));

 gtk_widget_set_realized(widget,TRUE);
 pmw=GTK_PROCMETERBAR(widget);

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

 BarUpdate(pmw,TRUE);
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
 g_return_if_fail(GTK_IS_PROCMETERBAR(widget));
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
 ProcMeterBar *pmw;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERBAR(widget));
 g_return_if_fail(minimal_height!=NULL);
 g_return_if_fail(natural_height!=NULL);

 pmw=GTK_PROCMETERBAR(widget);

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
 g_return_if_fail(GTK_IS_PROCMETERBAR(widget));
 g_return_if_fail(allocation!=NULL);

 gtk_widget_set_allocation(widget,allocation);
 if(gtk_widget_get_realized(widget))
   {
    ProcMeterBar *pmw=GTK_PROCMETERBAR(widget);

    gdk_window_move_resize(gtk_widget_get_window(widget),
                           allocation->x,allocation->y,
                           allocation->width,allocation->height);

    BarResize(pmw);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Bar Widget.

  gboolean draw Returns false

  GtkWidget *widget The Widget to redisplay.

  cairo_t *cr A cairo object describing the position to draw.
  ++++++++++++++++++++++++++++++++++++++*/

static gboolean draw(GtkWidget *widget,cairo_t *cr)
{
 ProcMeterBar *pmw;

 g_return_val_if_fail(widget!=NULL,FALSE);
 g_return_val_if_fail(GTK_IS_PROCMETERBAR(widget),FALSE);

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
 int allocation_width;
 cairo_t *cr;
 PangoLayout *layout;
 int width,height;

 g_return_if_fail(pmw!=NULL);

 if(parent_class->resize)
    parent_class->resize(&pmw->generic);

 pmw->generic.label_x=2;

 /* The grid parts. */

 allocation_width=gtk_widget_get_allocated_width(&pmw->generic.widget);

 cr=gdk_cairo_create(gtk_widget_get_root_window(&pmw->generic.widget));

 layout=pango_cairo_create_layout(cr);
 if(pmw->generic.label_font)
    pango_layout_set_font_description(layout,pmw->generic.label_font);
 else
    pango_layout_set_font_description(layout,gtk_style_context_get_font(gtk_widget_get_style_context(&pmw->generic.widget),GTK_STATE_FLAG_NORMAL));

 pango_layout_set_text(layout,pmw->grid_units,-1);
 pango_layout_get_pixel_size(layout,&width,&height);

 cairo_destroy(cr);

 pmw->grid_units_x=allocation_width-width;

 pmw->grid_maxvis=allocation_width/3;

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

 if(gtk_widget_get_visible(&pmw->generic.widget))
   {
    int allocation_width;
    cairo_t *cr;
    int i;
    int scale=PROCMETER_GRAPH_SCALE*pmw->grid_num;
    gshort pos;
    gshort top_average_bottom,bottom_average_top,average_size;

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
      }
    else
      {
       cairo_set_source_rgb(cr,pmw->generic.body_bg_color.red,pmw->generic.body_bg_color.green,pmw->generic.body_bg_color.blue);

       cairo_rectangle(cr,0,pmw->generic.body_start,allocation_width,pmw->generic.body_height);
       cairo_fill(cr);
      }

    pos=pmw->data_sum*allocation_width/(scale*2);

    top_average_bottom=pmw->generic.body_start+2*(pmw->generic.body_height>>3);
    bottom_average_top=pmw->generic.body_start+pmw->generic.body_height-2*(pmw->generic.body_height>>3);
    average_size=pmw->generic.body_height>>3;

    cairo_set_source_rgb(cr,pmw->generic.body_fg_color.red,pmw->generic.body_fg_color.green,pmw->generic.body_fg_color.blue);

    cairo_rectangle(cr,pos-average_size,top_average_bottom-average_size,average_size,average_size);
    cairo_fill(cr);

    cairo_rectangle(cr,pos-average_size,bottom_average_top             ,average_size,average_size);
    cairo_fill(cr);

    pos=pmw->data[pmw->data_index]*allocation_width/scale;

    cairo_rectangle(cr,0,top_average_bottom+1,pos,bottom_average_top-top_average_bottom-2);
    cairo_fill(cr);

    cairo_set_source_rgb(cr,pmw->grid_color.red,pmw->grid_color.green,pmw->grid_color.blue);

    if(pmw->grid_drawn==1)
       for(i=1;i<pmw->grid_num;i++)
         {
          pos=i*allocation_width/pmw->grid_num;
          cairo_move_to(cr,pos+0.5,pmw->generic.body_start);
          cairo_line_to(cr,pos+0.5,pmw->generic.body_height+pmw->generic.body_start);
          cairo_stroke(cr);
         }
    else
       if(pmw->grid_drawn==-1)
         {
          pos=pmw->grid_maxvis*allocation_width/pmw->grid_num;
          cairo_move_to(cr,pos+0.5,pmw->generic.body_start);
          cairo_line_to(cr,pos+0.5,pmw->generic.body_height+pmw->generic.body_start);
          cairo_stroke(cr);
         }

    cairo_destroy(cr);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Set the grid colour of the Widget.

  ProcMeterBar *pmw The widget to set.

  GdkRGBA *grid_color The grid.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterBarSetGridColour(ProcMeterBar *pmw,GdkRGBA *grid_color)
{
 pmw->grid_color=*grid_color;

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
 else
    pmw->grid_max=grid_max;

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

 BarUpdate(pmw,FALSE);
}
