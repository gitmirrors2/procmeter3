/***************************************
  $Header: /home/amb/CVS/procmeter3/procmeterrc.c,v 1.2 1999-12-04 16:56:51 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.2.

  Handle the .procmeterrc file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include "procmeter.h"
#include "procmeterp.h"

typedef struct _Section *Section;
typedef struct _Parameter *Parameter;

/*+ A section from the file. +*/
struct _Section
{
 char *name;                    /*+ The name of the section. +*/
 Section next;                  /*+ The next section; +*/
 Parameter first;               /*+ The first of the parameters. +*/
};

/*+ A parameter from the file. +*/
struct _Parameter
{
 char *name;                    /*+ The name of the parameter. +*/
 char *value;                   /*+ The value of the parameter. +*/
 Parameter next;                /*+ The next parameter. +*/
};

/*+ The first section. +*/
Section FirstSection=NULL;


static char *fgets_realloc(char *buffer,FILE *file);


/*++++++++++++++++++++++++++++++++++++++
  Load in the configuration file.
  ++++++++++++++++++++++++++++++++++++++*/

void LoadProcMeterRC(void)
{
 FILE *rc=NULL;
 struct stat buf;
 char *home,*line=NULL;
 Section *next_section=&FirstSection;
 Parameter *next_parameter=NULL,prev_parameter=NULL;
 int continued=0;

 if(!stat(".procmeterrc",&buf))
    rc=fopen(".procmeterrc","r");
 else if((home=getenv("HOME")))
   {
    char *procrc=(char*)malloc(strlen(home)+16);

    strcpy(procrc,home);
    strcat(procrc,"/.procmeterrc");

    if(!stat(procrc,&buf))
       rc=fopen(procrc,"r");

    free(procrc);
   }
 if(!rc)
    rc=fopen(RC_PATH,"r");

 if(rc)
    while((line=fgets_realloc(line,rc)))
      {
       char *l=line,*r=line+strlen(line)-1;

       while(*l==' ' || *l=='\t')
          l++;
       while(*r=='\n' || *r=='\r' || *r==' ' || *r=='\t')
          *r--=0;

       if(*l==';' || *l=='#' || l>=r)
          continue;

       if(*l=='[' && *r==']')
         {
          l++;*r--=0;
          *next_section=(Section)malloc(sizeof(struct _Section));
          strcpy((*next_section)->name=(char*)malloc(strlen(l)+1),l);
          (*next_section)->next=NULL;
          (*next_section)->first=NULL;
          next_parameter=&(*next_section)->first;
          next_section=&(*next_section)->next;
          continued=0;
         }
       else if(continued)
         {
          continued=0;

          if(l[strlen(l)-1]=='\\')
            {
             continued=1;
             l[strlen(l)-1]=0;
            }

          prev_parameter->value=(char*)realloc((void*)prev_parameter->value,strlen(l)+strlen(prev_parameter->value)+1);
          strcat(prev_parameter->value,l);
         }
       else if(next_parameter)
         {
          char *equal=strchr(l,'=');
          if(equal)
            {
             char *lr=equal-1,*rl=equal+1;
             *equal=0;
             while(*lr==' ' || *lr=='\t')
                *lr--=0;
             while(*rl==' ' || *rl=='\t')
                rl++;
             equal=rl;
            }

          *next_parameter=(Parameter)malloc(sizeof(struct _Parameter));
          strcpy((*next_parameter)->name=(char*)malloc(strlen(l)+1),l);

          continued=0;

          if(equal)
            {
             if(equal[strlen(equal)-1]=='\\')
               {
                continued=1;
                equal[strlen(equal)-1]=0;
               }
             strcpy((*next_parameter)->value=(char*)malloc(strlen(equal)+1),equal);
            }
          else
             (*next_parameter)->value=NULL;

          (*next_parameter)->next=NULL;
          prev_parameter=*next_parameter;
          next_parameter=&(*next_parameter)->next;
         }
      }
}


/*++++++++++++++++++++++++++++++++++++++
  Return the results of a section of the configuration file.

  char *GetProcMeterRC Returns the result string or NULL if none.

  char *section The section of the config file.

  char *item The item in the section of the config file.
  ++++++++++++++++++++++++++++++++++++++*/

char *GetProcMeterRC(char *section,char *item)
{
 Section this_section=FirstSection;

 while(this_section)
   {
    if(!strcmp(section,this_section->name))
      {
       Parameter this_parameter=this_section->first;

       while(this_parameter)
         {
          if(!strcmp(item,this_parameter->name))
             return(this_parameter->value);
          this_parameter=this_parameter->next;
         }
      }

    this_section=this_section->next;
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Get the resources for an output.

  char *GetProcMeterRC2 Returns the result string or NULL if none.

  char *module The module name (part of section name).

  char *output The output name (part of section name).

  char *item The item to search for.
  ++++++++++++++++++++++++++++++++++++++*/

char *GetProcMeterRC2(char *module,char *output,char *item)
{
 ProcMeterModule m;
 ProcMeterOutput o;
 char section[sizeof(m.name)+sizeof(o.name)+2];

 strcpy(section,module);
 strcat(section,".");
 strcat(section,output);

 return(GetProcMeterRC(section,item));
}


/*++++++++++++++++++++++++++++++++++++++
  Free up the memory that is used in this module.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeProcMeterRC(void)
{
 Section this_section=FirstSection;

 while(this_section)
   {
    Section last_section=this_section;
    Parameter this_parameter=this_section->first;

    while(this_parameter)
      {
       Parameter last_parameter=this_parameter;

       if(this_parameter->name)
          free(this_parameter->name);
       if(this_parameter->value)
          free(this_parameter->value);
       this_parameter=this_parameter->next;

       free(last_parameter);
      }

    this_section=this_section->next;

    free(last_section->name);
    free(last_section);
   }
}


#define BUFSIZE 64

/*++++++++++++++++++++++++++++++++++++++
  Call fgets and realloc the buffer as needed to get a whole line.

  char *fgets_realloc Returns the modified buffer (NULL at the end of the file).

  char *buffer The current buffer.

  FILE *file The file to read from.
  ++++++++++++++++++++++++++++++++++++++*/

static char *fgets_realloc(char *buffer,FILE *file)
{
 int n=0;
 char *buf;

 if(!buffer)
    buffer=(char*)malloc((BUFSIZE+1));

 while((buf=fgets(&buffer[n],BUFSIZE,file)))
   {
    int s=strlen(buf);
    n+=s;

    if(buffer[n-1]=='\n')
       break;
    else
       buffer=(char*)realloc(buffer,n+(BUFSIZE+1));
   }

 if(!buf)
   {free(buffer);buffer=NULL;}

 return(buffer);
}
