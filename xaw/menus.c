/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/menus.c,v 1.5 1999-02-09 19:56:18 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.1.

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

#include "xwindow.h"
#include "procmeterp.h"


/*+ The orientation of the windows. +*/
extern int vertical;

/*+ The pane that contains all of the outputs. +*/
extern Widget pane;

static void ModuleMenuStart(Widget w,XEvent *event,String *params,Cardinal *num_params);
static void ModuleMenuEvent(Widget w,XEvent *event,String *params,Cardinal *num_params);
static void OutputMenuStart(Widget w,XEvent *event,String *params,Cardinal *num_params);
static void FunctionsMenuStart(Widget w,XEvent *event,String *params,Cardinal *num_params);

static void PopupAMenu(Widget menu,Widget w,XEvent *event);

static void SelectModuleMenuCallback(Widget widget,XtPointer clientData,XtPointer callData);
static void SelectOutputMenuCallback(Widget widget,XtPointer clientData,XtPointer callData);
static void SelectFunctionsMenuCallback(Widget widget,XtPointer clientData,XtPointer callData);
static void DonePropertiesDialogCallback(Widget widget,XtPointer clientData,XtPointer callData);
static void AtomPropertiesDialogCloseCallback(Widget w,XtPointer va,XEvent* e,Boolean* vb);


static Widget module_menu;
static Widget functions_menu,properties_dialog;
static Widget sme_run;
static Widget prop_modname,prop_moddesc,prop_outname,prop_outdesc,prop_type,prop_interval,prop_scale;
static Boolean properties_popped_up=False,doing_move=False;

XtActionsRec MenuActions[]={{"ModuleMenuStart",ModuleMenuStart},
                            {"ModuleMenuEvent",ModuleMenuEvent},
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
 Widget menulabel1,smeline1;
 Widget menulabel2,smeline2,sme_prop,sme_above,sme_below;
 Widget prop_form,prop_modlabel,prop_outlabel,prop_typlabel,prop_intlabel,prop_scllabel,prop_done;
 Dimension width,height;
 char *string;
 XtPointer resource;
 Atom close_atom;

 XtAppAddActions(app_context,MenuActions,sizeof(MenuActions)/sizeof(MenuActions[0]));

 /* The module menu */

 module_menu=XtVaCreatePopupShell("ModuleMenu",simpleMenuWidgetClass,parent,
                                  XtNlabel,"Modules",
                                  NULL);

 XtOverrideTranslations(module_menu,XtParseTranslationTable("<BtnMotion>: highlight() ModuleMenuEvent()\n"
                                                            "<BtnUp>:     MenuPopdown() notify() unhighlight() ModuleMenuEvent()\n"));

 menulabel1=XtNameToWidget(module_menu,"menuLabel");

 smeline1=XtVaCreateManagedWidget("line",smeLineObjectClass,module_menu,NULL);

 /* The functions menu */

 functions_menu=XtVaCreatePopupShell("FunctionsMenu",simpleMenuWidgetClass,parent,
                                     XtNlabel,"Functions",
                                     NULL);

 menulabel2=XtNameToWidget(functions_menu,"menuLabel");

 smeline2=XtVaCreateManagedWidget("line",smeLineObjectClass,functions_menu,NULL);

 sme_prop=XtVaCreateManagedWidget("Properties",smeBSBObjectClass,functions_menu,
                                  XtNlabel,"Properties",
                                  XtNheight,10,
                                  NULL);

 XtAddCallback(sme_prop,XtNcallback,SelectFunctionsMenuCallback,0);

 sme_above=XtVaCreateManagedWidget("AboveOrLeft",smeBSBObjectClass,functions_menu,
                                   XtNlabel,vertical?"Move To Above":"Move To Left Of",
                                   XtNheight,10,
                                   NULL);

 XtAddCallback(sme_above,XtNcallback,SelectFunctionsMenuCallback,(XtPointer)1);

 sme_below=XtVaCreateManagedWidget("BelowOrRight",smeBSBObjectClass,functions_menu,
                                   XtNlabel,vertical?"Move To Below":"Move To Right Of",
                                   XtNheight,10,
                                   NULL);

 XtAddCallback(sme_below,XtNcallback,SelectFunctionsMenuCallback,(XtPointer)2);

 sme_run=XtVaCreateManagedWidget("Run",smeBSBObjectClass,functions_menu,
                                 XtNlabel,"Run",
                                 XtNheight,10,
                                 NULL);

 XtAddCallback(sme_run,XtNcallback,SelectFunctionsMenuCallback,(XtPointer)3);

 /* The properties_dialog */

 properties_dialog=XtVaCreatePopupShell("PropertiesDialog",topLevelShellWidgetClass,parent,
                                        XtNtitle,"ProcMeter Properties",XtNiconName,"ProcMeter Properties",
                                        NULL);

 prop_form=XtVaCreateManagedWidget("PropertiesForm",formWidgetClass,properties_dialog,
                                   XtNwidth,200,XtNheight,200,
                                   NULL);

 prop_modlabel=XtVaCreateManagedWidget("ModuleLabel",labelWidgetClass,prop_form,
                                       XtNlabel,"Module:",
                                       XtNwidth,100,XtNresizable,True,
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNborderWidth,0,
                                       NULL);
 prop_modname=XtVaCreateManagedWidget("ModuleName",labelWidgetClass,prop_form,
                                      XtNlabel,"NNNNNNNNNNNNNNN",XtNjustify,XtJustifyLeft,
                                      XtNwidth,100,XtNresizable,True,
                                      XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                      XtNfromHoriz,prop_modlabel,
                                      NULL);

 prop_moddesc=XtVaCreateManagedWidget("ModuleDesc",asciiTextWidgetClass,prop_form,
                                      XtNheight,100,XtNwidth,200,XtNresizable,True,
                                      XtNleft,XawChainLeft,XtNright,XawChainRight,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                      XtNfromVert,prop_modlabel,
                                      XtNtype,XawAsciiString,XtNstring,"NNNNNNNNNNNNNNN",
                                      XtNwrap,XawtextWrapWord,XtNeditType,XawtextRead,XtNdisplayCaret,False,
                                      XtNscrollVertical,XawtextScrollAlways,
                                      NULL);

 prop_outlabel=XtVaCreateManagedWidget("OutputLabel",labelWidgetClass,prop_form,
                                       XtNlabel,"Output:",
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNfromVert,prop_moddesc,
                                       XtNborderWidth,0,
                                       NULL);
 prop_outname=XtVaCreateManagedWidget("OutputName",labelWidgetClass,prop_form,
                                      XtNlabel,"NNNNNNNNNNNNNNN",XtNjustify,XtJustifyLeft,
                                      XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                      XtNfromHoriz,prop_outlabel,XtNfromVert,prop_moddesc,
                                      NULL);

 prop_outdesc=XtVaCreateManagedWidget("OutputDesc",asciiTextWidgetClass,prop_form,
                                      XtNheight,100,XtNwidth,200,XtNresizable,True,
                                      XtNleft,XawChainLeft,XtNright,XawChainRight,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                      XtNfromVert,prop_outlabel,
                                      XtNtype,XawAsciiString,XtNstring,"NNNNNNNNNNNNNNN",
                                      XtNwrap,XawtextWrapWord,XtNeditType,XawtextRead,XtNdisplayCaret,False,
                                      XtNscrollVertical,XawtextScrollAlways,
                                      NULL);

 prop_typlabel=XtVaCreateManagedWidget("TypeLabel",labelWidgetClass,prop_form,
                                       XtNlabel,"Type:",
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNfromVert,prop_outdesc,
                                       XtNborderWidth,0,
                                       NULL);
 prop_type=XtVaCreateManagedWidget("Type",labelWidgetClass,prop_form,
                                   XtNlabel,"NNNNNNN",XtNjustify,XtJustifyLeft,
                                   XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                   XtNfromHoriz,prop_typlabel,XtNfromVert,prop_outdesc,
                                   NULL);

 prop_intlabel=XtVaCreateManagedWidget("IntervalLabel",labelWidgetClass,prop_form,
                                       XtNlabel,"Interval:",
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNfromVert,prop_typlabel,
                                       XtNborderWidth,0,
                                       NULL);
 prop_interval=XtVaCreateManagedWidget("Interval",labelWidgetClass,prop_form,
                                       XtNlabel,"NNNNNNN",XtNjustify,XtJustifyLeft,
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNfromHoriz,prop_intlabel,XtNfromVert,prop_typlabel,
                                       NULL);

 prop_scllabel=XtVaCreateManagedWidget("ScaleLabel",labelWidgetClass,prop_form,
                                       XtNlabel,"Grid Spacing:",
                                       XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                       XtNfromVert,prop_intlabel,
                                       XtNborderWidth,0,
                                       NULL);
 prop_scale=XtVaCreateManagedWidget("Scale",labelWidgetClass,prop_form,
                                    XtNlabel,"NNNNNNN",XtNjustify,XtJustifyLeft,
                                    XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                    XtNfromHoriz,prop_scllabel,XtNfromVert,prop_intlabel,
                                    NULL);

 prop_done=XtVaCreateManagedWidget("Done",commandWidgetClass,prop_form,
                                   XtNwidth,200,XtNresizable,False,
                                   XtNleft,XawChainLeft,XtNright,XawChainLeft,XtNtop,XawChainTop,XtNbottom,XawChainTop,
                                   XtNfromVert,prop_scllabel,
                                   NULL);
 XtAddCallback(prop_done,XtNcallback,DonePropertiesDialogCallback,0);

 /* Add the resources. */

 if(((string=GetProcMeterRC("resources","menu-foreground"))) &&
    (resource=(XtPointer)StringToPixel(string)))
   {
    XtVaSetValues(menulabel1,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(menulabel2,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(smeline1,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(smeline2,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(sme_prop,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(sme_above,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(sme_below,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(sme_run,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(properties_dialog,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_form,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_modlabel,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_modname,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_moddesc,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_outlabel,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_outname,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_outdesc,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_typlabel,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_type,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_intlabel,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_interval,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_scllabel,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_scale,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_done,XtNforeground,*(Pixel*)resource,NULL);
   }

 if(((string=GetProcMeterRC("resources","menu-background"))) &&
    (resource=(XtPointer)StringToPixel(string)))
   {
    XtVaSetValues(module_menu,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(functions_menu,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(properties_dialog,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_form,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_modlabel,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_modname,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_moddesc,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_outlabel,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_outname,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_outdesc,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_typlabel,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_type,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_intlabel,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_interval,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_scllabel,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_scale,XtNbackground,*(Pixel*)resource,NULL);
    XtVaSetValues(prop_done,XtNbackground,*(Pixel*)resource,NULL);
   }

 if(((string=GetProcMeterRC("resources","menu-font"))) &&
    (resource=(XtPointer)StringToFont(string)))
   {
    XtVaSetValues(menulabel1,XtNfont,resource,NULL);
    XtVaSetValues(menulabel2,XtNfont,resource,NULL);
    XtVaSetValues(sme_prop,XtNfont,resource,NULL);
    XtVaSetValues(sme_above,XtNfont,resource,NULL);
    XtVaSetValues(sme_below,XtNfont,resource,NULL);
    XtVaSetValues(sme_run,XtNfont,resource,NULL);
    XtVaSetValues(prop_form,XtNfont,resource,NULL);
    XtVaSetValues(prop_modlabel,XtNfont,resource,NULL);
    XtVaSetValues(prop_modname,XtNfont,resource,NULL);
    XtVaSetValues(prop_moddesc,XtNfont,resource,NULL);
    XtVaSetValues(prop_outlabel,XtNfont,resource,NULL);
    XtVaSetValues(prop_outname,XtNfont,resource,NULL);
    XtVaSetValues(prop_outdesc,XtNfont,resource,NULL);
    XtVaSetValues(prop_typlabel,XtNfont,resource,NULL);
    XtVaSetValues(prop_type,XtNfont,resource,NULL);
    XtVaSetValues(prop_intlabel,XtNfont,resource,NULL);
    XtVaSetValues(prop_interval,XtNfont,resource,NULL);
    XtVaSetValues(prop_scllabel,XtNfont,resource,NULL);
    XtVaSetValues(prop_scale,XtNfont,resource,NULL);
    XtVaSetValues(prop_done,XtNfont,resource,NULL);
   }

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
 Widget menulabel,smeline;
 char *string;
 XtPointer resource;
 char menuname[32];

 if(!display)
    return;

 /* Create a new menu. */

 sprintf(menuname,"%sMenu",module->module->name);
 module->submenu_widget=XtVaCreatePopupShell(menuname,simpleMenuWidgetClass,module_menu,
                                             XtNlabel,module->module->name,
                                             NULL);

 menulabel=XtNameToWidget(module->submenu_widget,"menuLabel");

 smeline=XtVaCreateManagedWidget("line",smeLineObjectClass,module->submenu_widget,
                                 NULL);

 /* Add an entry to the module menu */

 module->menu_item_widget=XtVaCreateManagedWidget(module->module->name,smeBSBObjectClass,module_menu,
                                                  XtNlabel,module->module->name,
                                                  XtNheight,10,
                                                  NULL);

 XtAddCallback(module->menu_item_widget,XtNcallback,SelectModuleMenuCallback,(XtPointer)module->module);

 /* Add the resources. */

 if(((string=GetProcMeterRC(module->module->name,"menu-foreground")) ||
     (string=GetProcMeterRC("resources","menu-foreground"))) &&
    (resource=(XtPointer)StringToPixel(string)))
   {
    XtVaSetValues(menulabel,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(smeline,XtNforeground,*(Pixel*)resource,NULL);
    XtVaSetValues(module->menu_item_widget,XtNforeground,*(Pixel*)resource,NULL);
   }

 if(((string=GetProcMeterRC(module->module->name,"menu-background")) ||
     (string=GetProcMeterRC("resources","menu-background"))) &&
    (resource=(XtPointer)StringToPixel(string)))
    XtVaSetValues(module->submenu_widget,XtNbackground,*(Pixel*)resource,NULL);

 if(((string=GetProcMeterRC(module->module->name,"menu-font")) ||
     (string=GetProcMeterRC("resources","menu-font"))) &&
    (resource=(XtPointer)StringToFont(string)))
   {
    XtVaSetValues(menulabel,XtNfont,resource,NULL);
    XtVaSetValues(module->menu_item_widget,XtNfont,resource,NULL);
   }

 /* Add entries to it for each output. */

 for(i=0;module->outputs[i];i++)
   {
    Widget sme;
    Pixmap bitmap=CircleBitmap;

    if(module->outputs[i]->type==PROCMETER_GRAPH)
       bitmap=GraphBitmap;
    else if(module->outputs[i]->type==PROCMETER_TEXT)
       bitmap=TextBitmap;

    sme=XtVaCreateManagedWidget(module->outputs[i]->output->name,smeBSBObjectClass,module->submenu_widget,
                                XtNlabel,module->outputs[i]->output->name,
                                XtNleftMargin,12,XtNrightMargin,20,
                                XtNrightBitmap,bitmap,
                                XtNheight,10,
                                NULL);

    XtAddCallback(sme,XtNcallback,SelectOutputMenuCallback,(XtPointer)module->outputs[i]);

    if(((string=GetProcMeterRC2(module->module->name,module->outputs[i]->output->name,"menu-foreground")) ||
        (string=GetProcMeterRC(module->module->name,"menu-foreground")) ||
        (string=GetProcMeterRC("resources","menu-foreground"))) &&
       (resource=(XtPointer)StringToPixel(string)))
       XtVaSetValues(sme,XtNforeground,*(Pixel*)resource,NULL);

    if(((string=GetProcMeterRC2(module->module->name,module->outputs[i]->output->name,"menu-font")) ||
        (string=GetProcMeterRC(module->module->name,"menu-font")) ||
        (string=GetProcMeterRC("resources","menu-font"))) &&
       (resource=(XtPointer)StringToFont(string)))
       XtVaSetValues(sme,XtNfont,resource,NULL);

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
  The callback that is called by the module being selected on the module menu.

  Widget widget The widget that the callback came from.

  XtPointer clientData The client data from the callback.

  XtPointer callData The call data from the callback.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void SelectModuleMenuCallback(Widget widget,XtPointer clientData,XtPointer callData)
{
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
 else if((int)clientData==3)    /* Run */
   {
    if(fork()==0)
      {
       execl("/bin/sh","/bin/sh","-c",function_output->run,NULL);
       exit(1);
      }
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
 PopupAMenu(module_menu,w,event);
}


/*++++++++++++++++++++++++++++++++++++++
  Do something with the sub-menus when there is an interesting button event.

  Widget w The widget that caused the event.

  XEvent *event The event that caused the callback.

  String *params The parameters from the callback.

  Cardinal *num_params The number of parameters.

  This function is only ever called from the Xt Intrinsics routines.
  ++++++++++++++++++++++++++++++++++++++*/

static void ModuleMenuEvent(Widget w,XEvent *event,String *params,Cardinal *num_params)
{
 static Widget lastitem=NULL,lastmenu=NULL;
 Widget sme;
 Module *module;

 sme=XawSimpleMenuGetActiveEntry(w);

 if((!sme || sme!=lastitem) && lastmenu)
   {
    XtPopdown(lastmenu);
    lastmenu=lastitem=NULL;
   }

 if(!sme || sme==lastitem)
    return;

 lastitem=sme;

 for(module=Modules;*module;module++)
    if(sme==(*module)->menu_item_widget)
      {
       Position root_x,root_y;
       Dimension width,height;

       lastmenu=(*module)->submenu_widget;

       XtTranslateCoords(w,0,event->xbutton.y,&root_x,&root_y);
       XtVaGetValues(lastmenu,XtNheight,&height,NULL);

       root_y-=5;
       if((DisplayHeight(display,DefaultScreen(display))-height-2)<root_y)
          root_y=DisplayHeight(display,DefaultScreen(display))-height-2;
       if(root_y<2)
          root_y=2;

       if(root_x<(DisplayWidth(display,DefaultScreen(display))/2))
         {
          XtVaGetValues(w,XtNwidth,&width,NULL);
          XtVaSetValues(lastmenu,XtNx,root_x+width,XtNy,root_y,NULL);
         }
       else
         {
          XtVaGetValues(lastmenu,XtNwidth,&width,NULL);
          XtVaSetValues(lastmenu,XtNx,root_x-width,XtNy,root_y,NULL);
         }

       XtPopup(lastmenu,XtGrabNone);
      }
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

 PopupAMenu(menu,w,event);
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

 if((*outputp)->run)
   {
    char *r=(*outputp)->run;

    if(!strncmp(r,"xterm -e ",9) && r[9])
       r+=9;
    if(!strncmp(r,"sh -c ",6) && r[6])
       r+=6;
    if(*r=='\'')
       r++;

    strncpy(string,"Run '",16);
    strncpy(string+5,r,8);
    if(string[strlen(string)-1]=='\'')
       string[strlen(string)-1]=0;
    if(strlen(r)>16)
       strcat(string," ...'");
    else
       strcat(string,"'");
 
    XtVaSetValues(sme_run,XtNlabel,string,NULL);
    XtSetSensitive(sme_run,True);
   }
 else
   {
    XtVaSetValues(sme_run,XtNlabel,"Run",NULL);
    XtSetSensitive(sme_run,False);
   }

 /* Set up the properties window. */

 XtVaSetValues(prop_modname,XtNlabel,(*modulep)->module->name,NULL);
 XtVaSetValues(prop_moddesc,XtNstring,(*modulep)->module->description,NULL);

 XtVaSetValues(prop_outname,XtNlabel,(*outputp)->output->name,NULL);
 XtVaSetValues(prop_outdesc,XtNstring,(*outputp)->output->description,NULL);

 if((*outputp)->type==PROCMETER_GRAPH)
    XtVaSetValues(prop_type,XtNlabel,"Graph",NULL);
 else
    XtVaSetValues(prop_type,XtNlabel,"Text",NULL);

 if((*outputp)->output->interval)
    sprintf(string,"%d s",(*outputp)->output->interval);
 else
    strcpy(string,"Never");
 XtVaSetValues(prop_interval,XtNlabel,string,NULL);

 if((*outputp)->type==PROCMETER_GRAPH)
   {
    if(*((*outputp)->output->graph_units)=='(')
       strcpy(string,(*outputp)->output->graph_units+1);
    else
       strcpy(string,(*outputp)->output->graph_units);
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
    PopupAMenu(functions_menu,w,event);
}


/*++++++++++++++++++++++++++++++++++++++
  Popup a menu, a replacement for the main part of the XawPositionSimpleMenu() and MenuPopup() translations.

  Widget menu The menu to popup.

  Widget w The widget causing the event.

  XEvent *event The event that causes the popup.
  ++++++++++++++++++++++++++++++++++++++*/

static void PopupAMenu(Widget menu,Widget w,XEvent *event)
{
 Position root_x,root_y;
 Dimension width,height;

 XtRealizeWidget(menu);

 XtVaGetValues(menu,XtNwidth,&width,XtNheight,&height,NULL);
 XtTranslateCoords(w,event->xbutton.x,event->xbutton.y,&root_x,&root_y);

 root_y-=5;
 if((DisplayHeight(display,DefaultScreen(display))-height-2)<root_y)
    root_y=DisplayHeight(display,DefaultScreen(display))-height-2;
 if(root_y<2)
    root_y=2;

 root_x-=width/2;
 if((DisplayWidth(display,DefaultScreen(display))-width-2)<root_x)
    root_x=DisplayWidth(display,DefaultScreen(display))-width-2;
 if(root_x<2)
    root_x=2;

 XtVaSetValues(menu,XtNx,root_x,XtNy,root_y,NULL);
 XtPopupSpringLoaded(menu);
 XtGrabPointer(menu,True,ButtonReleaseMask|ButtonPressMask,GrabModeAsync,GrabModeAsync,None,None,CurrentTime);
}
