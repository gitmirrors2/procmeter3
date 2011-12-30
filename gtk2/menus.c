/***************************************
  ProcMeter - A system monitoring program for Linux - Version 3.6.

  X Window menus (GTK version).
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
#include <unistd.h>

#include <gtk/gtk.h>

#include "window.h"
#include "procmeterp.h"


/*+ The orientation of the windows. +*/
extern int vertical;

/*+ The pane that contains all of the outputs. +*/
extern GtkWidget *pane;

/* Local functions */

static void MenuStart(GtkWidget *w,GdkEvent *event,gpointer data);
static void ModuleMenuStart(GtkWidget *w,GdkEvent *event,gpointer data);
static void OutputMenuStart(GtkWidget *w,GdkEvent *event,gpointer data);
static void FunctionsMenuStart(GtkWidget *w,GdkEvent *event,gpointer data);

static void SelectOutputMenuCallback(gpointer clientData);
static void SelectFunctionsMenuCallback(gpointer clientData);

static void PropertiesDialogDoneCallback(GtkWidget *w,gpointer data);
static gint PropertiesDialogCloseCallback(GtkWidget *w,GdkEvent *event,gpointer data);

/* Menu widgets */

static GtkWidget *module_menu;
static GtkWidget *functions_menu,*properties_dialog;
static GtkWidget *func_run;
static GtkWidget *prop_modname,*prop_moddesc,
                 *prop_outname,*prop_outdesc,
                 *prop_label,*prop_type,*prop_interval,*prop_scale;
static GtkTextBuffer *prop_moddesc_text,*prop_outdesc_text;
static gboolean properties_popped_up=FALSE;
static gint doing_move=0;


/*+ The output that was used for the Functions menu. +*/
static Output function_output;


/*++++++++++++++++++++++++++++++++++++++
  Initialise the menus.

  GtkWidget *parent The parent widget that the menu belongs to.
  ++++++++++++++++++++++++++++++++++++++*/

void CreateMenus(GtkWidget *parent)
{
 GtkWidget *menulabel;
 GtkWidget *menuline;
 GtkWidget *func_prop,*func_above,*func_below,*func_delete;
 GtkWidget *prop_form,*prop_form1,*prop_form2,
           *prop_modlabel,*prop_outlabel,
           *prop_lbllabel,*prop_typlabel,*prop_intlabel,*prop_scllabel,
           *prop_done;
 // char *string;

 /* Sort out the resources in advance. */
 
 //  if((string=GetProcMeterRC("resources","menu-foreground")))
 //    {XtSetArg(args[nargs],XtNforeground,StringToPixel(string));nargs++;}
 // 
 //  if((string=GetProcMeterRC("resources","menu-background")))
 //    {XtSetArg(args[nargs],XtNbackground,StringToPixel(string));nargs++;}
 // 
 //  if((string=GetProcMeterRC("resources","menu-font")))
 //    {XtSetArg(args[nargs],XtNfont,StringToFont(string));nargs++;}

 /* The module menu */

 module_menu=gtk_menu_new();

 menulabel=gtk_menu_item_new_with_label("Modules");
 gtk_widget_set_sensitive(GTK_WIDGET(menulabel),FALSE);
 gtk_menu_append(GTK_MENU(module_menu),menulabel);
 gtk_widget_show(GTK_WIDGET(menulabel));

 menuline=gtk_menu_item_new();
 gtk_menu_append(GTK_MENU(module_menu),menuline);
 gtk_widget_show(GTK_WIDGET(menuline));

 /* The functions menu */

 functions_menu=gtk_menu_new();

 menulabel=gtk_menu_item_new_with_label("Functions");
 gtk_widget_set_sensitive(GTK_WIDGET(menulabel),FALSE);
 gtk_menu_append(GTK_MENU(functions_menu),menulabel);
 gtk_widget_show(GTK_WIDGET(menulabel));

 menuline=gtk_menu_item_new();
 gtk_menu_append(GTK_MENU(functions_menu),menuline);
 gtk_widget_show(GTK_WIDGET(menuline));

 /* The functions menu items */

 func_prop=gtk_menu_item_new_with_label("Properties");
 gtk_menu_append(GTK_MENU(functions_menu),func_prop);
 gtk_widget_show(GTK_WIDGET(func_prop));

 gtk_signal_connect_object(GTK_OBJECT(func_prop),"activate",
                           GTK_SIGNAL_FUNC(SelectFunctionsMenuCallback),(gpointer)0);

 func_above=gtk_menu_item_new_with_label(vertical?"Move To Above":"Move To Left Of");
 gtk_menu_append(GTK_MENU(functions_menu),func_above);
 gtk_widget_show(GTK_WIDGET(func_above));

 gtk_signal_connect_object(GTK_OBJECT(func_above),"activate",
                           GTK_SIGNAL_FUNC(SelectFunctionsMenuCallback),(gpointer)1);

 func_below=gtk_menu_item_new_with_label(vertical?"Move To Below":"Move To Right Of");
 gtk_menu_append(GTK_MENU(functions_menu),func_below);
 gtk_widget_show(GTK_WIDGET(func_below));

 gtk_signal_connect_object(GTK_OBJECT(func_below),"activate",
                           GTK_SIGNAL_FUNC(SelectFunctionsMenuCallback),(gpointer)2);

 func_delete=gtk_menu_item_new_with_label("Delete");
 gtk_menu_append(GTK_MENU(functions_menu),func_delete);
 gtk_widget_show(GTK_WIDGET(func_delete));

 gtk_signal_connect_object(GTK_OBJECT(func_delete),"activate",
                           GTK_SIGNAL_FUNC(SelectFunctionsMenuCallback),(gpointer)3);

 func_run=gtk_menu_item_new_with_label("Run");
 gtk_menu_append(GTK_MENU(functions_menu),func_run);
 gtk_widget_show(GTK_WIDGET(func_run));

 gtk_signal_connect_object(GTK_OBJECT(func_run),"activate",
                           GTK_SIGNAL_FUNC(SelectFunctionsMenuCallback),(gpointer)4);

 /* The properties_dialog */

 properties_dialog=gtk_dialog_new();
 gtk_window_set_title(GTK_WINDOW(properties_dialog),"ProcMeter Properties");
 gtk_widget_set_usize(GTK_WIDGET(properties_dialog),300,400);

 /* The properties dialog elements. */

 prop_form=gtk_hbox_new(FALSE,0);

 prop_modlabel=gtk_label_new("Module: ");
 gtk_label_set_justify(GTK_LABEL(prop_modlabel),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form),prop_modlabel,FALSE,FALSE,0);
 gtk_widget_show(GTK_WIDGET(prop_modlabel));

 prop_modname=gtk_label_new("NNNNNNNNNNNNNNN");
 gtk_label_set_justify(GTK_LABEL(prop_modname),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form),prop_modname,FALSE,FALSE,0);
 gtk_widget_show(GTK_WIDGET(prop_modname));

 gtk_box_pack_start(GTK_BOX(GTK_DIALOG(properties_dialog)->vbox),prop_form,FALSE,FALSE,0);
 gtk_widget_show(GTK_WIDGET(prop_form));

 prop_moddesc=gtk_text_view_new();
 gtk_text_view_set_editable(GTK_TEXT_VIEW(prop_moddesc),FALSE);
 gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(prop_moddesc),GTK_WRAP_WORD);
 prop_moddesc_text=gtk_text_buffer_new(NULL);
 gtk_text_view_set_buffer(GTK_TEXT_VIEW(prop_moddesc),GTK_TEXT_BUFFER(prop_moddesc_text));
 gtk_box_pack_start(GTK_BOX(GTK_DIALOG(properties_dialog)->vbox),prop_moddesc,FALSE,FALSE,0);
 gtk_widget_set_usize(GTK_WIDGET(prop_moddesc),250,100);
 gtk_widget_show(GTK_WIDGET(prop_moddesc));

 prop_form=gtk_hbox_new(FALSE,0);

 prop_outlabel=gtk_label_new("Output: ");
 gtk_label_set_justify(GTK_LABEL(prop_outlabel),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form),prop_outlabel,FALSE,FALSE,0);
 gtk_widget_show(GTK_WIDGET(prop_outlabel));

 prop_outname=gtk_label_new("NNNNNNNNNNNNNNN");
 gtk_label_set_justify(GTK_LABEL(prop_outname),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form),prop_outname,FALSE,FALSE,0);
 gtk_widget_show(GTK_WIDGET(prop_outname));

 gtk_box_pack_start(GTK_BOX(GTK_DIALOG(properties_dialog)->vbox),prop_form,FALSE,FALSE,0);
 gtk_widget_show(GTK_WIDGET(prop_form));

 prop_outdesc=gtk_text_view_new();
 gtk_text_view_set_editable(GTK_TEXT_VIEW(prop_outdesc),FALSE);
 gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(prop_outdesc),GTK_WRAP_WORD);
 prop_outdesc_text=gtk_text_buffer_new(NULL);
 gtk_text_view_set_buffer(GTK_TEXT_VIEW(prop_outdesc),GTK_TEXT_BUFFER(prop_outdesc_text));
 gtk_box_pack_start(GTK_BOX(GTK_DIALOG(properties_dialog)->vbox),prop_outdesc,FALSE,FALSE,0);
 gtk_widget_set_usize(GTK_WIDGET(prop_outdesc),250,100);
 gtk_widget_show(GTK_WIDGET(prop_outdesc));

 prop_form=gtk_hbox_new(FALSE,0);

 prop_form1=gtk_vbox_new(TRUE,0);

 prop_lbllabel=gtk_label_new("Label:");
 gtk_label_set_justify(GTK_LABEL(prop_lbllabel),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form1),prop_lbllabel,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_lbllabel));

 prop_typlabel=gtk_label_new("Type:");
 gtk_label_set_justify(GTK_LABEL(prop_typlabel),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form1),prop_typlabel,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_typlabel));

 prop_intlabel=gtk_label_new("Interval:");
 gtk_label_set_justify(GTK_LABEL(prop_intlabel),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form1),prop_intlabel,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_intlabel));

 prop_scllabel=gtk_label_new("Grid Spacing:");
 gtk_label_set_justify(GTK_LABEL(prop_scllabel),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form1),prop_scllabel,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_scllabel));

 gtk_box_pack_start(GTK_BOX(prop_form),prop_form1,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_form1));

 prop_form2=gtk_vbox_new(TRUE,0);

 prop_label=gtk_label_new("NNNNNNNNNNNNNNN");
 gtk_label_set_justify(GTK_LABEL(prop_label),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form2),prop_label,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_label));

 prop_type=gtk_label_new("NNNNNNN");
 gtk_label_set_justify(GTK_LABEL(prop_type),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form2),prop_type,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_type));

 prop_interval=gtk_label_new("NNNNNNN");
 gtk_label_set_justify(GTK_LABEL(prop_interval),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form2),prop_interval,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_interval));

 prop_scale=gtk_label_new("NNNNNNN");
 gtk_label_set_justify(GTK_LABEL(prop_scale),GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(prop_form2),prop_scale,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_scale));

 gtk_box_pack_start(GTK_BOX(prop_form),prop_form2,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_form2));

 gtk_box_pack_start(GTK_BOX(GTK_DIALOG(properties_dialog)->vbox),prop_form,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_form));

 prop_done=gtk_button_new_with_label("Done");
 gtk_box_pack_start(GTK_BOX(GTK_DIALOG(properties_dialog)->action_area),prop_done,TRUE,TRUE,0);
 gtk_widget_show(GTK_WIDGET(prop_done));

 gtk_signal_connect_object(GTK_OBJECT(prop_done),"clicked",
                           GTK_SIGNAL_FUNC(PropertiesDialogDoneCallback),NULL);

 /* Fixup the properties dialog box */

 gtk_signal_connect_object(GTK_OBJECT(properties_dialog),"delete_event",
                           GTK_SIGNAL_FUNC(PropertiesDialogCloseCallback),NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a specified module to the menus.

  Module module The module to add.
  ++++++++++++++++++++++++++++++++++++++*/

void AddModuleToMenu(Module module)
{
 int i;
 GtkWidget *menulabel,*menuline,*submenu=NULL,*menuitem=NULL;
 ProcMeterOutput *prevoutput=NULL;
 char *string;

 /* Return if pane widget is not initialised (e.g. gprocmeter3 -h). */

 if(!pane)
    return;

 /* Sort out the resources in advance. */

 // if(((string=GetProcMeterRC(module->module->name,"menu-foreground")) ||
 //     (string=GetProcMeterRC("resources","menu-foreground"))))
 //   {XtSetArg(args[nargs],XtNforeground,StringToPixel(string));nargs++;}
 //
 // if(((string=GetProcMeterRC(module->module->name,"menu-background")) ||
 //     (string=GetProcMeterRC("resources","menu-background"))))
 //   {XtSetArg(args[nargs],XtNbackground,StringToPixel(string));nargs++;}
 //
 // if(((string=GetProcMeterRC(module->module->name,"menu-font")) ||
 //     (string=GetProcMeterRC("resources","menu-font"))))
 //   {XtSetArg(args[nargs],XtNfont,StringToFont(string));nargs++;}

 /* Create a new menu. */

 module->submenu_widget=gtk_menu_new();

 menulabel=gtk_menu_item_new_with_label(module->module->name);
 gtk_widget_set_sensitive(GTK_WIDGET(menulabel),FALSE);
 gtk_menu_append(GTK_MENU(module->submenu_widget),menulabel);
 gtk_widget_show(GTK_WIDGET(menulabel));

 menuline=gtk_menu_item_new();
 gtk_menu_append(GTK_MENU(module->submenu_widget),menuline);
 gtk_widget_show(GTK_WIDGET(menuline));

 /* Add an entry to the module menu */

 module->menu_item_widget=gtk_menu_item_new_with_label(module->module->name);

 gtk_menu_append(GTK_MENU(module_menu),module->menu_item_widget);
 gtk_widget_show(GTK_WIDGET(module->menu_item_widget));
 gtk_menu_item_set_submenu(GTK_MENU_ITEM(module->menu_item_widget),module->submenu_widget);

 /* Add entries to it for each output. */

 for(i=0;module->outputs[i];i++)
   {
    GtkWidget *sme;

    if(module->outputs[i]->output!=prevoutput)
      {
       menuitem=gtk_menu_item_new_with_label(module->outputs[i]->output->name);

       gtk_menu_append(GTK_MENU(module->submenu_widget),menuitem);
       gtk_widget_show(GTK_WIDGET(menuitem));

 //       if(((string=GetProcMeterRC2(module->module->name,module->outputs[i]->output->name,"menu-foreground")) ||
 //           (string=GetProcMeterRC(module->module->name,"menu-foreground")) ||
 //           (string=GetProcMeterRC("resources","menu-foreground"))))
 //          XtVaSetValues(menuitem,XtNforeground,StringToPixel(string),NULL);
 //
 //       if(((string=GetProcMeterRC2(module->module->name,module->outputs[i]->output->name,"menu-font")) ||
 //           (string=GetProcMeterRC(module->module->name,"menu-font")) ||
 //           (string=GetProcMeterRC("resources","menu-font"))))
 //          XtVaSetValues(menuitem,XtNfont,StringToFont(string),NULL);

       submenu=gtk_menu_new();
       gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),submenu);

       prevoutput=module->outputs[i]->output;
      }

    string="";
    if(module->outputs[i]->type==PROCMETER_GRAPH)
       string="Graph";
    else if(module->outputs[i]->type==PROCMETER_TEXT)
       string="Text";
    else if(module->outputs[i]->type==PROCMETER_BAR)
       string="Bar";

    sme=gtk_check_menu_item_new_with_label(string);

    gtk_menu_append(GTK_MENU(submenu),sme);
    gtk_widget_show(GTK_WIDGET(sme));

    gtk_signal_connect_object(GTK_OBJECT(sme),"activate",
                              GTK_SIGNAL_FUNC(SelectOutputMenuCallback),(gpointer)module->outputs[i]);

 //    if(((string=GetProcMeterRC2(module->module->name,module->outputs[i]->output->name,"menu-foreground")) ||
 //        (string=GetProcMeterRC(module->module->name,"menu-foreground")) ||
 //        (string=GetProcMeterRC("resources","menu-foreground"))))
 //       XtVaSetValues(sme,XtNforeground,StringToPixel(string),NULL);

    module->outputs[i]->menu_item_widget=sme;
    module->outputs[i]->output_widget=NULL;
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Add the menus to the right mouse button of the output.

  GtkWidget *widget The widget itself.

  Module module The module that this widget belongs to.
  ++++++++++++++++++++++++++++++++++++++*/

void AddMenuToOutput(GtkWidget *widget,Module module)
{
 /* Return if pane widget is not initialised (e.g. gprocmeter3 -h). */

 if(!pane)
    return;

 gtk_signal_connect(GTK_OBJECT(widget),"button_press_event",
                    GTK_SIGNAL_FUNC(MenuStart),module?module->module->name:NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Remove a specified module from the menus.

  Module module The module to remove.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveModuleFromMenu(Module module)
{
 /* Return if pane widget is not initialised (e.g. gprocmeter3 -h). */

 if(!pane)
    return;

 gtk_widget_destroy(GTK_WIDGET(module->submenu_widget));
 gtk_object_destroy(GTK_OBJECT(module->menu_item_widget));
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy all of the menus.
  ++++++++++++++++++++++++++++++++++++++*/

void DestroyMenus(void)
{
 gtk_widget_destroy(GTK_WIDGET(module_menu));
 gtk_widget_destroy(GTK_WIDGET(functions_menu));
 gtk_widget_destroy(GTK_WIDGET(properties_dialog));
}


/*++++++++++++++++++++++++++++++++++++++
  The callback that is called by the output being selected on the output menu.

  gpointer clientData The client data from the callback.

  This function is only ever called from the GTK event loop.
  ++++++++++++++++++++++++++++++++++++++*/

static void SelectOutputMenuCallback(gpointer clientData)
{
 Output output=(Output)clientData;

 AddRemoveOutput(output);
}


/*++++++++++++++++++++++++++++++++++++++
  The callback that is called by the output being selected on the output menu.

  gpointer clientData The client data from the callback.

  This function is only ever called from the GTK event loop.
  ++++++++++++++++++++++++++++++++++++++*/

static void SelectFunctionsMenuCallback(gpointer clientData)
{
 if(clientData==(gpointer)0)         /* Properties */
   {
    gtk_widget_show(GTK_WIDGET(properties_dialog));

    properties_popped_up=TRUE;
   }
 else if(clientData==(gpointer)1 || clientData==(gpointer)2) /* Above / Below */
   {
    doing_move=(glong)clientData;

    gdk_pointer_grab(GTK_WIDGET(pane)->window,TRUE,GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK,NULL,gdk_cursor_new(GDK_HAND1),GDK_CURRENT_TIME);
   }
 else if(clientData==(gpointer)3)    /* Delete */
   {
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(function_output->menu_item_widget),FALSE);
   }
 else if(clientData==(gpointer)4)    /* Run */
   {
    if(function_output->menu_run.flag)
       RunProgram(&function_output->menu_run);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The callback for the done button on the properties dialog box.

  GtkWidget *w The widget that the callback came from.

  gpointer data The data from the callback.

  This function is only ever called from the GTK event loop.
  ++++++++++++++++++++++++++++++++++++++*/

static void PropertiesDialogDoneCallback(GtkWidget *w,gpointer data)
{
 gtk_widget_hide(GTK_WIDGET(properties_dialog));
 properties_popped_up=FALSE;
}


/*++++++++++++++++++++++++++++++++++++++
  The callback for the window manager close on the dialog.

  GtkWidget *w The widget that caused the event.

  GdkEvent *event The event information.

  gpointer data Not used.

  This function is only ever called from the GTK event loop.
  ++++++++++++++++++++++++++++++++++++++*/

static gint PropertiesDialogCloseCallback(GtkWidget *w,GdkEvent *event,gpointer data)
{
 gtk_widget_hide(GTK_WIDGET(properties_dialog));
 properties_popped_up=FALSE;

 return(TRUE);
}


/*++++++++++++++++++++++++++++++++++++++
  Start one of the popup menus.

  GtkWidget *w The widget that caused it.

  GdkEvent *event The event that caused it.

  gpointer data The name of the module (or NULL for the pane).

  This function is only ever called from the GTK event loop.
  ++++++++++++++++++++++++++++++++++++++*/

static void MenuStart(GtkWidget *w,GdkEvent *event,gpointer data)
{
 if(event->type!=GDK_BUTTON_PRESS)
    return;

 if(event->button.button==1 && data)
    FunctionsMenuStart(w,event,data);
 else if(event->button.button==2 && data)
    OutputMenuStart(w,event,data);
 else if(event->button.button==3)
    ModuleMenuStart(w,event,data);
}


/*++++++++++++++++++++++++++++++++++++++
  Start the Module menu.

  GtkWidget *w The widget that caused it.

  GdkEvent *event The event that caused it.

  gpointer data The name of the module (or NULL for the pane).

  This function is only ever called from the GTK event loop.
  ++++++++++++++++++++++++++++++++++++++*/

static void ModuleMenuStart(GtkWidget *w,GdkEvent *event,gpointer data)
{
 gtk_menu_popup(GTK_MENU(module_menu),NULL,NULL,NULL,NULL,event->button.button,event->button.time);
}


/*++++++++++++++++++++++++++++++++++++++
  Start one of the Output menus.

  GtkWidget *w The widget that caused it.

  GdkEvent *event The event that caused it.

  gpointer data The name of the module (or NULL for the pane).

  This function is only ever called from the GTK event loop.
  ++++++++++++++++++++++++++++++++++++++*/

static void OutputMenuStart(GtkWidget *w,GdkEvent *event,gpointer data)
{
 static Module *modulep=NULL;

 for(modulep=Modules;*modulep;modulep++)
    if(!strcmp((*modulep)->module->name,data))
       gtk_menu_popup(GTK_MENU((*modulep)->submenu_widget),NULL,NULL,NULL,NULL,event->button.button,event->button.time);
}


/*++++++++++++++++++++++++++++++++++++++
  Start the properties menu.

  GtkWidget *w The widget that caused it.

  GdkEvent *event The event that caused it.

  gpointer data The name of the module (or NULL for the pane).

  This function is only ever called from the GTK event loop.
  ++++++++++++++++++++++++++++++++++++++*/

static void FunctionsMenuStart(GtkWidget *w,GdkEvent *event,gpointer data)
{
 static Output *outputp=NULL;
 static Module *modulep=NULL;
 char string[24];

 for(modulep=Modules;*modulep;modulep++)
    if(!strcmp((*modulep)->module->name,data))
      {
       for(outputp=(*modulep)->outputs;*outputp;outputp++)
          if(w==(*outputp)->output_widget)
             break;
       break;
      }

 if(doing_move)
   {
    gdk_pointer_ungrab(GDK_CURRENT_TIME);

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
 
//     XtVaSetValues(func_run,XtNlabel,string,NULL);
    gtk_widget_set_sensitive(GTK_WIDGET(func_run),TRUE);
   }
 else
   {
//     XtVaSetValues(func_run,XtNlabel,"Run",NULL);
    gtk_widget_set_sensitive(GTK_WIDGET(func_run),FALSE);
   }

 /* Set up the properties window. */

 gtk_label_set_text(GTK_LABEL(prop_modname),(*modulep)->module->name);
 gtk_text_buffer_set_text(GTK_TEXT_BUFFER(prop_moddesc_text),
                          (*modulep)->module->description,strlen((*modulep)->module->description));

 gtk_label_set_text(GTK_LABEL(prop_outname),(*outputp)->output->name);
 gtk_text_buffer_set_text(GTK_TEXT_BUFFER(prop_outdesc_text),
                          (*modulep)->module->description,strlen((*modulep)->module->description));

 gtk_label_set_text(GTK_LABEL(prop_label),(*outputp)->label);

 if((*outputp)->type==PROCMETER_GRAPH)
    gtk_label_set_text(GTK_LABEL(prop_type),"Graph");
 else if((*outputp)->type==PROCMETER_TEXT)
    gtk_label_set_text(GTK_LABEL(prop_type),"Text");
 else if((*outputp)->type==PROCMETER_BAR)
    gtk_label_set_text(GTK_LABEL(prop_type),"Bar");

 if((*outputp)->output->interval)
    sprintf(string,"%d s",(*outputp)->output->interval);
 else
    strcpy(string,"Never");
 gtk_label_set_text(GTK_LABEL(prop_interval),string);

 if((*outputp)->type&(PROCMETER_GRAPH|PROCMETER_BAR))
   {
    char str[PROCMETER_UNITS_LEN+1];
    snprintf(str,PROCMETER_UNITS_LEN+1,(*outputp)->output->graph_units,(*outputp)->output->graph_scale);
    if(*str=='(')
       strcpy(string,str+1);
    else
       strcpy(string,str);
    if(string[strlen(string)-1]==')')
       string[strlen(string)-1]=0;

    gtk_label_set_text(GTK_LABEL(prop_scale),string);
   }
 else
   {
    gtk_label_set_text(GTK_LABEL(prop_scale),"n/a");
   }

 if(!properties_popped_up)
    gtk_menu_popup(GTK_MENU(functions_menu),NULL,NULL,NULL,NULL,event->button.button,event->button.time);
}
