/***************************************
  ProcMeter Bar Widget include file (for ProcMeter3 3.6).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996-2012 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/

#ifndef PMBAR_H
#define PMBAR_H    /*+ To stop multiple inclusions. +*/

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "PMGeneric.h"

#define GTK_TYPE_PROCMETERBAR          (gtk_procmeterbar_get_type())
#define GTK_PROCMETERBAR(obj)          G_TYPE_CHECK_INSTANCE_CAST((obj),GTK_TYPE_PROCMETERBAR,ProcMeterBar)
#define GTK_PROCMETERBAR_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST((klass),GTK_TYPE_PROCMETERBAR,ProcMeterBarClass)
#define GTK_IS_PROCMETERBAR(obj)       G_TYPE_CHECK_INSTANCE_TYPE((obj),GTK_TYPE_PROCMETERBAR)


typedef struct _ProcMeterBar       ProcMeterBar;
typedef struct _ProcMeterBarClass  ProcMeterBarClass;

struct _ProcMeterBar
{
 ProcMeterGeneric generic;

 gchar*           grid_units;       /*+ The number of things per grid line. +*/
 gushort          grid_units_x;     /*+ The position of the grid units. +*/
 GdkRGBA          grid_color;       /*+ The grid lines colour. +*/
 gint             grid_min;         /*+ The minimum number of grid lines. +*/
 gint             grid_max;         /*+ The maximum number of grid lines. +*/
 gint             grid_maxvis;      /*+ The maximum number of grid lines before removing them. +*/
 gint             grid_drawn;       /*+ If 1 then draw as normal, if 0 never draw, if -1 draw only one line. +*/
 gint             grid_num;         /*+ The actual number of grid lines. +*/

 gushort          data[8];          /*+ The data for the bar. +*/
 gushort          data_index;       /*+ A pointer into the array +*/
 gulong           data_sum;         /*+ The average value of the last 10 data points. +*/
};

struct _ProcMeterBarClass
{
 ProcMeterGenericClass parent_class;

 void (*resize)(ProcMeterBar *pmw);
 void (*update)(ProcMeterBar *pmw,gboolean all);
};

guint      gtk_procmeterbar_get_type(void);
GtkWidget* gtk_procmeterbar_new(void);


/* Public functions */

void ProcMeterBarSetGridColour(ProcMeterBar *pmw,GdkRGBA *grid_color);
void ProcMeterBarSetGridMin(ProcMeterBar *pmw,gint grid_min);
void ProcMeterBarSetGridMax(ProcMeterBar *pmw,gint grid_max);
void ProcMeterBarSetGridUnits(ProcMeterBar *pmw,gchar *units);

void ProcMeterBarAddDatum(ProcMeterBar *pmw,gushort datum);

#endif /* PMBAR_H */
