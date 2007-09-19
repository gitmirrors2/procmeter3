/***************************************
  $Header: /home/amb/CVS/procmeter3/gtk2/widgets/PMGeneric.h,v 1.1 2007-09-19 19:04:59 amb Exp $

  ProcMeter Generic Widget include file (for ProcMeter 3.3).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,2000 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMGENERIC_H
#define PMGENERIC_H    /*+ To stop multiple inclusions. +*/

#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkwidget.h>


#define GTK_TYPE_PROCMETERGENERIC          (gtk_procmetergeneric_get_type())
#define GTK_PROCMETERGENERIC(obj)          GTK_CHECK_CAST((obj),GTK_TYPE_PROCMETERGENERIC,ProcMeterGeneric)
#define GTK_PROCMETERGENERIC_CLASS(klass)  GTK_CHECK_CLASS_CAST((klass),GTK_TYPE_PROCMETERGENERIC,ProcMeterGenericClass)
#define GTK_IS_PROCMETERGENERIC(obj)       GTK_CHECK_TYPE((obj),GTK_TYPE_PROCMETERGENERIC)


typedef struct _ProcMeterGeneric       ProcMeterGeneric;
typedef struct _ProcMeterGenericClass  ProcMeterGenericClass;


struct _ProcMeterGeneric
{
 GtkWidget       widget;

 GdkColor        body_bg_color;    /*+ The body background colour. +*/
 gboolean        body_bg_set;      /*+ A flag to indicate if the background has been set. +*/
 GdkColor        body_fg_color;    /*+ The body foreground colour. +*/
 GdkGC*          body_gc;          /*+ The graphics context for the body. +*/
 gushort         body_height;      /*+ The height of the body part. +*/
 gushort         body_start;       /*+ The start position of the body part. +*/

 gchar*          label_string;     /*+ The label for the Widget. +*/
 GdkColor        label_color;      /*+ The label colour. +*/
 GdkGC*          label_gc;         /*+ The graphics context for the label. +*/
 gint            label_pos;        /*+ The position of the label. +*/
 GdkFont*        label_font;       /*+ The font for the label. +*/
 gushort         label_height;     /*+ The height of the label. +*/
 gushort         label_x,label_y;  /*+ The position of the label. +*/
};

struct _ProcMeterGenericClass
{
 GtkWidgetClass parent_class;

 void (*resize)(ProcMeterGeneric *pmw);
 void (*update)(ProcMeterGeneric *pmw);
};

guint      gtk_procmetergeneric_get_type(void);
GtkWidget* gtk_procmetergeneric_new(void);


/* The options for label placement */

#define ProcMeterLabelTop     1
#define ProcMeterLabelNone    0
#define ProcMeterLabelBottom -1


/* Public functions */

void ProcMeterGenericSetForegroundColour(ProcMeterGeneric *pmw,GdkColor body_fg_color);
void ProcMeterGenericSetBackgroundColour(ProcMeterGeneric *pmw,GdkColor body_bg_color);
void ProcMeterGenericSetLabelColour(ProcMeterGeneric *pmw,GdkColor label_color);

void ProcMeterGenericSetLabelPosition(ProcMeterGeneric *pmw,int label_position);
void ProcMeterGenericSetLabelFont(ProcMeterGeneric *pmw,GdkFont *font);
void ProcMeterGenericSetLabel(ProcMeterGeneric *pmw,gchar *label);

#endif /* PMGENERIC_H */
