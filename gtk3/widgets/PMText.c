/***************************************
  ProcMeter Text Widget Source file (for ProcMeter 3.6).
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
#include "PMText.h"

static void procmetertext_class_init (ProcMeterTextClass *class);
static void procmetertext_init(ProcMeterText *pmw);
static void destroy(GtkWidget *widget);
static void realize(GtkWidget *widget);
static gboolean draw(GtkWidget *widget,cairo_t *cr);
static void get_preferred_width(GtkWidget *widget,gint *minimal_width,gint *natural_width);
static void get_preferred_height(GtkWidget *widget,gint *minimal_height,gint *natural_height);
static void size_allocate(GtkWidget *widget,GtkAllocation *allocation);

static void TextResize(ProcMeterText *pmw);
static void TextUpdate(ProcMeterText *pmw,gboolean all);

static char *empty_string="";

static ProcMeterGenericClass *parent_class=NULL;


/*++++++++++++++++++++++++++++++++++++++
  Returns the type of a Widget.

  guint gtk_procmetertext_get_type Returns a unique pointer to the Widget type.
  ++++++++++++++++++++++++++++++++++++++*/

guint gtk_procmetertext_get_type(void)
{
 static guint pmw_type = 0;

 if(!pmw_type)
   {
    GTypeInfo pmw_info={sizeof(ProcMeterTextClass),
                        NULL,
                        NULL,
                        (GClassInitFunc) procmetertext_class_init,
                        NULL,
                        NULL,
                        sizeof(ProcMeterText),
                        0,
                        (GInstanceInitFunc) procmetertext_init,
                        NULL};

    pmw_type=g_type_register_static(gtk_procmetergeneric_get_type(),"ProcMeterText",&pmw_info,0);
   }

 return(pmw_type);
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise the Widget class

  ProcMeterTextClass *class The class of widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmetertext_class_init(ProcMeterTextClass *class)
{
 GtkWidgetClass *widget_class;

 g_return_if_fail(class!=NULL);

 widget_class=(GtkWidgetClass*)class;

 class->resize=TextResize;
 class->update=TextUpdate;

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

  ProcMeterText *pmw The Widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmetertext_init(ProcMeterText *pmw)
{
 g_return_if_fail(pmw!=NULL);

 /* The text parts. */

 pmw->text_string=empty_string;

 pmw->text_font=NULL;

 /* The rest of the sizing. */

 TextResize(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Create a new Widget.

  GtkWidget* gtk_procmetertext_new Returns the new widgets.
  ++++++++++++++++++++++++++++++++++++++*/

GtkWidget* gtk_procmetertext_new(void)
{
 ProcMeterText *pmw;

 pmw=g_object_new(gtk_procmetertext_get_type(),NULL);

 return(GTK_WIDGET(pmw));
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a Widget

  GtkWidget *widget The widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void destroy(GtkWidget *widget)
{
 ProcMeterText *pmw;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERTEXT(widget));

 pmw=GTK_PROCMETERTEXT(widget);

 if(pmw->text_string!=empty_string)
   {
    free(pmw->text_string);
    pmw->text_string=empty_string;
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
 ProcMeterText *pmw;
 GdkWindowAttr attributes;
 GtkAllocation allocation;
 gint attributes_mask;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERTEXT(widget));

 gtk_widget_set_realized(widget,TRUE);
 pmw=GTK_PROCMETERTEXT(widget);

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

 TextUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Return the preferred width of the widget

  GtkWidget *widget The widget whose width is requested.

  gint *minimal_width Returns the minimal width.

  gint *natural_width Returns the preferred width.
  ++++++++++++++++++++++++++++++++++++++*/

static void get_preferred_width(GtkWidget *widget,gint *minimal_width,gint *natural_width)
{
 ProcMeterText *pmw;
 PangoLayout *layout;
 cairo_t *cr;
 int width, height;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERTEXT(widget));
 g_return_if_fail(minimal_width!=NULL);
 g_return_if_fail(natural_width!=NULL);

 pmw=GTK_PROCMETERTEXT(widget);

 cr=gdk_cairo_create(gtk_widget_get_root_window(&pmw->generic.widget));

 layout=pango_cairo_create_layout(cr);
 if(pmw->text_font)
    pango_layout_set_font_description(layout,pmw->text_font);
 else
    pango_layout_set_font_description(layout,gtk_style_context_get_font(gtk_widget_get_style_context(&pmw->generic.widget),GTK_STATE_FLAG_NORMAL));

 pango_layout_set_text(layout,"NNNNNNNNNNNNNNN",-1);
 pango_layout_get_pixel_size(layout,&width,&height);

 cairo_destroy(cr);

 *minimal_width=*natural_width=width;
}


/*++++++++++++++++++++++++++++++++++++++
  Return the preferred height of the widget

  GtkWidget *widget The widget whose height is requested.

  gint *minimal_height Returns the minimal height.

  gint *natural_height Returns the preferred height.
  ++++++++++++++++++++++++++++++++++++++*/

static void get_preferred_height(GtkWidget *widget,gint *minimal_height,gint *natural_height)
{
 ProcMeterText *pmw;
 PangoRectangle ink_rect;
 PangoLayout *layout;
 cairo_t *cr;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERTEXT(widget));
 g_return_if_fail(minimal_height!=NULL);
 g_return_if_fail(natural_height!=NULL);

 pmw=GTK_PROCMETERTEXT(widget);

 cr=gdk_cairo_create(gtk_widget_get_root_window(&pmw->generic.widget));

 layout=pango_cairo_create_layout(cr);
 if(pmw->text_font)
    pango_layout_set_font_description(layout,pmw->text_font);
 else
    pango_layout_set_font_description(layout,gtk_style_context_get_font(gtk_widget_get_style_context(&pmw->generic.widget),GTK_STATE_FLAG_NORMAL));

 pango_layout_set_text(layout,"AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789",-1);
 pango_layout_get_pixel_extents(layout,&ink_rect,NULL);

 cairo_destroy(cr);

 *minimal_height=*natural_height=(1+PANGO_ASCENT(ink_rect))+(1+PANGO_DESCENT(ink_rect))+2+pmw->generic.label_height;
}


/*++++++++++++++++++++++++++++++++++++++
  Change to the size that has been specified by the container.

  GtkWidget *widget The widget that has been resized.

  GtkAllocation *allocation The size information.
  ++++++++++++++++++++++++++++++++++++++*/

static void size_allocate(GtkWidget *widget,GtkAllocation *allocation)
{
 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERTEXT(widget));
 g_return_if_fail(allocation!=NULL);

 gtk_widget_set_allocation(widget,allocation);

 if(gtk_widget_get_realized(widget))
   {
    ProcMeterText *pmw=GTK_PROCMETERTEXT(widget);

    gdk_window_move_resize(gtk_widget_get_window(widget),
                           allocation->x,allocation->y,
                           allocation->width,allocation->height);

    TextResize(pmw);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Text Widget.

  gboolean draw Returns false

  GtkWidget *widget The Widget to redisplay.

  cairo_t *cr A cairo object describing the position to draw.
  ++++++++++++++++++++++++++++++++++++++*/

static gboolean draw(GtkWidget *widget,cairo_t *cr)
{
 ProcMeterText *pmw;

 g_return_val_if_fail(widget!=NULL,FALSE);
 g_return_val_if_fail(GTK_IS_PROCMETERTEXT(widget),FALSE);

 pmw=GTK_PROCMETERTEXT(widget);

 TextUpdate(pmw,TRUE);

 return(FALSE);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform all of the sizing on the Widget when it is created/resized.

  ProcMeterText *pmw The Widget to resize.
  ++++++++++++++++++++++++++++++++++++++*/

static void TextResize(ProcMeterText *pmw)
{
 cairo_t *cr;
 PangoRectangle ink_rect;
 PangoLayout *layout;
 int width, height;

 g_return_if_fail(pmw!=NULL);

 if(parent_class->resize)
    parent_class->resize(&pmw->generic);

 /* The text parts. */

 cr=gdk_cairo_create(gtk_widget_get_root_window(&pmw->generic.widget));

 layout=pango_cairo_create_layout(cr);
 if(pmw->text_font)
    pango_layout_set_font_description(layout,pmw->text_font);
 else
    pango_layout_set_font_description(layout,gtk_style_context_get_font(gtk_widget_get_style_context(&pmw->generic.widget),GTK_STATE_FLAG_NORMAL));

 pango_layout_set_text(layout,"AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789",-1);
 pango_layout_get_pixel_size(layout,&width,&height);
 pango_layout_get_pixel_extents(layout,&ink_rect,NULL);

 cairo_destroy(cr);

 pmw->text_x=(gtk_widget_get_allocated_width(&pmw->generic.widget)-width)/2;
 pmw->text_y=pmw->generic.body_start+1+(1+PANGO_ASCENT(ink_rect));
}


/*++++++++++++++++++++++++++++++++++++++
  Update the display.

  ProcMeterText *pmw The Widget to update.

  gboolean all Indicates if the whole widget is to be updated.
  ++++++++++++++++++++++++++++++++++++++*/

static void TextUpdate(ProcMeterText *pmw,gboolean all)
{
 g_return_if_fail(pmw!=NULL);

 if(gtk_widget_get_visible(&pmw->generic.widget))
   {
    int allocation_width;
    cairo_t *cr;
    PangoLayout *layout;

    cr=gdk_cairo_create(gtk_widget_get_window(&pmw->generic.widget));
    cairo_set_line_width(cr,1.0);

    allocation_width=gtk_widget_get_allocated_width(&pmw->generic.widget);

    if(all)
      {
       if(parent_class->update)
          parent_class->update(&pmw->generic);
      }
    else
      {
       cairo_set_source_rgb(cr,pmw->generic.body_bg_color.red,pmw->generic.body_bg_color.green,pmw->generic.body_bg_color.blue);

       cairo_rectangle(cr,0,pmw->generic.body_start,allocation_width,pmw->generic.body_height);
       cairo_fill(cr);
      }

    cairo_set_source_rgb(cr,pmw->generic.body_fg_color.red,pmw->generic.body_fg_color.green,pmw->generic.body_fg_color.blue);

    layout=pango_cairo_create_layout(cr);
    if(pmw->text_font)
       pango_layout_set_font_description(layout,pmw->text_font);
    else
       pango_layout_set_font_description(layout,gtk_style_context_get_font(gtk_widget_get_style_context(&pmw->generic.widget),GTK_STATE_FLAG_NORMAL));

    cairo_move_to(cr,pmw->text_x,pmw->text_y);
    pango_layout_set_text(layout,pmw->text_string,-1);
    pango_cairo_show_layout(cr,layout);

    cairo_destroy(cr);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Set the text font for the widget

  ProcMeterText *pmw The widget to set.

  PangoFontDescription *font The font to use.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterTextSetFont(ProcMeterText *pmw,PangoFontDescription *font)
{
 pmw->text_font=font;

 TextResize(pmw);

 TextUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Change the data displayed in the ProcMeter Text Widget.

  ProcMeterText *pmw The ProcMeter Text Widget.

  char *text The new string to display.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterTextChangeData(ProcMeterText *pmw,char *text)
{
 cairo_t *cr;
 PangoLayout *layout;
 int width, height;

 if(pmw->text_string!=empty_string)
    free(pmw->text_string);
 pmw->text_string=(char*)malloc(strlen(text)+1);
 strcpy(pmw->text_string,text);

 cr=gdk_cairo_create(gtk_widget_get_root_window(&pmw->generic.widget));

 layout=pango_cairo_create_layout(cr);
 if(pmw->text_font)
    pango_layout_set_font_description(layout,pmw->text_font);
 else
    pango_layout_set_font_description(layout,gtk_style_context_get_font(gtk_widget_get_style_context(&pmw->generic.widget),GTK_STATE_FLAG_NORMAL));

 pango_layout_set_text(layout,pmw->text_string,-1);
 pango_layout_get_pixel_size(layout,&width,&height);

 cairo_destroy(cr);

 pmw->text_x=(gtk_widget_get_allocated_width(&pmw->generic.widget)-width)/2;

 TextUpdate(pmw,FALSE);
}
