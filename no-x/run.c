/***************************************
  $Header: /home/amb/CVS/procmeter3/no-x/run.c,v 1.1 2002-06-04 12:48:58 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3b.

  Run external programs.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1999,2000,02 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include "procmeterp.h"


/*++++++++++++++++++++++++++++++++++++++
  Parse the command from the config file to be run.

  char *string The string to be parsed.

  RunOption *run Contains the parsed result.
  ++++++++++++++++++++++++++++++++++++++*/

void ParseRunCommand(char *string,RunOption *run)
{

 /* Do nothing since we have no X windows. */

 run->flag=RUN_NONE;
 run->command=NULL;
}
