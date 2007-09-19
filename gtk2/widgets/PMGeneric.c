/***************************************
  $Header: /home/amb/CVS/procmeter3/gtk2/widgets/PMGeneric.c,v 1.1 2007-09-19 19:04:49 amb Exp $

  ProcMeter Generic Widget Source file (for ProcMeter 3.4a).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,2000,10,02,03 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdlib.h>
#include <string.h>

#include "PMGeneric.h"

static void procmetergeneric_class_init(ProcMeterGenericClass *class);
static void procmetergeneric_init(ProcMeterGeneric *pmw);
static void destroy(GtkObject *object);
static void realize(GtkWidget *widget);
static gint expose(GtkWidget *widget,GdkEventExpose *event);
static void size_request(GtkWidget *widget,GtkRequisition *requisition);
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
    GtkTypeInfo pmw_info={"ProcMeterGeneric",
                          sizeof(ProcMeterGeneric),
                          sizeof(ProcMeterGenericClass),
                          (GtkClassInitFunc)procmetergeneric_class_init,
                          (GtkObjectInitFunc)procmetergeneric_init,
                          (gpointer) NULL,
                          (gpointer) NULL,
                          (GtkClassInitFunc) NULL};

    pmw_type=gtk_type_unique(gtk_widget_get_type(),&pmw_info);
   }

 return(pmw_type);
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise the Widget class

  ProcMeterGenericClass *class The class of widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmetergeneric_class_init(ProcMeterGenericClass *class)
{
 GtkObjectClass *object_class;
 GtkWidgetClass *widget_class;

 g_return_if_fail(class!=NULL);

 object_class=(GtkObjectClass*)class;
 widget_class=(GtkWidgetClass*)class;

 class->resize=GenericResize;
 class->update=GenericUpdate;

 parent_class=gtk_type_class(gtk_widget_get_type());

 object_class->destroy=destroy;

 widget_class->realize=realize;
 widget_class->expose_event=expose;
 widget_class->size_request=size_request;
 widget_class->size_allocate=size_allocate;
}


/*++++++++++++++++++++++++++++++++++++++
  Create a new Widget.

  GtkWidget* gtk_procmetergeneric_new Returns the new widgets.
  ++++++++++++++++++++++++++++++++++++++*/

GtkWidget* gtk_procmetergeneric_new(void)
{
 ProcMeterGeneric *new;

 new=gtk_type_new(gtk_procmetergeneric_get_type());

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

 pmw->body_gc=NULL;

 pmw->body_bg_set=FALSE;

 /* The label parts. */

 pmw->label_pos=ProcMeterLabelBottom;

 pmw->label_string=empty_string;

 pmw->label_font=NULL;

 pmw->label_gc=NULL;

 /* The rest of the sizing. */

 GenericResize(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a Widget

  GtkObject *object The widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void destroy(GtkObject *object)
{
 ProcMeterGeneric *pmw;

 g_return_if_fail(object!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGENERIC(object));

 pmw=GTK_PROCMETERGENERIC(object);

 if(pmw->label_string!=empty_string)
    free(pmw->label_string);
 if(pmw->body_gc)
    gdk_gc_destroy(pmw->body_gc);
 if(pmw->label_gc)
    gdk_gc_destroy(pmw->label_gc);

 if(GTK_OBJECT_CLASS(parent_class)->destroy)
    (*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}


/*++++++++++++++++++++++++++++++++++++++
  Realize a widget.

  GtkWidget *widget The widget to realize.
  ++++++++++++++++++++++++++++++++++++++*/

static void realize(GtkWidget *widget)
{
 ProcMeterGeneric *pmw;
 GdkWindowAttr attributes;
 gint attributes_mask;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGENERIC(widget));

 GTK_WIDGET_SET_FLAGS(widget,GTK_REALIZED);
 pmw=GTK_PROCMETERGENERIC(widget);

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

 if(pmw->body_bg_set)
    gdk_window_set_background(widget->window,&pmw->body_bg_color);

 GenericUpdate(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Choose the size that the widget wants to be.

  GtkWidget *widget The widget to be resized.

  GtkRequisition *requisition Returns the request for the size.
  ++++++++++++++++++++++++++++++++++++++*/

static void size_request(GtkWidget *widget,GtkRequisition *requisition)
{
 ProcMeterGeneric *pmw;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERGENERIC(widget));
 g_return_if_fail(requisition!=NULL);

 pmw=GTK_PROCMETERGENERIC(widget);

 requisition->height=10+pmw->label_height;
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
 g_return_if_fail(GTK_IS_PROCMETERGENERIC(widget));
 g_return_if_fail(allocation!=NULL);

 widget->allocation=*allocation;
 if(GTK_WIDGET_REALIZED(widget))
   {
    ProcMeterGeneric *pmw=GTK_PROCMETERGENERIC(widget);

    gdk_window_move_resize(widget->window,
                           allocation->x,allocation->y,
                           allocation->width,allocation->height);

    GenericResize(pmw);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Generic Widget.

  gint expose Returns false

  GtkWidget *widget The Widget to redisplay.

  GdkEventExpose *event The event that caused the redisplay.
  ++++++++++++++++++++++++++++++++++++++*/

static gint expose(GtkWidget *widget,GdkEventExpose *event)
{
 ProcMeterGeneric *pmw;

 g_return_val_if_fail(widget!=NULL,FALSE);
 g_return_val_if_fail(GTK_IS_PROCMETERGENERIC(widget),FALSE);
 g_return_val_if_fail(event!=NULL,FALSE);

 if(event->count>0)
    return(FALSE);

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
 g_return_if_fail(pmw!=NULL);

 /* The label parts. */

 if(pmw->label_pos)
   {
    GdkFont *label_font=pmw->label_font?pmw->label_font:gtk_style_get_font(pmw->widget.style);

    pmw->label_height=label_font->ascent+label_font->descent+2;
    pmw->label_x=(pmw->widget.allocation.width-gdk_string_width(label_font,pmw->label_string))/2;
    if(pmw->label_pos==ProcMeterLabelTop)
       pmw->label_y=pmw->label_height-1-label_font->descent;
    else
       pmw->label_y=pmw->widget.allocation.height-label_font->descent;
   }
 else
   {
    pmw->label_height=0;
    pmw->label_x=0;
    pmw->label_y=0;
   }

 /* The body parts. */

 pmw->body_height=pmw->widget.allocation.height-pmw->label_height;

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

 if(GTK_WIDGET_VISIBLE(&pmw->widget))
   {
    GdkFont *label_font=pmw->label_font?pmw->label_font:gtk_style_get_font(pmw->widget.style);
    GdkGC *label_gc=pmw->label_gc?pmw->label_gc:pmw->widget.style->fg_gc[GTK_STATE_NORMAL];

    gdk_window_clear(pmw->widget.window);

    if(pmw->label_pos)
      {
       gdk_draw_string(pmw->widget.window,label_font,label_gc,
                       pmw->label_x,pmw->label_y,
                       pmw->label_string);

       if(pmw->label_pos==ProcMeterLabelTop)
          gdk_draw_line(pmw->widget.window,label_gc,
                        0                           ,pmw->label_height-1,
                        pmw->widget.allocation.width,pmw->label_height-1);
       else
          gdk_draw_line(pmw->widget.window,label_gc,
                        0                           ,pmw->body_height,
                        pmw->widget.allocation.width,pmw->body_height);
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Set the body colours of the Widget.

  ProcMeterGeneric *pmw The widget to set.

  GdkColor body_bg_color The body background.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGenericSetBackgroundColour(ProcMeterGeneric *pmw,GdkColor body_bg_color)
{
 pmw->body_bg_color=body_bg_color;

 pmw->body_bg_set=TRUE;

 if(pmw->widget.window)
    gdk_window_set_background(pmw->widget.window,&pmw->body_bg_color);

 if(pmw->body_gc)
    gdk_gc_set_background(pmw->body_gc,&pmw->body_bg_color);
 else
   {
    GdkGCValues values;

    values.background=pmw->body_bg_color;
    pmw->body_gc=gdk_gc_new_with_values(pmw->widget.parent->window,&values,GDK_GC_BACKGROUND);
   }

 if(pmw->label_gc)
    gdk_gc_set_background(pmw->label_gc,&pmw->body_bg_color);
 else
   {
    GdkGCValues values;

    values.background=pmw->body_bg_color;
    pmw->label_gc=gdk_gc_new_with_values(pmw->widget.parent->window,&values,GDK_GC_BACKGROUND);
   }

 GenericUpdate(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the body colours of the Widget.

  ProcMeterGeneric *pmw The widget to set.

  GdkColor body_fg_color The body foreground.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGenericSetForegroundColour(ProcMeterGeneric *pmw,GdkColor body_fg_color)
{
 pmw->body_fg_color=body_fg_color;

 if(pmw->body_gc)
    gdk_gc_set_foreground(pmw->body_gc,&pmw->body_fg_color);
 else
   {
    GdkGCValues values;

    values.foreground=pmw->body_fg_color;
    pmw->body_gc=gdk_gc_new_with_values(pmw->widget.parent->window,&values,GDK_GC_FOREGROUND);
   }

 GenericUpdate(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the label colour of the Widget.

  ProcMeterGeneric *pmw The widget to set.

  GdkColor label_color The label foreground.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGenericSetLabelColour(ProcMeterGeneric *pmw,GdkColor label_color)
{
 pmw->label_color=label_color;

 if(pmw->label_gc)
    gdk_gc_set_foreground(pmw->label_gc,&pmw->label_color);
 else
   {
    GdkGCValues values;

    values.foreground=pmw->label_color;
    pmw->label_gc=gdk_gc_new_with_values(pmw->widget.parent->window,&values,GDK_GC_FOREGROUND);
   }

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

  GdkFont *font The font to use.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterGenericSetLabelFont(ProcMeterGeneric *pmw,GdkFont *font)
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
