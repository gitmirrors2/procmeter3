/***************************************
  $Header: /home/amb/CVS/procmeter3/log/window.c,v 1.1 2002-06-04 12:49:15 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3b.

  No X Windows interface.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1997,98,99,2000,02 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>

#include "procmeter.h"
#include "procmeterp.h"

/* Local functions */

static void output_labels(void);
static void output_data(time_t now);

/*+ A list of the outputs that are currently visible. +*/
static Output *displayed=NULL;
static int ndisplayed=0;
static short *new_data=NULL;
static short *graph_data=NULL;
static char **text_data=NULL;

/*+ A flag to indicate that we are still initialising and not to output anything. +*/
static int initialising=1;


/*++++++++++++++++++++++++++++++++++++++
  Start the No X-Windows part.

  int *argc The number of command line arguments.

  char **argv The actual command line arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void Start(int *argc,char **argv)
{

 /* Do nothing since we have no X windows. */

}


/*++++++++++++++++++++++++++++++++++++++
  Stop the No X-Windows part.
  ++++++++++++++++++++++++++++++++++++++*/

void Stop(void)
{

 /* Do nothing since we have no X windows. */

}


/*++++++++++++++++++++++++++++++++++++++
  Sleep for the specified interval in seconds.

  time_t until The time to sleep until.
  ++++++++++++++++++++++++++++++++++++++*/

void Sleep(time_t until)
{
 struct timeval now;
 struct timespec delay;

 /* Before we sleep the first time, output the labels. */

 if(initialising)
   {
    output_labels();
    initialising=0;
   }
 else if(!displayed[0]->first)
    output_data(until);

 /* Sleep */

 gettimeofday(&now,NULL);

 delay.tv_sec=until-now.tv_sec-1;
 delay.tv_nsec=1000*(1000000-now.tv_usec);

 if(delay.tv_sec>=0)
   {
    nanosleep(&delay,NULL);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Add or remove an output

  Output output The output to be added or removed.
  ++++++++++++++++++++++++++++++++++++++*/

void AddRemoveOutput(Output output)
{
 if(output->output_widget)
   {
    int i,found=0;

    output->output_widget=NULL;

    for(i=0;i<ndisplayed;i++)
       if(displayed[i]==output)
          found=1;
       else if(found)
          displayed[i-1]=displayed[i];
    ndisplayed--;
   }
 else
   {
    output->output_widget=(void*)1;
    output->first=2;

    displayed=(Output*)realloc((void*)displayed,sizeof(Output)*(ndisplayed+1));
    displayed[ndisplayed]=output;
    ndisplayed++;

    new_data=(short*)realloc((void*)new_data,sizeof(short)*ndisplayed);
    graph_data=(short*)realloc((void*)graph_data,sizeof(short)*ndisplayed);
    text_data=(char**)realloc((void*)text_data,sizeof(char*)*ndisplayed);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Update a graph output.

  Output output The output to update.

  short value The new value.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateGraph(Output output,short value)
{
 int i;

 for(i=0;i<ndisplayed;i++)
    if(displayed[i]==output)
      {
       new_data[i]=1;
       graph_data[i]=value;
       break;
      }
}


/*++++++++++++++++++++++++++++++++++++++
  Update a text output.

  Output output The output to update.

  char *value The new value.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateText(Output output,char *value)
{
 int i;

 for(i=0;i<ndisplayed;i++)
    if(displayed[i]==output)
      {
       new_data[i]=2;
       text_data[i]=value;
       break;
      }
}


/*++++++++++++++++++++++++++++++++++++++
  Update a bar output.

  Output output The output to update.

  short value The new value.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateBar(Output output,short value)
{
 int i;

 for(i=0;i<ndisplayed;i++)
    if(displayed[i]==output)
      {
       new_data[i]=1;
       graph_data[i]=value;
       break;
      }
}


/*++++++++++++++++++++++++++++++++++++++
  Output the labels for the selected outputs.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_labels(void)
{
 int i;

 printf("UNIX Time");

 for(i=0;i<ndisplayed;i++)
   {
    Output *outputp=NULL;
    Module *modulep,module=NULL;

    for(modulep=Modules;*modulep;modulep++)
      {
       for(outputp=(*modulep)->outputs;*outputp;outputp++)
          if(displayed[i]==*outputp)
            {
             module=*modulep;
             break;
            }
       if(module)
          break;
      }

    printf("\t");

    printf("%s.%s",module->module->name,displayed[i]->output->name);
   }

 printf("\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for the selected outputs.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_data(time_t now)
{
 int i;

 printf("%ld",(long)now);

 for(i=0;i<ndisplayed;i++)
   {
    printf("\t");

    if(new_data[i]==1)
       printf("%d",graph_data[i]);
    else if(new_data[i]==2)
       printf("%s",text_data[i]);

    new_data[i]=0;
   }

 printf("\n");
}
