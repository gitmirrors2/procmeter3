/***************************************
  $Header: /home/amb/CVS/procmeter3/procmeterp.h,v 1.12 2002-06-04 13:53:39 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3b.

  Global private header file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2002 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#ifndef PROCMETERP_H
#define PROCMETERP_H    /*+ To stop multiple inclusions. +*/

/* The public header file. */

#include "procmeter.h"


/* The file locations. */

#ifndef INSTDIR
#define INSTDIR "/usr/local"
#endif

#ifndef LIB_PATH
#define LIB_PATH  INSTDIR  "/lib/X11/ProcMeter3"
#endif
#ifndef MOD_PATH
#define MOD_PATH  LIB_PATH "/modules"
#endif
#ifndef RC_PATH
#define RC_PATH   LIB_PATH "/.procmeterrc"
#endif

/* The run mode options */

#define RUN_NONE       0
#define RUN_SHELL      1
#define RUN_XTERM      2
#define RUN_XTERM_WAIT 3
#define RUN_XBELL      4

/*+ The information needed to run a command. +*/
typedef struct _RunOption
{
 char flag;                     /*+ The type of command to run. +*/
 char *command;                 /*+ The pre-parsed command string. +*/
}
RunOption;

/* A forward definition for the Output type. */

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

 char label[PROCMETER_NAME_LEN];/*+ The label for the output. +*/

 RunOption menu_run;            /*+ The function that can be run for this output. +*/

 int type;                      /*+ The type of output. +*/

 int first;                     /*+ Set this to true if the widget is new and needs an update. +*/

 void *menu_item_widget;        /*+ The menu item widget for the output on the module menu. +*/
 void *output_widget;           /*+ The widget for the output in the main window. +*/
};


/*+ The complete list of modules. +*/
extern Module* Modules;

/* In procmeter.c */

void UpdateOutputs(time_t now);
void AddDefaultOutputs(int argc,char **argv);

/* In module.c */

void LoadAllModules(void);
void UnloadAllModules(void);

Module LoadModule(char* filename);
void UnloadModule(Module module);

/* In procmeterrc.c */

void LoadProcMeterRC(int *argc,char **argv);

char *GetProcMeterRC(char *section,char *parameter);
char *GetProcMeterRC2(char *module,char *output,char *parameter);

void FreeProcMeterRC(void);

/* In xaw/window.c or gtk/window.c or no-x/window.c */

void Start(int *argc,char **argv);
void Stop(void);

void Sleep(time_t until);

void AddRemoveOutput(Output output);

void UpdateGraph(Output output,short value);
void UpdateText(Output output,char *value);
void UpdateBar(Output output,short value);

/* In xaw/menus.c or gtk/menus.c or no-x/menus.c */

void AddModuleToMenu(Module module);
void RemoveModuleFromMenu(Module module);

/* In xaw/run.c or gtk/run.c or no-x/run.c */

void ParseRunCommand(char *string,RunOption *run);

void RunProgram(RunOption *run);


#endif /* PROCMETERP_H */
