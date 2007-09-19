/***************************************
  $Header: /home/amb/CVS/procmeter3/gtk2/widgets/PMText.h,v 1.1 2007-09-19 19:05:37 amb Exp $

  ProcMeter Text Widget include file (for ProcMeter 3.3).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,2000 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMTEXT_H
#define PMTEXT_H    /*+ To stop multiple inclusions. +*/

#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>

#include "PMGeneric.h"

#define GTK_TYPE_PROCMETERTEXT          (gtk_procmetertext_get_type())
#define GTK_PROCMETERTEXT(obj)          GTK_CHECK_CAST((obj),GTK_TYPE_PROCMETERTEXT,ProcMeterText)
#define GTK_PROCMETERTEXT_CLASS(klass)  GTK_CHECK_CLASS_CAST((klass),GTK_TYPE_PROCMETERTEXT,ProcMeterTextClass)
#define GTK_IS_PROCMETERTEXT(obj)       GTK_CHECK_TYPE((obj),GTK_TYPE_PROCMETERTEXT)


typedef struct _ProcMeterText       ProcMeterText;
typedef struct _ProcMeterTextClass  ProcMeterTextClass;


struct _ProcMeterText
{
 ProcMeterGeneric generic;

 gchar*           text_string;     /*+ The text for the Widget. +*/
 GdkFont*         text_font;       /*+ The font for the text. +*/
 gushort          text_x,text_y;   /*+ The position of the text. +*/
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

void ProcMeterTextSetFont(ProcMeterText *pmw,GdkFont *font);

void ProcMeterTextChangeData(ProcMeterText *pmt,char *data);

#endif /* PMTEXT_H */
