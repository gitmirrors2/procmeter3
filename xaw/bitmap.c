/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/bitmap.c,v 1.3 2000-12-16 16:46:19 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3.

  X Window Bitmaps.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,2000 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#include "window.h"


#define circle_width  8
#define circle_height 8
static unsigned char circle_bits[] = {
   0x00, 0x18, 0x3c, 0x7e, 0x7e, 0x3c, 0x18, 0x00};

#define graph_width 16
#define graph_height 8
static unsigned char graph_bits[] = {
   0x00, 0x00, 0x42, 0x00, 0xa2, 0x41, 0x12, 0x22, 0x0e, 0x1c, 0x02, 0x00,
   0xfe, 0x7f, 0x00, 0x00};

#define text_width 16
#define text_height 8
static unsigned char text_bits[] = {
   0x00, 0x00, 0xcc, 0x31, 0x52, 0x4a, 0xd2, 0x09, 0x5e, 0x0a, 0x52, 0x4a,
   0xd2, 0x31, 0x00, 0x00};

#define bar_width 16
#define bar_height 8
static unsigned char bar_bits[] = {
   0x01, 0x00, 0xff, 0x0f, 0xff, 0x0f, 0xff, 0x0f, 0xff, 0x0f, 0x01, 0x00,
   0x49, 0x92, 0xff, 0xff};

Pixmap CircleBitmap;
Pixmap GraphBitmap;
Pixmap TextBitmap;
Pixmap BarBitmap;


/*++++++++++++++++++++++++++++++++++++++
  Initialise the Bitmaps.

  Widget w A widget to start with.
  ++++++++++++++++++++++++++++++++++++++*/

void CreateBitmaps(Widget w)
{
 CircleBitmap=XCreateBitmapFromData(display,RootWindowOfScreen(XtScreen(w)),circle_bits,circle_width,circle_height);
 GraphBitmap=XCreateBitmapFromData(display,RootWindowOfScreen(XtScreen(w)),graph_bits,graph_width,graph_height);
 TextBitmap=XCreateBitmapFromData(display,RootWindowOfScreen(XtScreen(w)),text_bits,text_width,text_height);
 BarBitmap=XCreateBitmapFromData(display,RootWindowOfScreen(XtScreen(w)),bar_bits,bar_width,bar_height);
}
