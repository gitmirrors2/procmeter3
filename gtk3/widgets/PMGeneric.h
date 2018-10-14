/***************************************
  ProcMeter Generic Widget include file (for ProcMeter 3.6).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996-2012, 2018 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMGENERIC_H
#define PMGENERIC_H    /*+ To stop multiple inclusions. +*/

#define GDK_DISABLE_DEPRECATION_WARNINGS 1
#include <gdk/gdk.h>
#include <gtk/gtk.h>


#define GTK_TYPE_PROCMETERGENERIC          (gtk_procmetergeneric_get_type())
#define GTK_PROCMETERGENERIC(obj)          G_TYPE_CHECK_INSTANCE_CAST((obj),GTK_TYPE_PROCMETERGENERIC,ProcMeterGeneric)
#define GTK_PROCMETERGENERIC_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST((klass),GTK_TYPE_PROCMETERGENERIC,ProcMeterGenericClass)
#define GTK_IS_PROCMETERGENERIC(obj)       G_TYPE_CHECK_INSTANCE_TYPE((obj),GTK_TYPE_PROCMETERGENERIC)


typedef struct _ProcMeterGeneric       ProcMeterGeneric;
typedef struct _ProcMeterGenericClass  ProcMeterGenericClass;


struct _ProcMeterGeneric
{
 GtkWidget       widget;

 GdkRGBA         body_bg_color;    /*+ The body background colour. +*/
 GdkRGBA         body_fg_color;    /*+ The body foreground colour. +*/
 gushort         body_height;      /*+ The height of the body part. +*/
 gushort         body_start;       /*+ The start position of the body part. +*/

 gchar*          label_string;     /*+ The label for the Widget. +*/
 GdkRGBA         label_color;      /*+ The label colour. +*/
 gint            label_pos;        /*+ The position of the label. +*/
 PangoFontDescription* label_font; /*+ The font for the label. +*/
 gushort         label_height;     /*+ The height of the label. +*/
 gshort          label_x,label_y;  /*+ The position of the label. +*/
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

void ProcMeterGenericSetForegroundColour(ProcMeterGeneric *pmw,GdkRGBA *body_fg_color);
void ProcMeterGenericSetBackgroundColour(ProcMeterGeneric *pmw,GdkRGBA *body_bg_color);
void ProcMeterGenericSetLabelColour(ProcMeterGeneric *pmw,GdkRGBA *label_color);

void ProcMeterGenericSetLabelPosition(ProcMeterGeneric *pmw,int label_position);
void ProcMeterGenericSetLabelFont(ProcMeterGeneric *pmw,PangoFontDescription *font);
void ProcMeterGenericSetLabel(ProcMeterGeneric *pmw,gchar *label);

#endif /* PMGENERIC_H */
