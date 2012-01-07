/***************************************
  ProcMeter - A system monitoring program for Linux - Version 3.6.

  X Window resource conversions.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998-2011 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "window.h"
#include "procmeterp.h"


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to a font structure.

  PangoFontDescription *StringToFont Returns a pointer to a font structure.

  char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

PangoFontDescription *StringToFont(char *string)
{
 return(pango_font_description_from_string(string));
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to a colour.

  GdkRGBA *StringToColour Returns a GdkRGBA value.

  char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

GdkRGBA *StringToColour(char *string)
{
 static GdkRGBA rgba;

 gdk_rgba_parse(&rgba,string);

 return(&rgba);
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
