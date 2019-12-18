/***************************************
  ProcMeter - A system monitoring program for Linux - Version 3.6a.

  Mail inbox monitor.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998-2008, 2017 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
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

/* The name of the file or directory to monitor */
static char *filename=NULL;
static char *filedir=NULL;


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
    if(stat(options,&buf))
      {fprintf(stderr,"ProcMeter(%s): Cannot stat the file/directory '%s', ignoring it.\n",__FILE__,options);return(null_outputs);}
    else
       if(S_ISDIR(buf.st_mode))
          filedir=options;
       else
          filename=options;
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
 module.description=(char*)malloc(strlen(str)+strlen(filename?filename:filedir)+1);
 sprintf(module.description,str,filename?filename:filedir);

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

 if(now!=last)
   {
    struct utimbuf utimebuf;
    struct stat buf;

    if(filename)
      {
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
      }
    else if(filedir)
      {
       if(stat(filedir,&buf))
         {
          count=size=0;
          mtime=atime=0;
         }
       else
         {
          if(mtime!=buf.st_mtime || atime!=buf.st_atime)
            {
             struct dirent *ent;
             DIR *dir;

             count=0;
             size=0;

             dir=opendir(filedir);

             if(dir)
               {
                while((ent = readdir(dir)) != NULL)
                  {
                   struct stat buf2;
                   char name[2*NAME_MAX+2];

                   strcpy(name,filedir);
                   strcat(name,"/");
                   strcat(name,ent->d_name);

                   if(!stat(name,&buf2) && S_ISREG(buf2.st_mode))
                     {
                      count++;
                      size+=buf2.st_size;
                     }
                  }

                closedir(dir);
               }

             mtime=buf.st_mtime;
             atime=buf.st_atime;
          
             utimebuf.actime=atime;
             utimebuf.modtime=mtime;
             utime(filename,&utimebuf);
            }
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
 if(filename || filedir)
    free(module.description);

 if(line)
    free(line);
}
