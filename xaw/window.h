/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/window.h,v 1.5 2002-06-04 12:52:32 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3b.

  Global X windows header file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2000 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef XWINDOW_H
#define XWINDOW_H    /*+ To stop multiple inclusions. +*/

#include "procmeterp.h"

/* In window.c */

extern Display *display;
extern XtAppContext app_context;

void MoveOutput(Output output1,Output output2,int direction);
void Resize(void);

/* In menus.c */

void CreateMenus(Widget parent);
void DestroyMenus(void);
void AddMenuToOutput(Widget widget,Module module);

/* In resources.c */

XFontStruct *StringToFont(char *string);
Pixel StringToPixel(char *string);
Boolean StringToBoolean(char *string);
int StringToInt(char *string);
int StringToLabelPosition(char *string);

/* In bitmap.c */

extern Pixmap CircleBitmap;
extern Pixmap TextBitmap;
extern Pixmap GraphBitmap;
extern Pixmap BarBitmap;

void CreateBitmaps(Widget w);

#endif /* XWINDOW_H */
