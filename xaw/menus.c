/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/menus.c,v 1.13 1999-11-30 19:48:28 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.2.

  X Window menus.
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
#include <X11/Shell.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/cursorfont.h>

#include "widgets/SubMenus.h"

#include "xwindow.h"
#include "procmeterp.h"


/* This is already defined if using Xaw3d. */
#ifndef XtNshadowWidth
#define XtNshadowWidth "shadowWidth"
#endif

/*+ The orientation of the windows. +*/
extern int vertical;

/*+ The pane that contains all of the outputs. +*/
extern Widget pane;

/* Local functions */

static void ModuleMenuStart(Widget w,XEvent *event,String *params,Cardinal *num_params);
static void OutputMenuStart(Widget w,XEvent *event,String *params,Cardinal *num_params);
static void FunctionsMenuStart(Widget w,XEvent *event,String *params,Cardinal *num_params);

static void SelectOutputMenuCallback(Widget widget,XtPointer clientData,XtPointer callData);
static void SelectFunctionsMenuCallback(Widget widget,XtPointer clientData,XtPointer callData);

static void DonePropertiesDialogCallback(Widget widget,XtPointer clientData,XtPointer callData);
static void AtomPropertiesDialogCloseCallback(Widget w,XtPointer va,XEvent* e,Boolean* vb);

/* Menu widgets */

static Widget module_menu;
static Widget functions_menu,properties_dialog;
static Widget func_run;
static Widget prop_modname,prop_moddesc,
              prop_outname,prop_outdesc,
              prop_label,prop_type,prop_interval,prop_scale;
static Boolean properties_popped_up=False,doing_move=False;

XtActionsRec MenuActions[]={{"ModuleMenuStart",ModuleMenuStart},
                            {"OutputMenuStart",OutputMenuStart},
                            {"FunctionsMenuStart",FunctionsMenuStart}};

/*+ The output that was used for the Functions menu. +*/
static Output function_output;


/*++++++++++++++++++++++++++++++++++++++
  Initialise the menus.

  Widget parent The parent widget that the menu belongs to.
  ++++++++++++++++++++++++++++++++++++++*/

void CreateMenus(Widget parent)
{
 Widget menulabel;
 Widget menuline;
 Widget func_prop,func_above,func_below,func_delete;
 Widget prop_form,
        prop_modlabel,prop_outlabel,
        prop_lbllabel,prop_typlabel,prop_intlabel,prop_scllabel,
        prop_done;
 Dimension width,height;
 char *string;
 Arg args[3];
 int nargs=0;
 XtVarArgsList resources=NULL;
 Atom close_atom;

 /* Add the application actions. */

 XtAppAddActions(app_context,MenuActions,sizeof(MenuActions)/sizeof(MenuActions[0]));
 InitialiseSubMenus(app_context);

 /* Sort out the resources in advance, this is required for the dialog box.
    This is horrible because Xaw3d requires that you set the background when you
    create the widget because the shadows don't update if you do it later. */

 if((string=GetProcMeterRC("resources","menu-foreground")))
   {XtSetArg(args[nargs],XtNforeground,StringToPixel(string));nargs++;}

 if((string=GetProcMeterRC("resources","menu-background")))
   {XtSetArg(args[nargs],XtNbackground,StringToPixel(string));nargs++;}

 if((string=GetProcMeterRC("resources","menu-font")))
   {XtSetArg(args[nargs],XtNfont,StringToFont(string));nargs++;}

 if(nargs==1)
    resources=XtVaCreateArgsList(NULL,
                                 args[0].name,args[0].value,
                                 NULL);
 else if(nargs==2)
    resources=XtVaCreateArgsList(NULL,
                                 args[0].name,args[0].value,
                                 args[1].name,args[1].value,
                                 NULL);
 else if(nargs==3)
    resources=XtVaCreateArgsList(NULL,
                                 args[0].name,args[0].value,
                                 args[1].name,args[1].value,
                                 args[2].name,args[2].value,
                                 NULL);
 else
    resources=XtVaCreateArgsList(NULL,
                                 NULL);

 /* The module menu */

 module_menu=XtVaCreatePopupShell("ModuleMenu",simpleMenuWidgetClass,parent,
                                  XtNlabel,"Modules",
                                  XtVaNestedList,resources,
                                  NULL);


 menulabel=XtNameToWidget(module_menu,"menuLabel");
 XtSetValues(menulabel,args,nargs);

 menuline=XtVaCreateManagedWidget("line",smeLineObjectClass,module_menu,
                                  XtVaNestedList,resources,
                                  NULL);

 /* The functions menu */

 functions_menu=XtVaCreatePopupShell("FunctionsMenu",simpleMenuWidgetClass,parent,
                                     XtNlabel,"Functions",
                                     XtVaNestedList,resources,
                                     NULL);

 menulabel=XtNameToWidget(functions_menu,"menuLabel");
 XtSetValues(menulabel,args,nargs);

 menuline=XtVaCreateManagedWidget("line",smeLineObjectClass,functions_menu,
                                  XtVaNestedList,resources,
                                  NULL);

 /* The functions menu items */

 func_prop=XtVaCreateManagedWidget("Properties",smeBSBObjectClass,functions_menu,
                                   XtNlabel,"Properties",
                                   XtNheight,10,
                                   XtVaNestedList,resources,
                                   NULL);

 XtAddCallback(func_prop,XtNcallback,SelectFunctionsMenuCallback,0);

 func_above=XtVaCreateManagedWidget("AboveOrLeft",smeBSBObjectClass,functions_menu,
                                    XtNlabel,vertical?"Move To Above":"Move To Left Of",
                                    XtNheight,10,
                                    XtVaNestedList,resources,
                                    NULL);

 XtAddCallback(func_above,XtNcallback,SelectFunctionsMenuCallback,(XtPointer)1);

 func_below=XtVaCreateManagedWidget("BelowOrRight",smeBSBObjectClass,functions_menu,
                                    XtNlabel,vertical?"Move To Below":"Move To Right Of",
                                    XtNheight,10,
                                    XtVaNestedList,resources,
                                    NULL);

 XtAddCallback(func_below,XtNcallback,SelectFunctionsMenuCallback,(XtPointer)2);

 func_delete=XtVaCreateManagedWidget("Delete",smeBSBObjectClass,functions_menu,
                                     XtNlabel,"Delete",
                                     XtNheight,10,
                                     XtVaNestedList,resources,
                                     NULL);

 XtAddCallback(func_delete,XtNcallback,SelectFunctionsMenuCallback,(XtPointer)3);

 func_run=XtVaCreateManagedWidget("Run",smeBSBObjectClass,functions_menu,
                                  XtNlabel,"Run",
                                  XtNheight,10,
                                  XtVaNestedList,resources,
                                  NULL);

 XtAddCallback(func_run,XtNcallback,SelectFunctionsMenuCallback,(XtPointer)4);

 /* The properties_dialog */

 properties_dialog=XtVaCreatePopupShell("PropertiesDialog",topLevelShellWidgetClass,parent,
                                        XtNtitle,"ProcMeter Properties",XtNiconName,"ProcMeter Properties",
                                        XtVaNestedList,resources,
                                        NULL);

 prop_form=XtVaCreateManagedWidget("PropertiesForm",formWidgetClass,properties_dialog,
                                   XtNwidth,300,XtNheight,300,
                                   XtVaNestedList,resources,
                                   NULL);

 /* The properties dialog elements. */

 prop_modlabel=XtVaCreateManagedWidget("ModuleLabel",labelWidgetClass,prop_form,
                                       XtNlabel,"Module:",
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNborderWidth,0,XtNshadowWidth,0,
                                       XtVaNestedList,resources,
                                       NULL);

 prop_modname=XtVaCreateManagedWidget("ModuleName",labelWidgetClass,prop_form,
                                      XtNlabel,"NNNNNNNNNNNNNNN",XtNjustify,XtJustifyLeft,
                                      XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                      XtNfromHoriz,prop_modlabel,
                                      XtNborderWidth,0,XtNshadowWidth,0,
                                      XtVaNestedList,resources,
                                      NULL);

 prop_moddesc=XtVaCreateManagedWidget("ModuleDesc",asciiTextWidgetClass,prop_form,
                                      XtNheight,80,XtNwidth,240,XtNresizable,True,
                                      XtNleft,XawChainLeft,XtNright,XawChainRight,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                      XtNfromVert,prop_modlabel,
                                      XtNtype,XawAsciiString,XtNstring,"NNNNNNNNNNNNNNN",
                                      XtNwrap,XawtextWrapWord,XtNeditType,XawtextRead,XtNdisplayCaret,False,
                                      XtNscrollVertical,XawtextScrollAlways,
                                      XtVaNestedList,resources,
                                      NULL);

 prop_outlabel=XtVaCreateManagedWidget("OutputLabel",labelWidgetClass,prop_form,
                                       XtNlabel,"Output:",
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNfromVert,prop_moddesc,
                                       XtNborderWidth,0,XtNshadowWidth,0,
                                       XtVaNestedList,resources,
                                       NULL);

 prop_outname=XtVaCreateManagedWidget("OutputName",labelWidgetClass,prop_form,
                                      XtNlabel,"NNNNNNNNNNNNNNN",XtNjustify,XtJustifyLeft,
                                      XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                      XtNfromHoriz,prop_outlabel,XtNfromVert,prop_moddesc,
                                      XtNborderWidth,0,XtNshadowWidth,0,
                                      XtVaNestedList,resources,
                                      NULL);

 prop_outdesc=XtVaCreateManagedWidget("OutputDesc",asciiTextWidgetClass,prop_form,
                                      XtNheight,80,XtNwidth,240,XtNresizable,True,
                                      XtNleft,XawChainLeft,XtNright,XawChainRight,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                      XtNfromVert,prop_outlabel,
                                      XtNtype,XawAsciiString,XtNstring,"NNNNNNNNNNNNNNN",
                                      XtNwrap,XawtextWrapWord,XtNeditType,XawtextRead,XtNdisplayCaret,False,
                                      XtNscrollVertical,XawtextScrollAlways,
                                      XtVaNestedList,resources,
                                      NULL);

 prop_lbllabel=XtVaCreateManagedWidget("LabelLabel",labelWidgetClass,prop_form,
                                       XtNlabel,"Label:",
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNfromVert,prop_outdesc,
                                       XtNborderWidth,0,XtNshadowWidth,0,
                                       XtVaNestedList,resources,
                                       NULL);

 prop_typlabel=XtVaCreateManagedWidget("TypeLabel",labelWidgetClass,prop_form,
                                       XtNlabel,"Type:",
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNfromVert,prop_lbllabel,
                                       XtNborderWidth,0,XtNshadowWidth,0,
                                       XtVaNestedList,resources,
                                       NULL);

 prop_intlabel=XtVaCreateManagedWidget("IntervalLabel",labelWidgetClass,prop_form,
                                       XtNlabel,"Interval:",
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNfromVert,prop_typlabel,
                                       XtNborderWidth,0,XtNshadowWidth,0,
                                       XtVaNestedList,resources,
                                       NULL);

 prop_scllabel=XtVaCreateManagedWidget("ScaleLabel",labelWidgetClass,prop_form,
                                       XtNlabel,"Grid Spacing:",
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNfromVert,prop_intlabel,
                                       XtNborderWidth,0,XtNshadowWidth,0,
                                       XtVaNestedList,resources,
                                       NULL);

 prop_label=XtVaCreateManagedWidget("Label",labelWidgetClass,prop_form,
                                    XtNlabel,"NNNNNNNNNNNNNNNN",XtNjustify,XtJustifyLeft,
                                    XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                    XtNfromHoriz,prop_scllabel,XtNfromVert,prop_outdesc,
                                    XtNborderWidth,0,XtNshadowWidth,0,
                                    XtVaNestedList,resources,
                                    NULL);

 prop_type=XtVaCreateManagedWidget("Type",labelWidgetClass,prop_form,
                                   XtNlabel,"NNNNNNN",XtNjustify,XtJustifyLeft,
                                   XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                   XtNfromHoriz,prop_scllabel,XtNfromVert,prop_lbllabel,
                                   XtNborderWidth,0,XtNshadowWidth,0,
                                   XtVaNestedList,resources,
                                   NULL);

 prop_interval=XtVaCreateManagedWidget("Interval",labelWidgetClass,prop_form,
                                       XtNlabel,"NNNNNNN",XtNjustify,XtJustifyLeft,
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNfromHoriz,prop_scllabel,XtNfromVert,prop_typlabel,
                                       XtNborderWidth,0,XtNshadowWidth,0,
                                       XtVaNestedList,resources,
                                       NULL);

 prop_scale=XtVaCreateManagedWidget("Scale",labelWidgetClass,prop_form,
                                    XtNlabel,"NNNNNNN",XtNjustify,XtJustifyLeft,
                                    XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                    XtNfromHoriz,prop_scllabel,XtNfromVert,prop_intlabel,
                                    XtNborderWidth,0,XtNshadowWidth,0,
                                    XtVaNestedList,resources,
                                    NULL);

 prop_done=XtVaCreateManagedWidget("Done",commandWidgetClass,prop_form,
                                   XtNwidth,240,XtNresizable,True,
                                   XtNleft,XawChainLeft,XtNright,XawChainRight,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                   XtNfromVert,prop_scllabel,
                                   XtVaNestedList,resources,
                                   NULL);

 XtAddCallback(prop_done,XtNcallback,DonePropertiesDialogCallback,0);

 XtFree(resources);

 /* Fixup the properties dialog box */

 XtRealizeWidget(properties_dialog);

 XtVaGetValues(properties_dialog,XtNwidth,&width,XtNheight,&height,NULL);
 XtVaSetValues(properties_dialog,XtNminWidth,width,XtNminHeight,height,XtNmaxWidth,width,XtNmaxHeight,height,NULL);

 close_atom=XInternAtom(display,"WM_DELETE_WINDOW",False);

 XSetWMProtocols(display,XtWindow(properties_dialog),&close_atom,1);

 XtAddEventHandler(properties_dialog,0,True,AtomPropertiesDialogCloseCallback,NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a specified module to the menus.

  Module module The module to add.
  ++++++++++++++++++++++++++++++++++++++*/

void AddModuleToMenu(Module module)
{
 int i;
 Widget menulabel,menuline,submenu=NULL,menuitem=NULL;
 ProcMeterOutput *prevoutput=NULL;
 char *string;
 char menuname[32];
 Arg args[3];
 int nargs=0;

 if(!display)
    return;

 /* Sort out the resources in advance. */

 if(((string=GetProcMeterRC(module->module->name,"menu-foreground")) ||
     (string=GetProcMeterRC("resources","menu-foreground"))))
   {XtSetArg(args[nargs],XtNforeground,StringToPixel(string));nargs++;}

 if(((string=GetProcMeterRC(module->module->name,"menu-background")) ||
     (string=GetProcMeterRC("resources","menu-background"))))
   {XtSetArg(args[nargs],XtNbackground,StringToPixel(string));nargs++;}

 if(((string=GetProcMeterRC(module->module->name,"menu-font")) ||
     (string=GetProcMeterRC("resources","menu-font"))))
   {XtSetArg(args[nargs],XtNfont,StringToFont(string));nargs++;}

 /* Create a new menu. */

 sprintf(menuname,"%sMenu",module->module->name);
 module->submenu_widget=XtVaCreatePopupShell(menuname,simpleMenuWidgetClass,module_menu,
                                             XtNlabel,module->module->name,
                                             NULL);
 XtSetValues(module->submenu_widget,args,nargs);

 menulabel=XtNameToWidget(module->submenu_widget,"menuLabel");
 XtSetValues(menulabel,args,nargs);

 menuline=XtVaCreateManagedWidget("line",smeLineObjectClass,module->submenu_widget,
                                  NULL);
 XtSetValues(menuline,args,nargs);

 /* Add an entry to the module menu */

 module->menu_item_widget=XtVaCreateManagedWidget(module->module->name,smeBSBObjectClass,module_menu,
                                                  XtNlabel,module->module->name,
                                                  XtNheight,10,
                                                  NULL);
 XtSetValues(module->menu_item_widget,args,nargs);

 AddSubMenu(module->menu_item_widget,module->submenu_widget);

 /* Add entries to it for each output. */

 for(i=0;module->outputs[i];i++)
   {
    Widget sme;
    Pixmap bitmap=CircleBitmap;

    if(module->outputs[i]->output!=prevoutput)
      {
       menuitem=XtVaCreateManagedWidget(module->outputs[i]->output->name,smeBSBObjectClass,module->submenu_widget,
                                       XtNlabel,module->outputs[i]->label,
                                       XtNheight,10,
                                       NULL);

       if(((string=GetProcMeterRC2(module->module->name,module->outputs[i]->output->name,"menu-foreground")) ||
           (string=GetProcMeterRC(module->module->name,"menu-foreground")) ||
           (string=GetProcMeterRC("resources","menu-foreground"))))
          XtVaSetValues(menuitem,XtNforeground,StringToPixel(string),NULL);

       if(((string=GetProcMeterRC2(module->module->name,module->outputs[i]->output->name,"menu-font")) ||
           (string=GetProcMeterRC(module->module->name,"menu-font")) ||
           (string=GetProcMeterRC("resources","menu-font"))))
          XtVaSetValues(menuitem,XtNfont,StringToFont(string),NULL);

       submenu=XtVaCreatePopupShell("Type",simpleMenuWidgetClass,module->submenu_widget,
                                    NULL);
       XtSetValues(submenu,args,nargs);

       AddSubMenu(menuitem,submenu);

       prevoutput=module->outputs[i]->output;
      }

    if(module->outputs[i]->type==PROCMETER_GRAPH)
       bitmap=GraphBitmap;
    else if(module->outputs[i]->type==PROCMETER_TEXT)
       bitmap=TextBitmap;
    else if(module->outputs[i]->type==PROCMETER_BAR)
       bitmap=BarBitmap;

    sme=XtVaCreateManagedWidget(module->outputs[i]->output->name,smeBSBObjectClass,submenu,
                                XtNlabel,"",
                                XtNleftMargin,10,XtNrightMargin,20,
                                XtNrightBitmap,bitmap,
                                XtNheight,10,
                                NULL);

    XtAddCallback(sme,XtNcallback,SelectOutputMenuCallback,(XtPointer)module->outputs[i]);

    if(((string=GetProcMeterRC2(module->module->name,module->outputs[i]->output->name,"menu-foreground")) ||
        (string=GetProcMeterRC(module->module->name,"menu-foreground")) ||
        (string=GetProcMeterRC("resources","menu-foreground"))))
       XtVaSetValues(sme,XtNforeground,StringToPixel(string),NULL);

    module->outputs[i]->menu_item_widget=sme;
    module->outputs[i]->output_widget=NULL;
   }

 XtRealizeWidget(module->submenu_widget);
}


/*++++++++++++++++++++++++++++++++++++++
  Add the menus to the right mouse button of the output.

  Widget widget The widget itself.

  Module module The module that this widget belongs to.

  Output output The output that this widget belongs to.
  ++++++++++++++++++++++++++++++++++++++*/

void AddMenuToOutput(Widget widget,Module module,Output output)
{
 XtOverrideTranslations(widget,XtParseTranslationTable("<Btn3Down>: ModuleMenuStart()"));

 if(module)
   {
    char string[64];
    sprintf(string,"<Btn2Down>: OutputMenuStart(%sMenu)",module->module->name);
    XtOverrideTranslations(widget,XtParseTranslationTable(string));
   }

 if(output)
   {
    char string[80];
    sprintf(string,"<Btn1Down>: FunctionsMenuStart(%s)",module->module->name);
    XtOverrideTranslations(widget,XtParseTranslationTable(string));
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Remove a specified module from the menus.

  Module module The module to remove.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveModuleFromMenu(Module module)
{
 if(!display)
    return;

 XtDestroyWidget(module->menu_item_widget);

 XtDestroyWidget(module->submenu_widget);
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy all of the menus.
  ++++++++++++++++++++++++++++++++++++++*/

void DestroyMenus(void)
{
 XtDestroyWidget(module_menu);
}


/*++++++++++++++++++++++++++++++++++++++
  The callback that is called by the output being selected on the output menu.

  Widget widget The widget that the callback came from.

  XtPointer clientData The client data from the callback.

  XtPointer callData The call data from the callback.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void SelectOutputMenuCallback(Widget widget,XtPointer clientData,XtPointer callData)
{
 Output output=(Output)clientData;

 AddRemoveOutput(output);
}


/*++++++++++++++++++++++++++++++++++++++
  The callback that is called by an item being selected on the functions menu.

  Widget widget The widget that the callback came from.

  XtPointer clientData The client data from the callback.

  XtPointer callData The call data from the callback.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void SelectFunctionsMenuCallback(Widget widget,XtPointer clientData,XtPointer callData)
{
 if((int)clientData==0)         /* Properties */
   {
    Window root_return, child_return;
    int root_x_return, root_y_return;
    int win_x_return, win_y_return;
    unsigned int mask_return;
    Dimension width;

    XtVaGetValues(properties_dialog,XtNwidth,&width,NULL);

    XQueryPointer(display,XtWindow(properties_dialog),&root_return,&child_return,&root_x_return,&root_y_return,&win_x_return,&win_y_return,&mask_return);

    XtVaSetValues(properties_dialog,XtNx,root_x_return-width/2,XtNy,root_y_return,NULL);

    XtPopup(properties_dialog,XtGrabNone);
    properties_popped_up=True;
   }
 else if((int)clientData==1 || (int)clientData==2) /* Above / Below */
   {
    doing_move=(int)clientData;

    XtGrabPointer(pane,True,ButtonPressMask|ButtonReleaseMask,GrabModeAsync,GrabModeAsync,None,XCreateFontCursor(XtDisplay(pane),XC_hand1),CurrentTime);
   }
 else if((int)clientData==3)    /* Delete */
   {
    AddRemoveOutput(function_output);
   }
 else if((int)clientData==4)    /* Run */
   {
    if(function_output->menu_run.flag)
       RunProgram(&function_output->menu_run);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The callback for the done button on the properties dialog box.

  Widget widget The widget that the callback came from.

  XtPointer clientData The client data from the callback.

  XtPointer callData The call data from the callback.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void DonePropertiesDialogCallback(Widget widget,XtPointer clientData,XtPointer callData)
{
 XtPopdown(properties_dialog);
 properties_popped_up=False;
}


/*++++++++++++++++++++++++++++++++++++++
  The callback for the window manager close on the dialog.

  Widget w The widget that caused the callback.

  XtPointer va Not used.

  XEvent* e The event that requires action.

  Boolean* vb Not used.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void AtomPropertiesDialogCloseCallback(Widget w,XtPointer va,XEvent* e,Boolean* vb)
{
 XtPopdown(properties_dialog);
 properties_popped_up=False;
}


/*++++++++++++++++++++++++++++++++++++++
  Start the Module menu.

  Widget w The widget that caused the event.

  XEvent *event The event that caused the callback.

  String *params The parameters from the callback.

  Cardinal *num_params The number of parameters.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void ModuleMenuStart(Widget w,XEvent *event,String *params,Cardinal *num_params)
{
 PopupMenuHere(module_menu,w,event);
}


/*++++++++++++++++++++++++++++++++++++++
  Start one of the Output menus.

  Widget w The widget that caused the event.

  XEvent *event The event that caused the callback.

  String *params The parameters from the callback.

  Cardinal *num_params The number of parameters.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void OutputMenuStart(Widget w,XEvent *event,String *params,Cardinal *num_params)
{
 Widget menu=XtNameToWidget(module_menu,params[0]);

 PopupMenuHere(menu,w,event);
}


/*++++++++++++++++++++++++++++++++++++++
  Start the properties menu.

  Widget w The widget that caused the event.

  XEvent *event The event that caused the callback.

  String *params The parameters from the callback.

  Cardinal *num_params The number of parameters.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void FunctionsMenuStart(Widget w,XEvent *event,String *params,Cardinal *num_params)
{
 static Output *outputp=NULL;
 static Module *modulep=NULL;
 char string[24];

 for(modulep=Modules;*modulep;modulep++)
    if(!strcmp((*modulep)->module->name,params[0]))
      {
       for(outputp=(*modulep)->outputs;*outputp;outputp++)
          if(w==(*outputp)->output_widget)
             break;
       break;
      }

 if(doing_move)
   {
    XtUngrabPointer(pane,CurrentTime);

    MoveOutput(function_output,*outputp,doing_move);
    doing_move=0;
    return;
   }

 if(!*modulep || !*outputp)
    return;

 function_output=*outputp;

 if((*outputp)->menu_run.flag)
   {
    char *r=(*outputp)->menu_run.command;

    strncpy(string,"Run '",16);
    strncpy(string+5,r,8);
    if(strlen(r)>8)
       strcat(string," ...'");
    else
       strcat(string,"'");
 
    XtVaSetValues(func_run,XtNlabel,string,NULL);
    XtSetSensitive(func_run,True);
   }
 else
   {
    XtVaSetValues(func_run,XtNlabel,"Run",NULL);
    XtSetSensitive(func_run,False);
   }

 /* Set up the properties window. */

 XtVaSetValues(prop_modname,XtNlabel,(*modulep)->module->name,NULL);
 XtVaSetValues(prop_moddesc,XtNstring,(*modulep)->module->description,NULL);

 XtVaSetValues(prop_outname,XtNlabel,(*outputp)->output->name,NULL);
 XtVaSetValues(prop_outdesc,XtNstring,(*outputp)->output->description,NULL);

 XtVaSetValues(prop_label,XtNlabel,(*outputp)->label,NULL);

 if((*outputp)->type==PROCMETER_GRAPH)
    XtVaSetValues(prop_type,XtNlabel,"Graph",NULL);
 else if((*outputp)->type==PROCMETER_TEXT)
    XtVaSetValues(prop_type,XtNlabel,"Text",NULL);
 else if((*outputp)->type==PROCMETER_BAR)
    XtVaSetValues(prop_type,XtNlabel,"Bar",NULL);

 if((*outputp)->output->interval)
    sprintf(string,"%d s",(*outputp)->output->interval);
 else
    strcpy(string,"Never");
 XtVaSetValues(prop_interval,XtNlabel,string,NULL);

 if((*outputp)->type&(PROCMETER_GRAPH|PROCMETER_BAR))
   {
    char str[16];
    sprintf(str,(*outputp)->output->graph_units,(*outputp)->output->graph_scale);
    if(*str=='(')
       strcpy(string,str+1);
    else
       strcpy(string,str);
    if(string[strlen(string)-1]==')')
       string[strlen(string)-1]=0;
    XtVaSetValues(prop_scale,XtNlabel,string,NULL);
    XtSetSensitive(prop_scale,True);
   }
 else
   {
    XtVaSetValues(prop_scale,XtNlabel,"n/a",NULL);
    XtSetSensitive(prop_scale,False);
   }

 if(!properties_popped_up)
    PopupMenuHere(functions_menu,w,event);
}
