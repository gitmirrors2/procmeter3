/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/logfile.c,v 1.9 2008-05-05 18:45:35 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.5b.

  A log file monitoring source file.
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

#include <sys/stat.h>
#include <sys/types.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The template for the log file +*/
ProcMeterOutput _outputs[4]=
{
 /*+ The size in bytes +*/
 {
  /* char  name[];          */ "Log_Size_%s",
  /* char *description;     */ "The size of the log file '%s' in KBytes.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 KB",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 100,
  /* char  graph_units[];   */ "(%dKB)"
 },
 /*+ The rate of change of size in bytes +*/
 {
  /* char  name[];          */ "Log_Grow_%s",
  /* char *description;     */ "The rate at which the size of the log file '%s' is increasing in KBytes/second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 KB/s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 1,
  /* char  graph_units[];   */ "(%dK/s)"
 },
 /*+ The number of lines +*/
 {
  /* char  name[];          */ "Log_Line_%s",
  /* char *description;     */ "The number of lines in the log file '%s'.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 lines",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 100,
  /* char  graph_units[];   */ "(%d)"
 },
 /*+ The rate of change of number of lines +*/
 {
  /* char  name[];          */ "Log_Rate_%s",
  /* char *description;     */ "The number of lines by which the log file '%s' is increasing per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 lines/s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 10,
  /* char  graph_units[];   */ "(%d/s)"
 }
};


/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];            */ "LogFile",
 /* char *description;      */ "The size and number of lines in the specified log files.  "
                               "(Use 'options=<filename1> <filename2>' in the configuration file to specify the files.)"
};


/* The files to monitor and the information about them */
static int nfiles=0;
static char **file=NULL;
static long *last=NULL;
static long *mtime=NULL;
static long *size=NULL;
static long *grow=NULL;
static long *line=NULL;
static long *rate=NULL;

/* Add a file to be monitored */
static void add_file(char *fil);


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
 outputs=(ProcMeterOutput**)malloc(sizeof(ProcMeterOutput*));
 outputs[0]=NULL;

 /* Get the options */

 if(options)
   {
    char *l=options;

    while(*l && *l==' ')
       l++;

    while(*l)
      {
       char *r=l,pr;

       while(*r && *r!=' ')
          r++;

       pr=*r;
       *r=0;

       add_file(l);

       *r=pr;
       while(*r && *r==' ')
          r++;

       if(!*r)
          break;

       l=r;
      }
   }

 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a new file to the list.

  char *fil The name of the file to add.
  ++++++++++++++++++++++++++++++++++++++*/

static void add_file(char *fil)
{
 int i,j;
 char *slash,*lastslash=fil-1;

 for(i=0;i<nfiles;i++)
    if(!strcmp(file[i],fil))
       return;

 while((slash=strchr(lastslash+1,'/')))
    lastslash=slash;

 outputs=(ProcMeterOutput**)realloc((void*)outputs,(nfiles*4+5)*sizeof(ProcMeterOutput*));

 for(i=0,j=4*nfiles;i<4;i++,j++)
   {
    outputs[j]=(ProcMeterOutput*)malloc(sizeof(ProcMeterOutput));

    *outputs[j]=_outputs[i];
    snprintf(outputs[j]->name, PROCMETER_NAME_LEN+1, _outputs[i].name, lastslash+1);
    outputs[j]->description=(char*)malloc(strlen(fil)+strlen(_outputs[i].description)+4);
    sprintf(outputs[j]->description,_outputs[i].description,fil);
   }

 file =(char**)realloc((void*)file ,(nfiles+1)*sizeof(char*));
 last =(long *)realloc((void*)last ,(nfiles+1)*sizeof(long));
 mtime=(long *)realloc((void*)mtime,(nfiles+1)*sizeof(long));
 size =(long *)realloc((void*)size ,(nfiles+1)*sizeof(long));
 grow =(long *)realloc((void*)grow ,(nfiles+1)*sizeof(long));
 line =(long *)realloc((void*)line ,(nfiles+1)*sizeof(long));
 rate =(long *)realloc((void*)rate ,(nfiles+1)*sizeof(long));

 file[nfiles]=(char*)malloc(strlen(fil)+1);
 strcpy(file[nfiles],fil);
 last[nfiles]=mtime[nfiles]=size[nfiles]=grow[nfiles]=line[nfiles]=rate[nfiles]=0;
 nfiles++;

 outputs[j]=NULL;
}


/*++++++++++++++++++++++++++++++++++++++
  Perform an update on one of the statistics.

  int Update Returns 0 if OK, else -1.

  time_t now The current time.

  ProcMeterOutput *output The output that the value is wanted for.
  ++++++++++++++++++++++++++++++++++++++*/

int Update(time_t now,ProcMeterOutput *output)
{
 int i;

 for(i=0;outputs[i];i++)
    if(output==outputs[i])
      {
       if(last[i/4]!=now)
         {
          struct stat buf;

          if(stat(file[i/4],&buf))
             mtime[i/4]=size[i/4]=grow[i/4]=line[i/4]=rate[i/4]=0;
          else
            {
             int lines=0;

             if(buf.st_size<size[i/4])
                size[i/4]=line[i/4]=0;

             if(buf.st_size>size[i/4])
               {
                FILE *f=fopen(file[i/4],"r");
                char buffer[2048];
                int nread,byte;

                if(f)
                  {
                   fseek(f,size[i/4],SEEK_SET);

                   while((nread=fread(buffer,1,2048,f))>0)
                      for(byte=0;byte<nread;byte++)
                         if(buffer[byte]=='\n')
                            lines++;

                   fclose(f);
                  }
               }

             mtime[i/4]=buf.st_mtime;
             grow[i/4]=(buf.st_size-size[i/4])/(now-last[i/4]);
             size[i/4]=buf.st_size;
             rate[i/4]=lines/(now-last[i/4]);
             line[i/4]+=lines;
            }

          last[i/4]=now;
         }

       switch(i%4)
         {
         case 0:
          output->graph_value=PROCMETER_GRAPH_FLOATING((double)size[i/4]/(1024.0*output->graph_scale));
          sprintf(output->text_value,"%.1f KB",(double)size[i/4]/1024);
          break;
         case 1:
          output->graph_value=PROCMETER_GRAPH_FLOATING((double)grow[i/4]/(1024.0*output->graph_scale));
          sprintf(output->text_value,"%.2f KB/s",(double)grow[i/4]/1024);
          break;
         case 2:
          output->graph_value=PROCMETER_GRAPH_FLOATING((double)line[i/4]/output->graph_scale);
          sprintf(output->text_value,"%.0f lines",(double)line[i/4]);
          break;
         case 3:
          output->graph_value=PROCMETER_GRAPH_FLOATING((double)rate[i/4]/output->graph_scale);
          sprintf(output->text_value,"%.0f lines/s",(double)rate[i/4]);
          break;
         }

       return(0);
      }

 return(-1);
}


/*++++++++++++++++++++++++++++++++++++++
  Tidy up and prepare to have the module unloaded.
  ++++++++++++++++++++++++++++++++++++++*/

void Unload(void)
{
 int i;

 if(outputs)
   {
    for(i=0;outputs[i];i++)
      {
       free(outputs[i]->description);
       free(outputs[i]);
      }
    free(outputs);
   }

 if(file)
   {
    for(i=0;i<nfiles;i++)
       free(file[i]);
    free(file);
    free(last);
    free(mtime);
    free(size);
    free(grow);
    free(line);
    free(rate);
   }
}
