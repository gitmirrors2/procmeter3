/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/resources.c,v 1.2 1999-08-31 18:22:33 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.2.

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

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#include "xwindow.h"
#include "procmeterp.h"


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to a font structure.

  XFontStruct *StringToFont Returns a pointer to a font structure.

  char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

XFontStruct *StringToFont(char *string)
{
 return(XLoadQueryFont(display,string));
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to a pixel.

  Pixel StringToPixel Returns a Pixel value.

  char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

Pixel StringToPixel(char *string)
{
 XColor screenColor;
 XColor exactColor;

 XAllocNamedColor(display, DefaultColormap(display,DefaultScreen(display)), string, &screenColor, &exactColor);

 return(screenColor.pixel);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to a Boolean.

  Boolean StringToBoolean Returns a Boolean value.

  char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

Boolean StringToBoolean(char *string)
{
 Boolean b;

 if(!strcasecmp(string,"true") ||
    !strcasecmp(string,"yes") ||
    !strcasecmp(string,"1"))
    b=True;
 else
    b=False;

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
