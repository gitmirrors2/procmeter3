/***************************************
  $Header: /home/amb/CVS/procmeter3/gtk1/widgets/PMText.c,v 1.1 2000-12-16 16:39:11 amb Exp $

  ProcMeter Text Widget Source file (for ProcMeter 3.3).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,2000 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdlib.h>

#include "PMGeneric.h"
#include "PMText.h"

static void procmetertext_class_init (ProcMeterTextClass *class);
static void procmetertext_init(ProcMeterText *pmw);
static void destroy(GtkObject *object);
static void realize(GtkWidget *widget);
static gint expose(GtkWidget *widget,GdkEventExpose *event);
static void size_request(GtkWidget *widget,GtkRequisition *requisition);
static void size_allocate(GtkWidget *widget,GtkAllocation *allocation);

static void ProcMeterTextResize(ProcMeterText *pmw);
static void ProcMeterTextUpdate(ProcMeterText *pmw,gboolean all);

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
    GtkTypeInfo pmw_info={"ProcMeterText",
                          sizeof(ProcMeterText),
                          sizeof(ProcMeterTextClass),
                          (GtkClassInitFunc) procmetertext_class_init,
                          (GtkObjectInitFunc) procmetertext_init,
                          (GtkArgSetFunc) NULL,
                          (GtkArgGetFunc) NULL};

    pmw_type=gtk_type_unique(gtk_procmetergeneric_get_type(),&pmw_info);
   }

 return(pmw_type);
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise the Widget class

  ProcMeterTextClass *class The class of widget to initialise.
  ++++++++++++++++++++++++++++++++++++++*/

static void procmetertext_class_init(ProcMeterTextClass *class)
{
 GtkObjectClass *object_class;
 GtkWidgetClass *widget_class;

 g_return_if_fail(class!=NULL);

 object_class=(GtkObjectClass*)class;
 widget_class=(GtkWidgetClass*)class;

 class->resize=ProcMeterTextResize;
 class->update=ProcMeterTextUpdate;

 parent_class=gtk_type_class(gtk_procmetergeneric_get_type());

 object_class->destroy=destroy;

 widget_class->realize=realize;
 widget_class->expose_event=expose;
 widget_class->size_request=size_request;
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

 ProcMeterTextResize(pmw);
}


/*++++++++++++++++++++++++++++++++++++++
  Create a new Widget.

  GtkWidget* gtk_procmetertext_new Returns the new widgets.
  ++++++++++++++++++++++++++++++++++++++*/

GtkWidget* gtk_procmetertext_new(void)
{
 ProcMeterText *pmw;

 pmw=gtk_type_new(gtk_procmetertext_get_type());

 return(GTK_WIDGET(pmw));
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy a Widget

  GtkObject *object The widget to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

static void destroy(GtkObject *object)
{
 ProcMeterText *pmw;

 g_return_if_fail(object!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERTEXT(object));

 pmw=GTK_PROCMETERTEXT(object);

 if(pmw->text_string!=empty_string)
    free(pmw->text_string);

 if(GTK_OBJECT_CLASS(parent_class)->destroy)
    (*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}


/*++++++++++++++++++++++++++++++++++++++
  Realize a widget.

  GtkWidget *widget The widget to realize.
  ++++++++++++++++++++++++++++++++++++++*/

static void realize(GtkWidget *widget)
{
 ProcMeterText *pmw;
 GdkWindowAttr attributes;
 gint attributes_mask;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERTEXT(widget));

 GTK_WIDGET_SET_FLAGS(widget,GTK_REALIZED);
 pmw=GTK_PROCMETERTEXT(widget);

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

 ProcMeterTextUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Choose the size that the widget wants to be.

  GtkWidget *widget The widget to be resized.

  GtkRequisition *requisition Returns the request for the size.
  ++++++++++++++++++++++++++++++++++++++*/

static void size_request(GtkWidget *widget,GtkRequisition *requisition)
{
 ProcMeterText *pmw;
 GdkFont *text_font;

 g_return_if_fail(widget!=NULL);
 g_return_if_fail(GTK_IS_PROCMETERTEXT(widget));
 g_return_if_fail(requisition!=NULL);

 pmw=GTK_PROCMETERTEXT(widget);
 text_font=pmw->text_font?pmw->text_font:pmw->generic.widget.style->font;

 requisition->width=gdk_string_width(text_font,"NNNNNNNNNNNNNNN");
 requisition->height=text_font->ascent+text_font->descent+2+pmw->generic.label_height;
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

 widget->allocation=*allocation;
 if(GTK_WIDGET_REALIZED(widget))
   {
    ProcMeterText *pmw=GTK_PROCMETERTEXT(widget);

    gdk_window_move_resize(widget->window,
                           allocation->x,allocation->y,
                           allocation->width,allocation->height);

    ProcMeterTextResize(pmw);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Redisplay the ProcMeter Text Widget.

  gint expose Returns false

  GtkWidget *widget The Widget to redisplay.

  GdkEventExpose *event The event that caused the redisplay.
  ++++++++++++++++++++++++++++++++++++++*/

static gint expose(GtkWidget *widget,GdkEventExpose *event)
{
 ProcMeterText *pmw;

 g_return_val_if_fail(widget!=NULL,FALSE);
 g_return_val_if_fail(GTK_IS_PROCMETERTEXT(widget),FALSE);
 g_return_val_if_fail(event!=NULL,FALSE);

 if(event->count>0)
    return(FALSE);

 pmw=GTK_PROCMETERTEXT(widget);

 ProcMeterTextUpdate(pmw,TRUE);

 return(FALSE);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform all of the sizing on the Widget when it is created/resized.

  ProcMeterText *pmw The Widget to resize.
  ++++++++++++++++++++++++++++++++++++++*/

static void ProcMeterTextResize(ProcMeterText *pmw)
{
 GdkFont *text_font;

 g_return_if_fail(pmw!=NULL);

 (parent_class->resize)(&pmw->generic);

 /* The text parts. */

 text_font=pmw->text_font?pmw->text_font:pmw->generic.widget.style->font;

 pmw->text_x=(pmw->generic.widget.allocation.width-gdk_string_width(text_font,pmw->text_string))/2;
 pmw->text_y=pmw->generic.body_start+1+text_font->ascent;
}


/*++++++++++++++++++++++++++++++++++++++
  Update the display.

  ProcMeterText *pmw The Widget to update.

  gboolean all Indicates if the whole widget is to be updated.
  ++++++++++++++++++++++++++++++++++++++*/

static void ProcMeterTextUpdate(ProcMeterText *pmw,gboolean all)
{
 g_return_if_fail(pmw!=NULL);

 if(GTK_WIDGET_VISIBLE(&pmw->generic.widget))
   {
    GdkGC *body_gc=pmw->generic.body_gc?pmw->generic.body_gc:pmw->generic.widget.style->fg_gc[GTK_STATE_NORMAL];
    GdkFont *text_font=pmw->text_font?pmw->text_font:pmw->generic.widget.style->font;

    if(all)
       (parent_class->update)(&pmw->generic);
    else
       gdk_window_clear_area(pmw->generic.widget.window,
                             0,pmw->generic.body_start,
                             pmw->generic.widget.allocation.width,pmw->generic.body_height);

    gdk_draw_string(pmw->generic.widget.window,text_font,body_gc,
                    pmw->text_x,pmw->text_y,
                    pmw->text_string);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Set the text font for the widget

  ProcMeterText *pmw The widget to set.

  GdkFont *font The font to use.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterTextSetFont(ProcMeterText *pmw,GdkFont *font)
{
 pmw->text_font=font;

 ProcMeterTextResize(pmw);

 ProcMeterTextUpdate(pmw,TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Change the data displayed in the ProcMeter Text Widget.

  ProcMeterText *pmw The ProcMeter Text Widget.

  char *text The new string to display.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcMeterTextChangeData(ProcMeterText *pmw,char *text)
{
 if(pmw->text_string!=empty_string)
    free(pmw->text_string);
 pmw->text_string=(char*)malloc(strlen(text)+1);
 strcpy(pmw->text_string,text);

 pmw->text_x=(pmw->generic.widget.allocation.width-gdk_string_width(pmw->text_font,pmw->text_string))/2;

 ProcMeterTextUpdate(pmw,FALSE);
}
