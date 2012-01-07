/***************************************
  ProcMeter Generic Widget Source file (for ProcMeter 3.6).
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

static void procmetergeneric_class_init(ProcMeterGenericClass *class);
static void procmetergeneric_init(ProcMeterGeneric *pmw);
static void destroy(GtkWidget *widget);
static void realize(GtkWidget *widget);
static gboolean draw(GtkWidget *widget,cairo_t *cr);
static void get_preferred_width(GtkWidget *widget,gint *minimal_width,gint *natural_width);
static void get_preferred_height(GtkWidget *widget,gint *minimal_height,gint *natural_height);
static void size_allocate(GtkWidget *widget,GtkAllocation *allocation);

static void GenericResize(ProcMeterGeneric *pmw);
static void GenericUpdate(ProcMeterGeneric *pmw);

static char *empty_string="";

static GtkWidgetClass *parent_class=NULL;


/*++++++++++++++++++++++++++++++++++++++
  Returns the type of a Widget.

  guint procmetergeneric_get_type Returns a unique pointer to the Widget type.
  ++++++++++++++++++++++++++++++++++++++*/

guint gtk_procmetergeneric_get_type(void)
{
 static guint pmw_type=0;

 if(!pmw_type)
   {
    GTypeInfo pmw_info={sizeof(ProcMeterGenericClass),
                        NULL,
                        NULL,
                        (GClassInitFunc) procmetergeneric_class_init,
                        NULL,
                        NULL,
                        sizeof(ProcMeterGeneric),
                        0,
                        (GInstanceInitFunc) procmetergeneric_init,
                        NULL};

    pmw_type=g_type_register_static(gtk_widget_get_type(),"ProcMeterGeneric",&pmw_info,0);
   }

 return(pmw_type);
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise the Widget class

  ProcMeterGenericClass *class The class of widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmetergeneric_class_init(ProcMeterGenericClass *class)
{
 GtkWidgetClass *widget_class;

 g_return_if_fail(class!=NULL);

 widget_class=(GtkWidgetClass*)class;

 class->resize=GenericResize;
 class->update=GenericUpdate;

 parent_class=g_type_class_ref(gtk_widget_get_type());

 widget_class->destroy=destroy;
 widget_class->realize=realize;
 widget_class->draw=draw;
 widget_class->get_preferred_width=get_preferred_width;
 widget_class->get_preferred_height=get_preferred_height;
 widget_class->size_allocate=size_allocate;
}


/*++++++++++++++++++++++++++++++++++++++
  Create a new Widget.

  GtkWidget* gtk_procmetergeneric_new Returns the new widgets.
  ++++++++++++++++++++++++++++++++++++++*/

GtkWidget* gtk_procmetergeneric_new(void)
{
 ProcMeterGeneric *new;

 new=g_object_new(gtk_procmetergeneric_get_type(),NULL);

 return(GTK_WIDGET(new));
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise an instance of the Widget

  ProcMeterGeneric *pmw The Widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmetergeneric_init(ProcMeterGeneric *pmw)
{
 g_return_if_fail(pmw!=NULL);

 /* The body parts. */

 gtk_style_context_get_color(gtk_widget_get_style_context(&pmw->widget),GTK_STATE_FLAG_NORMAL,&pmw->body_fg_color);
 gtk_style_context_get_background_color(gtk_widget_get_style_context(&pmw->widget),GTK_STATE_FLAG_NORMAL,&pmw->body_bg_color);

 /* The label parts. */

 pmw->label_pos=ProcMeterLabelBottom;

 pmw->label_string=empty_string;

 pmw->label_font=NULL;

 gtk_style_context_get_color(gtk_widget_get_style_context(&pmw->widget),GTK_STATE_FLAG_NORMAL,&pmw->label_color);

 /* The rest of the sizing. */

 GenericResize(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a Widget

  GtkWidget *widget The widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void destroy(GtkWidget *widget)
{
 ProcMeterGeneric *pmw;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGENERIC(widget));

 pmw=GTK_PROCMETERGENERIC(widget);

 if(pmw->label_string!=empty_string)
   {
    free(pmw->label_string);
    pmw->label_string=empty_string;
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
 ProcMeterGeneric *pmw;
 GdkWindowAttr attributes;
 GtkAllocation allocation;
 gint attributes_mask;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGENERIC(widget));

 gtk_widget_set_realized(widget,TRUE);
 pmw=GTK_PROCMETERGENERIC(widget);

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

 GenericUpdate(pmw);
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
 g_return_if_fail(GTK_IS_PROCMETERGENERIC(widget));
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
 ProcMeterGeneric *pmw;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGENERIC(widget));
 g_return_if_fail(minimal_height!=NULL);
 g_return_if_fail(natural_height!=NULL);

 pmw=GTK_PROCMETERGENERIC(widget);

 *minimal_height=*natural_height=10+pmw->label_height;
}


/*++++++++++++++++++++++++++++++++++++++
  Change to the size that has been specified by the container.

  GtkWidget *widget The widget that has been resized.

  GtkAllocation *allocation The size information.
  ++++++++++++++++++++++++++++++++++++++*/

static void size_allocate(GtkWidget *widget,GtkAllocation *allocation)
{
 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGENERIC(widget));
 g_return_if_fail(allocation!=NULL);

 gtk_widget_set_allocation(widget,allocation);
 if(gtk_widget_get_realized(widget))
   {
    ProcMeterGeneric *pmw=GTK_PROCMETERGENERIC(widget);

    gdk_window_move_resize(gtk_widget_get_window(widget),
                           allocation->x,allocation->y,
                           allocation->width,allocation->height);

    GenericResize(pmw);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Generic Widget.

  gboolean draw Returns false

  GtkWidget *widget The Widget to redisplay.

  cairo_t *cr A cairo object describing the position to draw.
  ++++++++++++++++++++++++++++++++++++++*/

static gboolean draw(GtkWidget *widget,cairo_t *cr)
{
 ProcMeterGeneric *pmw;

 g_return_val_if_fail(widget!=NULL,FALSE);
 g_return_val_if_fail(GTK_IS_PROCMETERGENERIC(widget),FALSE);

 pmw=GTK_PROCMETERGENERIC(widget);

 GenericUpdate(pmw);

 return(FALSE);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform all of the sizing on the Widget when it is created/resized.

  ProcMeterGeneric *pmw The Widget to resize.
  ++++++++++++++++++++++++++++++++++++++*/

static void GenericResize(ProcMeterGeneric *pmw)
{
 int allocation_width,allocation_height;

 g_return_if_fail(pmw!=NULL);

 allocation_width =gtk_widget_get_allocated_width (&pmw->widget);
 allocation_height=gtk_widget_get_allocated_height(&pmw->widget);

 /* The label parts. */

 if(pmw->label_pos)
   {
    PangoRectangle ink_rect;
    PangoLayout *layout;
    cairo_t *cr;
    int width,height;

    cr=gdk_cairo_create(gtk_widget_get_root_window(&pmw->widget));

    layout=pango_cairo_create_layout(cr);
    if(pmw->label_font)
       pango_layout_set_font_description(layout,pmw->label_font);
    else
       pango_layout_set_font_description(layout,gtk_style_context_get_font(gtk_widget_get_style_context(&pmw->widget),GTK_STATE_FLAG_NORMAL));

    pango_layout_set_text(layout,pmw->label_string,-1);
    pango_layout_get_pixel_size(layout,&width,&height);
    pango_layout_get_pixel_extents(layout,&ink_rect,NULL);

    cairo_destroy(cr);

    pmw->label_height=PANGO_ASCENT(ink_rect)+(1+PANGO_DESCENT(ink_rect))+2;
    pmw->label_x=(allocation_width-width)/2;
    if(pmw->label_pos==ProcMeterLabelTop)
       pmw->label_y=pmw->label_height-1-(1+PANGO_DESCENT(ink_rect));
    else
       pmw->label_y=allocation_height-(1+PANGO_DESCENT(ink_rect));
   }
 else
   {
    pmw->label_height=0;
    pmw->label_x=0;
    pmw->label_y=0;
   }

 /* The body parts. */

 pmw->body_height=allocation_height-pmw->label_height;

 if(pmw->label_pos==ProcMeterLabelTop)
    pmw->body_start=pmw->label_height;
 else
    pmw->body_start=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Update the display of the generic part of the widget.

  ProcMeterGeneric *pmw The Widget to update.
  ++++++++++++++++++++++++++++++++++++++*/

static void GenericUpdate(ProcMeterGeneric *pmw)
{
 g_return_if_fail(pmw!=NULL);

 if(gtk_widget_get_visible(&pmw->widget))
   {
    int allocation_width,allocation_height;
    cairo_t *cr;

    cr=gdk_cairo_create(gtk_widget_get_window(&pmw->widget));
    cairo_set_line_width(cr,1.0);

    allocation_width =gtk_widget_get_allocated_width (&pmw->widget);
    allocation_height=gtk_widget_get_allocated_height(&pmw->widget);

    cairo_set_source_rgb(cr,pmw->body_bg_color.red,pmw->body_bg_color.green,pmw->body_bg_color.blue);
    cairo_rectangle(cr,0,0,allocation_width,allocation_height);
    cairo_fill(cr);

    if(pmw->label_pos)
      {
       PangoLayout *layout;

       cairo_set_source_rgb(cr,pmw->label_color.red,pmw->label_color.green,pmw->label_color.blue);

       layout=pango_cairo_create_layout(cr);
       if(pmw->label_font)
          pango_layout_set_font_description(layout,pmw->label_font);
       else
          pango_layout_set_font_description(layout,gtk_style_context_get_font(gtk_widget_get_style_context(&pmw->widget),GTK_STATE_FLAG_NORMAL));

       cairo_move_to(cr,pmw->label_x,pmw->label_y);
       pango_layout_set_text(layout,pmw->label_string,-1);
       pango_cairo_show_layout(cr,layout);

       if(pmw->label_pos==ProcMeterLabelTop)
         {
          cairo_move_to(cr,0               ,pmw->label_height-1+0.5);
          cairo_line_to(cr,allocation_width,pmw->label_height-1+0.5);
          cairo_stroke(cr);
         }
       else
         {
          cairo_move_to(cr,0               ,pmw->body_height+0.5);
          cairo_line_to(cr,allocation_width,pmw->body_height+0.5);
          cairo_stroke(cr);
         }
      }

    cairo_destroy(cr);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Set the body colours of the Widget.

  ProcMeterGeneric *pmw The widget to set.

  GdkRGBA *body_bg_color The body background.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGenericSetBackgroundColour(ProcMeterGeneric *pmw,GdkRGBA *body_bg_color)
{
 pmw->body_bg_color=*body_bg_color;

 GenericUpdate(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the body colours of the Widget.

  ProcMeterGeneric *pmw The widget to set.

  GdkRGBA *body_fg_color The body foreground.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGenericSetForegroundColour(ProcMeterGeneric *pmw,GdkRGBA *body_fg_color)
{
 pmw->body_fg_color=*body_fg_color;

 GenericUpdate(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the label colour of the Widget.

  ProcMeterGeneric *pmw The widget to set.

  GdkRGBA *label_color The label foreground.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGenericSetLabelColour(ProcMeterGeneric *pmw,GdkRGBA *label_color)
{
 pmw->label_color=*label_color;

 GenericUpdate(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the label position of the Widget.

  ProcMeterGeneric *pmw The widget to set.

  int label_position The position of the label.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGenericSetLabelPosition(ProcMeterGeneric *pmw,int label_position)
{
 if((label_position==ProcMeterLabelTop) ||
    (label_position==ProcMeterLabelNone) ||
    (label_position==ProcMeterLabelBottom))
   {
    pmw->label_pos=label_position;

    GenericResize(pmw);

    GenericUpdate(pmw);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Set the label font for the widget

  ProcMeterGeneric *pmw The widget to set.

  PangoFontDescription *font The font to use.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGenericSetLabelFont(ProcMeterGeneric *pmw,PangoFontDescription *font)
{
 pmw->label_font=font;

 GenericResize(pmw);

 GenericUpdate(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the label for the widget

  ProcMeterGeneric *pmw The widget to set.

  gchar *label The name of the label.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGenericSetLabel(ProcMeterGeneric *pmw,gchar *label)
{
 pmw->label_string=(char*)malloc(strlen(label)+1);
 strcpy(pmw->label_string,label);

 GenericUpdate(pmw);
}
