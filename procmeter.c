/***************************************
  $Header: /home/amb/CVS/procmeter3/procmeter.c,v 1.1 1998-09-19 15:19:41 amb Exp $

  ProcMeter - A system monitoring program for Linux.

  Main program.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "procmeter.h"
#include "procmeterp.h"


static void sigexit(int signum);

/*+ The signal to tell us to exit. +*/
int quit=0;

/*+ Indicates that help mode is wanted. +*/
static int help=0;


int main(int argc,char **argv)
{
 time_t now,now2;
 struct sigaction action;
 int i;

 /* Handle signals */

 /* SIGINT, SIGQUIT, SIGTERM */
 action.sa_handler = sigexit;
 sigemptyset(&action.sa_mask);
 sigaddset(&action.sa_mask, SIGINT);           /* Block all of them */
 sigaddset(&action.sa_mask, SIGQUIT);
 sigaddset(&action.sa_mask, SIGTERM);
 action.sa_flags = 0;
 if(sigaction(SIGINT, &action, NULL) != 0)
    fprintf(stderr,"ProcMeter: Cannot install SIGINT handler.\n");
 if(sigaction(SIGQUIT, &action, NULL) != 0)
    fprintf(stderr,"ProcMeter: Cannot install SIGQUIT handler.\n");
 if(sigaction(SIGTERM, &action, NULL) != 0)
    fprintf(stderr,"ProcMeter: Cannot install SIGTERM handler.\n");

 /* Parse the command line. */

 for(i=1;i<argc;i++)
    if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help"))
       help=1;

 /* Initialise things */

 LoadProcMeterRC();

 if(!help)
    StartX(&argc,argv);

 LoadAllModules();

 if(!help)
    AddDefaultOutputs(argc,argv);

 if(!help)
   {
    /* The main loop */

    now=time(NULL);

    do
      {
       /* handle time leaps (rdate, xntpd, or laptop sleep/hibernation) */
       /* We choose to be somewhat lenient and decide that one occured if the
          expected time is more than 5 intervals ahead or behind the current time. */

       now2=time(NULL);            /* Should be now+interval if there is no jump. */

       if(now2>(now+6) || now2<(now-4))
          now=now2+1;
       else
          now=now+1;

       /* Wait for a while */

       SleepX(now);

       /* Update the outputs */

       UpdateX(now);
      }
    while(!quit);
   }
 else
   {
    static char underline[16]="---------------";
    Module *modulep;
    Output *outputp;

    printf("\nProcMeter Version %s\n\n",PROCMETER_VERSION);
    printf("A system monitoring program for Linux.\n");
    printf("(c) Andrew Bishop 1998. [amb@gedanken.demon.co.uk]\n\n");

    printf("Usage: ProcMeter [-h] ...\n\n");

    printf("To specify the default output use <module>.<output>[-g|-t] where module\n"
           "and output come from the list below and '-g' and '-t' choose graph or text\n"
           "e.g. procmeter3 Statistics.CPU-g Processes.Load-t\n");

    for(modulep=Modules;*modulep;modulep++)
      {
       ProcMeterOutput *last=NULL;

       printf("\n\n%s\n%s\n\n",(*modulep)->module->name,&underline[15-strlen((*modulep)->module->name)]);
       printf("%s\n\n",(*modulep)->module->description);

       for(outputp=(*modulep)->outputs;*outputp;outputp++)
         {
          if(last!=(*outputp)->output)
             printf("%-16s : %s\n",(*outputp)->output->name,(*outputp)->output->description);
          last=(*outputp)->output;
         }
      }
   }

 /* Tidy up and exit. */

 UnloadAllModules();

 if(!help)
    StopX();

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The signal handler for the signals to tell us to exit.

  int signum The signal number.
  ++++++++++++++++++++++++++++++++++++++*/

static void sigexit(int signum)
{
 quit=1;
}
