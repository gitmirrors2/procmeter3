/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/biff.c,v 1.7 2008-05-05 18:45:17 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.5b.

  Mail inbox monitor.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998-2008 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <utime.h>
#include <sys/time.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The number of e-mails output. +*/
ProcMeterOutput count_output=
{
 /* char  name[];          */ "Inbox_Count",
 /* char *description;     */ "The number of mail messages that are in the inbox.",
 /* char  type;            */ PROCMETER_TEXT,
 /* short interval;        */ 15,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ -1,
 /* short graph_scale;     */ 0,
 /* char  graph_units[];   */ "n/a"
};

/*+ The size of the mailbox output. +*/
ProcMeterOutput size_output=
{
 /* char  name[];          */ "Inbox_Size",
 /* char *description;     */ "The size of the mail inbox.",
 /* char  type;            */ PROCMETER_TEXT,
 /* short interval;        */ 15,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ -1,
 /* short graph_scale;     */ 0,
 /* char  graph_units[];   */ "n/a"
};

/*+ The outputs. +*/
ProcMeterOutput *outputs[]=
{
 &count_output,
 &size_output,
 NULL
};

/*+ The null outputs. +*/
ProcMeterOutput *null_outputs[]=
{
 NULL
};

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];           */ "Biff",
 /* char *description;     */ "Monitors the size and number of e-mails that are waiting in the UNIX email inbox '%s'.",
};


/* The line buffer */
static char *line=NULL;
static size_t length=0;

/* The name of the file to monitor */
static char *filename=NULL;


/*++++++++++++++++++++++++++++++++++++++
  Load the module.

  ProcMeterModule *Load Returns the module information.
  ++++++++++++++++++++++++++++++++++++++*/

ProcMeterModule *Load(void)
{
 return(&module);
}


/*++++++++++++++++++++++++++++++++++++++
  Initialise the module, creating the outputs as required.

  ProcMeterOutput **Initialise Returns a NULL terminated list of outputs.

  char *options The options string for the module from the .procmeterrc file.
  ++++++++++++++++++++++++++++++++++++++*/

ProcMeterOutput **Initialise(char *options)
{
 char *str;
 struct stat buf;

 if(options)
   {
    filename=options;

    if(stat(filename,&buf))
       fprintf(stderr,"ProcMeter(%s): Cannot stat the file '%s', continuing anyway.\n",__FILE__,filename);
   }
 else
   {
    static char location[40];
    struct passwd *pw=getpwuid(getuid());

    if(!pw)
      {fprintf(stderr,"ProcMeter(%s): Cannot get username information.\n",__FILE__);return(null_outputs);}

    sprintf(location,"/var/spool/mail/%s",pw->pw_name);
    if(stat(location,&buf))
      {
       fprintf(stderr,"ProcMeter(%s): Cannot stat the file '%s' trying another.\n",__FILE__,location);

       sprintf(location,"/var/mail/%s",pw->pw_name);
       if(stat(location,&buf))
         {
          fprintf(stderr,"ProcMeter(%s): Cannot stat the file '%s', continuing with first choice.\n",__FILE__,location);
          sprintf(location,"/var/spool/mail/%s",pw->pw_name);
         }
      }

    filename=location;
   }

 str=module.description;
 module.description=(char*)malloc(strlen(str)+strlen(filename)+1);
 sprintf(module.description,str,filename);

 Update(1,NULL);

 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Perform an update on one of the statistics.

  int Update Returns 0 if OK, else -1.

  time_t now The current time.

  ProcMeterOutput *output The output that the value is wanted for.
  ++++++++++++++++++++++++++++++++++++++*/

int Update(time_t now,ProcMeterOutput *output)
{
 static time_t last=0,mtime=0,atime=0;
 static int count,size;
 struct utimbuf utimebuf;

 if(now!=last)
   {
    struct stat buf;

    if(stat(filename,&buf))
      {
       count=size=0;
       mtime=atime=0;
      }
    else
      {
       if(mtime!=buf.st_mtime || atime!=buf.st_atime || size!=buf.st_size)
         {
          FILE *f=fopen(filename,"r");

          count=0;

          if(f)
            {
             while(fgets_realloc(&line,&length,f))
                if(!strncmp("From ",line,5))
                   count++;

             fclose(f);
            }

          mtime=buf.st_mtime;
          atime=buf.st_atime;
          size=buf.st_size;
          
          utimebuf.actime=atime;
          utimebuf.modtime=mtime;
          utime(filename,&utimebuf);
         }
      }

    last=now;
   }

 if(output==&count_output)
   {
    sprintf(output->text_value,"%d emails",count);

    return(0);
   }
 else if(output==&size_output)
   {
    sprintf(output->text_value,"%d KB",size/1024);

    return(0);
   }

 return(-1);
}


/*++++++++++++++++++++++++++++++++++++++
  Tidy up and prepare to have the module unloaded.
  ++++++++++++++++++++++++++++++++++++++*/

void Unload(void)
{
 if(filename)
    free(module.description);

 if(line)
    free(line);
}
