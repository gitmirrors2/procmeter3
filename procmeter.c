/***************************************
  $Header: /home/amb/CVS/procmeter3/procmeter.c,v 1.6 1999-12-04 16:56:45 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.2.

  Main program.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

#include "procmeter.h"
#include "procmeterp.h"


static char *get_substring(char **start,int length);
static void sigexit(int signum);
static void sigchild(int signum);

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
 sigaddset(&action.sa_mask, SIGCHLD);
 action.sa_flags = 0;
 if(sigaction(SIGINT, &action, NULL) != 0)
    fprintf(stderr,"ProcMeter: Cannot install SIGINT handler.\n");
 if(sigaction(SIGQUIT, &action, NULL) != 0)
    fprintf(stderr,"ProcMeter: Cannot install SIGQUIT handler.\n");
 if(sigaction(SIGTERM, &action, NULL) != 0)
    fprintf(stderr,"ProcMeter: Cannot install SIGTERM handler.\n");

 /* SIGCHILD */
 action.sa_handler = sigchild;
 sigemptyset(&action.sa_mask);
 sigaddset(&action.sa_mask, SIGINT);           /* Block all of them */
 sigaddset(&action.sa_mask, SIGQUIT);
 sigaddset(&action.sa_mask, SIGTERM);
 action.sa_flags = 0;
 if(sigaction(SIGCHLD, &action, NULL) != 0)
    fprintf(stderr,"ProcMeter: Cannot install SIGCHLD handler.\n");

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

       if(now2>(now+4) || now2<(now-4))
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
    printf("(c) Andrew M. Bishop 1998,99 [amb@gedanken.demon.co.uk]\n\n");

    printf("Usage: ProcMeter [-h] ...\n\n");

    printf("To specify the default output use <module>.<output>[-g|-t] where module and\n"
           "output come from the list below and '-g', '-t' and '-b' choose graph, text or\n"
           "bar format.\n"
           "e.g. procmeter3 Statistics.CPU-g Processes.Load-t\n");

    for(modulep=Modules;*modulep;modulep++)
      {
       ProcMeterOutput *last=NULL;
       char *p=(*modulep)->module->description;

       printf("\n\n%s\n%s\n\n",(*modulep)->module->name,&underline[15-strlen((*modulep)->module->name)]);

       while(p)
          printf("%s\n",get_substring(&p,80));
       printf("\n");

       for(outputp=(*modulep)->outputs;*outputp;outputp++)
          {
          if(last!=(*outputp)->output)
            {
             char *p=(*outputp)->output->description;
             int first=1;

             while(p)
               {
                if(first)
                   printf("%-16s (%c%c%c) : ",(*outputp)->output->name,
                          (*outputp)->output->type&PROCMETER_GRAPH?'G':' ',
                          (*outputp)->output->type&PROCMETER_TEXT?'T':' ',
                          (*outputp)->output->type&PROCMETER_BAR?'B':' ');
                else
                   printf("                        ");

                printf("%s\n",get_substring(&p,55));

                first=0;
               }
            }
          last=(*outputp)->output;
         }
      }
   }

 /* Tidy up and exit. */

 UnloadAllModules();

 FreeProcMeterRC();

 if(!help)
    StopX();

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Make a substring copy of the specified string.

  char *get_substring Returns a copy.

  char **start The start position in the string.

  int length The length of string to return.
  ++++++++++++++++++++++++++++++++++++++*/

static char *get_substring(char **start,int length)
{
 static char string[80];

 if(strlen(*start)>length)
   {
    char *p=*start+length;

    if(!isspace(*p))
      {
       while(p>*start && !isspace(*p))
          p--;
       if(p==*start)
          p=*start+length;
       else
          p++;
      }

    strncpy(string,*start,(p-*start));
    string[p-*start]=0;

    while(*p==' ')
       p++;
    if(!*p)
       p=NULL;

    *start=p;
   }
 else
   {
    strcpy(string,*start);
    *start=NULL;
   }

 return(string);
}


/*++++++++++++++++++++++++++++++++++++++
  The signal handler for the signals to tell us to exit.

  int signum The signal number.
  ++++++++++++++++++++++++++++++++++++++*/

static void sigexit(int signum)
{
 quit=1;
}


/*++++++++++++++++++++++++++++++++++++++
  The signal handler for the child processes terminating.

  int signum The signal number.
  ++++++++++++++++++++++++++++++++++++++*/

static void sigchild(int signum)
{
 pid_t pid;
 int status;

 while((pid=waitpid(-1,&status,WNOHANG))>0)
    ;
}
