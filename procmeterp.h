/***************************************
  $Header: /home/amb/CVS/procmeter3/procmeterp.h,v 1.3 1999-02-07 14:20:26 amb Exp $

  ProcMeter - A system monitoring program for Linux.

  Global private header file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PROCMETERP_H
#define PROCMETERP_H    /*+ To stop multiple inclusions. +*/

/* The file locations. */

#ifndef INSTDIR
#define INSTDIR "/usr/local"
#endif

#define LIB_PATH  INSTDIR  "/lib/X11/ProcMeter3"
#define MOD_PATH  LIB_PATH "/modules"
#define RC_PATH   LIB_PATH "/.procmeterrc"

/* The public header file. */

#include "procmeter.h"


typedef struct _Output *Output;

/*+ The information about a module that is used internally. +*/
typedef struct _Module
{
 char *filename;                /*+ The filename of the module. +*/

 void *dlhandle;                /*+ The handle returned by dlopen(). +*/

 int (*Update)(time_t,ProcMeterOutput*); /*+ The update function for the outputs in the module. +*/

 ProcMeterModule *module;       /*+ The module information returned by the Load() function. +*/
 Output *outputs;               /*+ The outputs based on those returned by the Initialise function. +*/

 void *menu_item_widget;        /*+ The menu item widget for the module on the main menu. +*/
 void *submenu_widget;          /*+ The submenu widget for the module. +*/
}
*Module;

/*+ The information about an output that is used internally. +*/
struct _Output
{
 ProcMeterOutput *output;       /*+ The output that this represents. +*/

 int type;                      /*+ The type of output. +*/

 int first;                     /*+ Set this to true if the widget is new and needs an update. +*/

 void *menu_item_widget;        /*+ The menu item widget for the output on the module menu. +*/
 void *output_widget;           /*+ The widget for the output in the main window. +*/
};


/*+ The complete list of modules. +*/
extern Module* Modules;


/* In module.c */

void LoadAllModules(void);
void UnloadAllModules(void);

Module LoadModule(char* filename);
void UnloadModule(Module module);

/* In procmeterrc.c */

void LoadProcMeterRC(void);
char *GetProcMeterRC(char *section,char *item);
char *GetProcMeterRC2(char *module,char *output,char *item);

/* In xwindow.c */

void StartX(int *argc,char **argv);
void StopX(void);
void SleepX(time_t until);
void UpdateX(time_t now);
void AddDefaultOutputs(int argc,char **argv);
void AddRemoveOutput(Output);
void MoveOutput(Output output1,Output output2,int direction);

/* In xmenus.c */

void AddModuleToMenu(Module module);
void RemoveModuleFromMenu(Module module);

#endif /* PROCMETERP_H */
