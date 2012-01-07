/***************************************
  ProcMeter Text Widget include file (for ProcMeter 3.6).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996-2012 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMTEXT_H
#define PMTEXT_H    /*+ To stop multiple inclusions. +*/

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "PMGeneric.h"

#define GTK_TYPE_PROCMETERTEXT          (gtk_procmetertext_get_type())
#define GTK_PROCMETERTEXT(obj)          G_TYPE_CHECK_INSTANCE_CAST((obj),GTK_TYPE_PROCMETERTEXT,ProcMeterText)
#define GTK_PROCMETERTEXT_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST((klass),GTK_TYPE_PROCMETERTEXT,ProcMeterTextClass)
#define GTK_IS_PROCMETERTEXT(obj)       G_TYPE_CHECK_INSTANCE_TYPE((obj),GTK_TYPE_PROCMETERTEXT)


typedef struct _ProcMeterText       ProcMeterText;
typedef struct _ProcMeterTextClass  ProcMeterTextClass;


struct _ProcMeterText
{
 ProcMeterGeneric generic;

 gchar*           text_string;     /*+ The text for the Widget. +*/
 PangoFontDescription* text_font;  /*+ The font for the text. +*/
 gshort           text_x,text_y;   /*+ The position of the text. +*/
};

struct _ProcMeterTextClass
{
 ProcMeterGenericClass parent_class;

 void (*resize)(ProcMeterText *pmw);
 void (*update)(ProcMeterText *pmw,gboolean all);
};

guint      gtk_procmetertext_get_type(void);
GtkWidget* gtk_procmetertext_new(void);


/* Public functions */

void ProcMeterTextSetFont(ProcMeterText *pmw,PangoFontDescription *font);

void ProcMeterTextChangeData(ProcMeterText *pmt,char *data);

#endif /* PMTEXT_H */
