/***************************************
  $Header: /home/amb/CVS/procmeter3/procmeter.c,v 1.11 2002-12-07 19:32:00 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4.

  Main program.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2000,01,02 Andrew M. Bishop
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
      {
       for(argc--;i<argc;i++)
          argv[i]=argv[i+1];
       help=1;
      }

 /* Initialise things */

 LoadProcMeterRC(&argc,argv);

 if(!help)
    Start(&argc,argv);

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
          expected time is more than 5 seconds ahead or behind the current time. */

       now2=time(NULL);            /* Should be now+interval if there is no jump. */

       if(now2>(now+4) || now2<(now-4))
          now=now2+1;
       else
          now=now+1;

       /* Wait for a while */

       Sleep(now);

       /* Update the outputs */

       UpdateOutputs(now);
      }
    while(!quit);
   }
 else
   {
    static char double_underline[]="===============================";
    static char underline[]="-------------------------------";
    Module *modulep;
    Output *outputp;

    printf("\nProcMeter Version %s\n%s\n\n",PROCMETER_VERSION,&double_underline[sizeof(double_underline)-18-sizeof(PROCMETER_VERSION)]);
    printf("An efficient modular system monitoring program for Linux.\n");
    printf("(c) Andrew M. Bishop 1998,99,2000,01,02 [amb@gedanken.demon.co.uk]\n\n");

    printf("Usage: ProcMeter [-h] [--rc=<filename>] [--...] [...]\n\n");

    printf("To specify the displayed outputs use <module>.<output>[-g|-t|-b] where module\n"
           "and output come from the list below and '-g', '-t' and '-b' choose graph, text\n"
           "or bar format.   e.g. procmeter3 Statistics.CPU-g Processes.Load-t\n");

    for(modulep=Modules;*modulep;modulep++)
      {
       ProcMeterOutput *last=NULL;
       char *p=(*modulep)->module->description;

       printf("\n\n%s\n%s\n\n",(*modulep)->module->name,&underline[sizeof(underline)-1-strlen((*modulep)->module->name)]);

       while(p)
          printf("%s\n",get_substring(&p,80));

       if(*(*modulep)->outputs)
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
                   printf("%-*s (%c%c%c) : ",PROCMETER_NAME_LEN,(*outputp)->output->name,
                          (*outputp)->output->type&PROCMETER_GRAPH?'G':' ',
                          (*outputp)->output->type&PROCMETER_TEXT?'T':' ',
                          (*outputp)->output->type&PROCMETER_BAR?'B':' ');
                else
                   printf("%-*s         ",PROCMETER_NAME_LEN," ");

                printf("%s\n",get_substring(&p,80-PROCMETER_NAME_LEN-9));

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
    Stop();

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform the updates.

  time_t now The current time.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateOutputs(time_t now)
{
 Module *module;
 Output *output;
 ProcMeterOutput *last=NULL;

 for(module=Modules;*module;module++)
    for(output=(*module)->outputs;*output;output++)
       if((*output)->output_widget &&
          (((*output)->output->interval && !(now%(*output)->output->interval)) ||
          (*output)->first))
         {
          if(last!=(*output)->output)
             if((*module)->Update(now,(*output)->output)==-1)
                fprintf(stderr,"ProcMeter: Error updating %s.%s\n",(*module)->module->name,(*output)->output->name);

          if((*output)->first)
             (*output)->first--;

          if(!(*output)->first)
            {
             if((*output)->type==PROCMETER_GRAPH)
               {
                long value=(*output)->output->graph_value;
                if(value<0)
                   value=0;
                if(value>65535)
                   value=65535;
                UpdateGraph(*output,value);
               }
             else if((*output)->type==PROCMETER_TEXT)
                UpdateText(*output,(*output)->output->text_value);
             else if((*output)->type==PROCMETER_BAR)
               {
                long value=(*output)->output->graph_value;
                if(value<0)
                   value=0;
                if(value>65535)
                   value=65535;
                UpdateBar(*output,value);
               }
            }

          last=(*output)->output;
         }
}


/*++++++++++++++++++++++++++++++++++++++
  Add the default outputs at startup.

  int argc The number of command line arguments.

  char **argv The command line arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void AddDefaultOutputs(int argc,char **argv)
{
 Output *outputp=NULL;
 Module *modulep=NULL;
 char *string;
 int arg;

 if((string=GetProcMeterRC("startup","order")))
   {
    char *s=string;

    while(*s && *s==' ')
       s++;

    while(*s)
      {
       int found=0;

       for(modulep=Modules;*modulep;modulep++)
         {
          if(!strncmp((*modulep)->module->name,s,strlen((*modulep)->module->name)) &&
             s[strlen((*modulep)->module->name)]=='.')
            {
             for(outputp=(*modulep)->outputs;*outputp;outputp++)
                if(!strncmp((*outputp)->output->name,&s[strlen((*modulep)->module->name)+1],strlen((*outputp)->output->name)) &&
                   (s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]=='-' ||
                    s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]==' ' ||
                    s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]==0))
                  {
                   if((s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]==' ' ||
                       s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]==0) &&
                      !(*outputp)->output_widget)
                      AddRemoveOutput(*outputp);
                   else if(s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='g' &&
                           (*outputp)->type==PROCMETER_GRAPH &&
                           !(*outputp)->output_widget)
                      AddRemoveOutput(*outputp);
                   else if(s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='t' &&
                           (*outputp)->type==PROCMETER_TEXT &&
                           !(*outputp)->output_widget)
                      AddRemoveOutput(*outputp);
                   else if(s[strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='b' &&
                           (*outputp)->type==PROCMETER_BAR &&
                           !(*outputp)->output_widget)
                      AddRemoveOutput(*outputp);
                   found=1;
                  }

             if(found)
                break;
            }
         }

       while(*s && *s!=' ')
          s++;
       while(*s && *s==' ')
          s++;
      }
   }

 for(arg=1;arg<argc;arg++)
   {
    int found=0;

    for(modulep=Modules;*modulep;modulep++)
       if(!strncmp((*modulep)->module->name,argv[arg],strlen((*modulep)->module->name)) &&
          argv[arg][strlen((*modulep)->module->name)]=='.')
         {
          for(outputp=(*modulep)->outputs;*outputp;outputp++)
             if(!strncmp((*outputp)->output->name,&argv[arg][strlen((*modulep)->module->name)+1],strlen((*outputp)->output->name)) &&
                (argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]=='-' ||
                 argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1]==0))
               {
                if(!argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+1] &&
                   !(*outputp)->output_widget)
                   AddRemoveOutput(*outputp);
                else if(argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='g' &&
                        (*outputp)->type==PROCMETER_GRAPH &&
                        !(*outputp)->output_widget)
                   AddRemoveOutput(*outputp);
                else if(argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='t' &&
                        (*outputp)->type==PROCMETER_TEXT &&
                        !(*outputp)->output_widget)
                   AddRemoveOutput(*outputp);
                else if(argv[arg][strlen((*modulep)->module->name)+strlen((*outputp)->output->name)+2]=='b' &&
                        (*outputp)->type==PROCMETER_BAR &&
                        !(*outputp)->output_widget)
                   AddRemoveOutput(*outputp);
                found=1;
               }

          if(found)
             break;
         }

    if(!*modulep)
       fprintf(stderr,"ProcMeter: Unrecognised output '%s'\n",argv[arg]);
   }
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
