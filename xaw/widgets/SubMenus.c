/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/widgets/SubMenus.c,v 1.2 1999-09-28 18:41:08 amb Exp $

  ProcMeter Extensions for Athena SubMenus (for ProcMeter 3.2).
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>

#include "SubMenus.h"


/*+ A structure to contain the menu and sub-menu relationship information. +*/
typedef struct SubMenu
{
 Widget item;                   /*+ The menu item that drops the sub-menu down. +*/
 Widget submenu;                /*+ The drop down sub-menu. +*/
 int depth;                     /*+ The depth of this menu in the visible submenus. +*/
}
SubMenu;


/*+ The klist of menu items and submenus. +*/
static SubMenu *submenus=NULL;

/*+ The number of submenus. +*/
static int nsubmenus=0;

/* The local functions */
static void SubMenuEvent(Widget w,XEvent *event,String *params,Cardinal *num_params);

/*+ The actions we need to add for the menu item. +*/
static XtActionsRec SubMenuActions[]={{"SubMenuEvent",SubMenuEvent}};


/*++++++++++++++++++++++++++++++++++++++
  Initialise the submenu actions.

  XtAppContext app_context
  ++++++++++++++++++++++++++++++++++++++*/

void InitialiseSubMenus(XtAppContext app_context)
{
 /* Add the application actions. */

 XtAppAddActions(app_context,SubMenuActions,sizeof(SubMenuActions)/sizeof(SubMenuActions[0]));
}

/*++++++++++++++++++++++++++++++++++++++
  Adds a menu item and a submenu to the list.

  Widget item The menu item to add.

  Widget submenu The submenu to add.
  ++++++++++++++++++++++++++++++++++++++*/

void AddSubMenu(Widget item,Widget submenu)
{
 /* Add the item and menu to the list. */

 if(nsubmenus==0)
    submenus=(SubMenu*)malloc(8*sizeof(SubMenu));
 else if((nsubmenus%8)==7)
    submenus=(SubMenu*)realloc((void*)submenus,(8+nsubmenus+1)*sizeof(SubMenu));

 submenus[nsubmenus].item=item;
 submenus[nsubmenus].submenu=submenu;
 submenus[nsubmenus].depth=0;

 nsubmenus++;

 XtOverrideTranslations(XtParent(item),
                        XtParseTranslationTable("<BtnMotion>: highlight() SubMenuEvent()\n"
                                                "<BtnUp>:     MenuPopdown() notify() unhighlight() SubMenuEvent()\n"));
}


/*++++++++++++++++++++++++++++++++++++++
  Do something with the sub-menus when there is an interesting button event.

  Widget w The widget that caused the event.

  XEvent *event The event that caused the callback.

  String *params The parameters from the callback.

  Cardinal *num_params The number of parameters.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void SubMenuEvent(Widget w,XEvent *event,String *params,Cardinal *num_params)
{
 static Widget lastitem=NULL;
 int i,depth=0;
 Widget item,submenu=NULL;
 Position root_x,root_y;
 Position item_y;
 Dimension item_width,submenu_width,submenu_height;

 item=XawSimpleMenuGetActiveEntry(w);

 for(i=0;i<nsubmenus;i++)
    if(item==submenus[i].item)
       submenu=submenus[i].submenu;

 for(i=0;i<nsubmenus;i++)
    if(w==submenus[i].submenu)
       depth=submenus[i].depth;

 if(!item || item!=lastitem)
    for(i=0;i<nsubmenus;i++)
       if(submenus[i].depth>depth)
         {
          submenus[i].depth=0;
          XtPopdown(submenus[i].submenu);
         }

 if(!item || item==lastitem)
    return;

 lastitem=item;

 for(i=0;i<nsubmenus;i++)
    if(item==submenus[i].item)
       submenus[i].depth=++depth;

 XtRealizeWidget(submenu);

 XtVaGetValues(submenu,XtNheight,&submenu_height,XtNwidth,&submenu_width,NULL);
 XtVaGetValues(item,XtNy,&item_y,XtNwidth,&item_width,NULL);
 XtTranslateCoords(w,0,event->xbutton.y,&root_x,&root_y);

 root_y-=event->xbutton.y-item_y;
 if((DisplayHeight(XtDisplay(w),DefaultScreen(XtDisplay(w)))-submenu_height-2)<root_y)
    root_y=DisplayHeight(XtDisplay(w),DefaultScreen(XtDisplay(w)))-submenu_height-2;
 if(root_y<2)
    root_y=2;

 if(root_x<(DisplayWidth(XtDisplay(w),DefaultScreen(XtDisplay(w)))/2))
    XtVaSetValues(submenu,XtNx,root_x+item_width,XtNy,root_y,NULL);
 else
    XtVaSetValues(submenu,XtNx,root_x-submenu_width,XtNy,root_y,NULL);

 XtPopup(submenu,XtGrabNone);
}


/*++++++++++++++++++++++++++++++++++++++
  Popup a menu, a replacement for the main part of the XawPositionSimpleMenu() and MenuPopup() translations.

  Widget menu The menu to popup.

  Widget w The widget causing the event.

  XEvent *event The event that causes the popup.
  ++++++++++++++++++++++++++++++++++++++*/

void PopupMenuHere(Widget menu,Widget w,XEvent *event)
{
 Position root_x,root_y;
 Dimension width,height;

 XtRealizeWidget(menu);

 XtVaGetValues(menu,XtNwidth,&width,XtNheight,&height,NULL);
 XtTranslateCoords(w,event->xbutton.x,event->xbutton.y,&root_x,&root_y);

 root_y-=5;
 if((DisplayHeight(XtDisplay(w),DefaultScreen(XtDisplay(w)))-height-2)<root_y)
    root_y=DisplayHeight(XtDisplay(w),DefaultScreen(XtDisplay(w)))-height-2;
 if(root_y<2)
    root_y=2;

 root_x-=width/2;
 if((DisplayWidth(XtDisplay(w),DefaultScreen(XtDisplay(w)))-width-2)<root_x)
    root_x=DisplayWidth(XtDisplay(w),DefaultScreen(XtDisplay(w)))-width-2;
 if(root_x<2)
    root_x=2;

 XtVaSetValues(menu,XtNx,root_x,XtNy,root_y,NULL);
 XtPopupSpringLoaded(menu);
 XtGrabPointer(menu,True,ButtonReleaseMask|ButtonPressMask,GrabModeAsync,GrabModeAsync,None,None,CurrentTime);
}
