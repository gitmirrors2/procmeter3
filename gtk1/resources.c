/***************************************
  $Header: /home/amb/CVS/procmeter3/gtk1/resources.c,v 1.1 2000-12-16 16:35:43 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3.

  X Window resource conversions.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#include "window.h"
#include "procmeterp.h"


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to a font structure.

  GdkFont *StringToFont Returns a pointer to a font structure.

  char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

GdkFont *StringToFont(char *string)
{
 return(gdk_font_load(string));
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to a pixel.

  GdkColor StringToPixel Returns a GdkColor value.

  char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

GdkColor StringToPixel(char *string)
{
 static GdkColor color;

 gdk_color_parse(string,&color);
 gdk_colormap_alloc_color(gdk_colormap_get_system(),&color,FALSE,TRUE);

 return(color);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to a Boolean.

  gboolean StringToBoolean Returns a Boolean value.

  char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

gboolean StringToBoolean(char *string)
{
 gboolean b;

 if(!strcasecmp(string,"true") ||
    !strcasecmp(string,"yes") ||
    !strcasecmp(string,"1"))
    b=TRUE;
 else
    b=FALSE;

 return(b);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to an integer

  int StringToInt Returns an integer value.

  char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

int StringToInt(char *string)
{
 int i;

 if(sscanf(string,"%d",&i)!=1)
    i=0;

 return(i);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to a label position.

  int StringToLabelPosition Returns an integer value.

  char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

int StringToLabelPosition(char *string)
{
 int i;

 if(!strcasecmp(string,"top"))
    i=1;
 else if(!strcasecmp(string,"bottom"))
    i=-1;
 else
    i=0;

 return(i);
}
