/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/SubMenus.h,v 1.1 1999-09-27 19:02:55 amb Exp $

  ProcMeter Extensions for Athena SubMenus (for ProcMeter 3.2).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1999 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef SUBMENUS_H
#define SUBMENUS_H    /*+ To stop multiple inclusions. +*/



void InitialiseSubMenus(XtAppContext app_context);

void AddSubMenu(Widget item,Widget submenu);

void PopupMenuHere(Widget menu,Widget w,XEvent *event);

#endif /* SUBMENUS_H */
