/***************************************
  $Header: /home/amb/CVS/procmeter3/xaw/run.c,v 1.4 2000-12-16 16:50:29 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3.

  Run external programs.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1999,2000 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include "procmeterp.h"


/*+ The display that we are using. +*/
extern Display *display;


/*++++++++++++++++++++++++++++++++++++++
  Parse the command from the config file to be run.

  char *string The string to be parsed.

  RunOption *run Contains the parsed result.
  ++++++++++++++++++++++++++++++++++++++*/

void ParseRunCommand(char *string,RunOption *run)
{
 int offset;
 char *l,*r;

 run->flag=RUN_NONE;
 run->command=NULL;

 if(!string)
    return;

 if(!strncmp("XBeep",string,5))
   {offset=5; run->flag=RUN_XBELL;}
 else if(!strncmp("Shell",string,5))
   {offset=5; run->flag=RUN_SHELL;}
 else if(!strncmp("XTermWait",string,9))
   {offset=9; run->flag=RUN_XTERM_WAIT;}
 else if(!strncmp("XTerm",string,5))
   {offset=5; run->flag=RUN_XTERM;}
 else
   {offset=0; run->flag=RUN_SHELL;}

 l=string+offset;
 r=string+strlen(string)-1;

 while(isspace(*l))
    l++;
 if(offset && *l!='(')
   {
    fprintf(stderr,"ProcMeter3: Cannot parse run command '%s'\n",string);
    run->flag=RUN_NONE;
    return;
   }       
 else if(offset)
    l++;
 while(isspace(*l))
    l++;

 while(isspace(*r))
    r--;
 if(offset && *r!=')')
   {
    fprintf(stderr,"ProcMeter3: Cannot parse run command '%s'\n",string);
    run->flag=RUN_NONE;
    return;
   }       
 else if(offset)
    r--;
 while(isspace(*r))
    r--;

 if(r<l)
    return;

 run->command=(char*)malloc(r-l+2);
 strncpy(run->command,l,r-l+1);
 *(run->command+(r-l)+1)=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Run a program.

  RunOption *run The information about the program to run.
  ++++++++++++++++++++++++++++++++++++++*/

void RunProgram(RunOption *run)
{
 if(run->flag==RUN_NONE || !run->command)
    return;

 if(run->flag==RUN_XBELL)
    XBell(display,0);
 else
   {
    pid_t pid=fork();

    if(pid==-1)
       fprintf(stderr,"ProcMeter3: Cannot fork child process %s\n",strerror(errno));
    else if(pid==0)
      {
       char *string,*displayname,*displayenv;

       displayname=XDisplayString(display);
       displayenv=(char*)malloc(strlen(displayname)+10);
       sprintf(displayenv,"DISPLAY=%s",displayname);
       putenv(displayenv);

       /* close the X connection */

       close(ConnectionNumber(display));

       switch(run->flag)
         {
         default:
         case RUN_SHELL:
          execl("/bin/sh","/bin/sh","-c",run->command,NULL);
          break;

         case RUN_XTERM:
          execlp("xterm","xterm","-title","ProcMeter3","-e","/bin/sh","-c",run->command,NULL);
          break;

         case RUN_XTERM_WAIT:
          string=(char*)malloc(strlen(run->command)+64);
          sprintf(string,"( %s ) ; echo -n 'Press Return to exit' ; read x",run->command);
          execlp("xterm","xterm","-title","ProcMeter3","-e","/bin/sh","-c",string,NULL);
          break;
         }

       /* Never reached */

       exit(1);
      }
   }
}
