/***************************************
  $Header: /home/amb/CVS/procmeter3/gtk1/window.h,v 1.2 2002-06-04 12:52:27 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3b.

  Global X windows header file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2000 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef WINDOW_H
#define WINDOW_H    /*+ To stop multiple inclusions. +*/

#include "procmeterp.h"

/* In window.c */

void MoveOutput(Output output1,Output output2,int direction);
void Resize(void);

/* In menus.c */

void CreateMenus(GtkWidget *parent);
void DestroyMenus(void);
void AddMenuToOutput(GtkWidget *widget,Module module);

/* In resources.c */

GdkFont *StringToFont(char *string);
GdkColor StringToPixel(char *string);
gboolean StringToBoolean(char *string);
int StringToInt(char *string);
int StringToLabelPosition(char *string);

/* In bitmap.c */

extern GdkPixmap *CircleBitmap;
extern GdkPixmap *TextBitmap;
extern GdkPixmap *GraphBitmap;
extern GdkPixmap *BarBitmap;

void CreateBitmaps(GtkWidget *w);

#endif /* WINDOW_H */
