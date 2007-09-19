/***************************************
  $Header: /home/amb/CVS/procmeter3/gtk2/widgets/PMGraph.h,v 1.1 2007-09-19 19:05:17 amb Exp $

  ProcMeter Graph Widget include file (for ProcMeter3 3.3).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1996,98,99,2000 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PMGRAPH_H
#define PMGRAPH_H    /*+ To stop multiple inclusions. +*/

#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>

#include "PMGeneric.h"

#define GTK_TYPE_PROCMETERGRAPH          (gtk_procmetergraph_get_type())
#define GTK_PROCMETERGRAPH(obj)          GTK_CHECK_CAST((obj),GTK_TYPE_PROCMETERGRAPH,ProcMeterGraph)
#define GTK_PROCMETERGRAPH_CLASS(klass)  GTK_CHECK_CLASS_CAST((klass),GTK_TYPE_PROCMETERGRAPH,ProcMeterGraphClass)
#define GTK_IS_PROCMETERGRAPH(obj)       GTK_CHECK_TYPE((obj),GTK_TYPE_PROCMETERGRAPH)


typedef struct _ProcMeterGraph       ProcMeterGraph;
typedef struct _ProcMeterGraphClass  ProcMeterGraphClass;


struct _ProcMeterGraph
{
 ProcMeterGeneric generic;

 gchar*           grid_units;       /*+ The number of things per grid line. +*/
 gushort          grid_units_x;     /*+ The position of the grid units. +*/
 GdkColor         grid_color;       /*+ The grid lines colour. +*/
 GdkGC*           grid_gc;          /*+ The graphics context for the grid lines. +*/
 gint             grid_min;         /*+ The minimum number of grid lines. +*/
 gint             grid_max;         /*+ The maximum number of grid lines. +*/
 gint             grid_maxvis;      /*+ The maximum number of grid lines before removing them. +*/
 gint             grid_drawn;       /*+ If 1 then draw as normal, if 0 never draw, if -1 draw only one line. +*/
 gint             grid_num;         /*+ The actual number of grid lines. +*/

 gboolean         line_solid;       /*+ True if the area under the graph is to be filled. +*/

 gushort*         data;             /*+ The data for the graph. +*/
 gushort          data_max;         /*+ The maximum data value. +*/
 guint            data_num;         /*+ The number of data points. +*/
 gint             data_index;       /*+ An index into the array for the new value. +*/
};

struct _ProcMeterGraphClass
{
 ProcMeterGenericClass parent_class;

 void (*resize)(ProcMeterGraph *pmw);
 void (*update)(ProcMeterGraph *pmw,gboolean all);
};

guint      gtk_procmetergraph_get_type(void);
GtkWidget* gtk_procmetergraph_new(void);


/* Public functions */

void ProcMeterGraphSetGridColour(ProcMeterGraph *pmw,GdkColor grid_color);
void ProcMeterGraphSetGridMin(ProcMeterGraph *pmw,gint grid_min);
void ProcMeterGraphSetGridMax(ProcMeterGraph *pmw,gint grid_max);
void ProcMeterGraphSetGridUnits(ProcMeterGraph *pmw,gchar *units);
void ProcMeterGraphSetSolid(ProcMeterGraph *pmw,gboolean solid);

void ProcMeterGraphAddDatum(ProcMeterGraph *pmw,gushort datum);

#endif /* PMGRAPH_H */
