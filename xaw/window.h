/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/window.h,v 1.1 1998-09-19 15:22:23 amb Exp $

  ProcMeter - A system monitoring program for Linux.

  Global X windows header file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef XWINDOW_H
#define XWINDOW_H    /*+ To stop multiple inclusions. +*/

#include "procmeterp.h"

/* In xwindow.c */

extern Display *display;
extern XtAppContext app_context;

/* In xmenus.c */

void CreateMenus(Widget parent);
void DestroyMenus(void);
void AddMenuToOutput(Widget widget,Module module,Output output);

/* In xresources.c */

XFontStruct *StringToFont(char *string);
Pixel *StringToPixel(char *string);
Boolean *StringToBoolean(char *string);
int *StringToInt(char *string);
int *StringToLabelPosition(char *string);

/* In xbitmap.c */

extern Pixmap CircleBitmap;
extern Pixmap TextBitmap;
extern Pixmap GraphBitmap;

void CreateBitmaps(Widget w);

#endif /* XWINDOW_H */