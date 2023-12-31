/***************************************
  $Header: /home/amb/CVS/procmeter3/procmeterrc.c,v 1.10 2008-05-05 18:45:17 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.5b.

  Handle the .procmeterrc file.
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


/*++++++++++++++++++++++++++++++++++++++
  Load in the configuration file.

  int *argc The number of command line arguments.

  char **argv The command line arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void LoadProcMeterRC(int *argc,char **argv)
{
 char *rcpath=NULL,*home;
 Section *next_section=&FirstSection;
 Parameter *next_parameter=NULL,prev_parameter=NULL;
 int i;

 /* Find the .procmeterrc file. */

 for(i=1;i<*argc;i++)
    if(!strncmp(argv[i],"--rc=",5))
      {
       rcpath=argv[i]+5;
       for((*argc)--;i<*argc;i++)
          argv[i]=argv[i+1];
      }

 if(!rcpath)
   {
    struct stat buf;

    if(!stat(".procmeterrc",&buf))
       rcpath=".procmeterrc";
    else if((home=getenv("HOME")))
      {
       rcpath=(char*)malloc(strlen(home)+16);

       strcpy(rcpath,home);
       strcat(rcpath,"/.procmeterrc");

       if(stat(rcpath,&buf))
         {free(rcpath);rcpath=NULL;}
      }
   }
 if(!rcpath)
    rcpath=RC_PATH "/procmeterrc";

 /* Read the .procmeterrc file. */

 if(rcpath)
   {
    FILE *rc=fopen(rcpath,"r");
    char *line=NULL;
    size_t length=256;
    int continued=0;

    if(!rc)
      {
       fprintf(stderr,"ProcMeter: Cannot open the configuration file %s for reading.\n",rcpath);
       exit(1);
      }

    while(fgets_realloc(&line,&length,rc))
      {
       char *l=line,*r=line+strlen(line)-1;

       while(*l==' ' || *l=='\t')
          l++;
       while(r>l && (*r=='\n' || *r=='\r' || *r==' ' || *r=='\t'))
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

    fclose(rc);
   }

 /* Add in extra command line options. */

 for(i=1;i<*argc;i++)
    if(!strncmp(argv[i],"--",2))
      {
       int j;
       char *equal=strchr(argv[i],'='),*dot;

       if(equal)
         {
          char *section=argv[i]+2,*parameter,*value=equal+1;
          Section this_section=FirstSection;

          *equal=0;
          dot=strchr(section,'.');

          if(dot)
            {
             char *dot2=strchr(dot+1,'.');

             if(dot2) /* 2 dots => (module.output).parameter */
               {
                parameter=dot2+1;
                *dot2=0;
               }
             else     /* 1 dot => section.parameter */
               {
                parameter=dot+1;
                *dot=0;
               }

             while(this_section)
               {
                if(!strcasecmp(section,this_section->name))
                  {
                   Parameter this_parameter=this_section->first;

                   while(this_parameter)
                     {
                      if(!strcasecmp(parameter,this_parameter->name))
                        {
                         strcpy(this_parameter->value=(char*)realloc((void*)this_parameter->value,strlen(value)+1),value);
                         break;
                        }
                      this_parameter=this_parameter->next;
                     }

                   if(!this_parameter)
                     {
                      Parameter new_parameter=(Parameter)malloc(sizeof(struct _Parameter));
                      strcpy(new_parameter->name=(char*)malloc(strlen(parameter)+1),parameter);
                      if(value)
                         strcpy(new_parameter->value=(char*)malloc(strlen(value)+1),value);
                      else
                         new_parameter->value=NULL;

                      new_parameter->next=this_section->first;
                      this_section->first=new_parameter;
                     }

                   break;
                  }

                this_section=this_section->next;
               }

             if(!this_section)
               {
                Section new_section=(Section)malloc(sizeof(struct _Section));
                strcpy(new_section->name=(char*)malloc(strlen(section)+1),section);

                new_section->first=(Parameter)malloc(sizeof(struct _Parameter));
                strcpy(new_section->first->name=(char*)malloc(strlen(parameter)+1),parameter);
                if(value)
                   strcpy(new_section->first->value=(char*)malloc(strlen(value)+1),value);
                else
                   new_section->first->value=NULL;
                new_section->first->next=NULL;

                new_section->next=FirstSection;
                FirstSection=new_section;
               }
            }
         }

       for(j=i,i--,(*argc)--;j<*argc;j++)
          argv[j]=argv[j+1];
      }
}


/*++++++++++++++++++++++++++++++++++++++
  Return the results of a section of the configuration file.

  char *GetProcMeterRC Returns the result string or NULL if none.

  char *section The section of the config file.

  char *parameter The parameter in the section of the config file.
  ++++++++++++++++++++++++++++++++++++++*/

char *GetProcMeterRC(char *section,char *parameter)
{
 Section this_section=FirstSection;

 while(this_section)
   {
    if(!strcasecmp(section,this_section->name))
      {
       Parameter this_parameter=this_section->first;

       while(this_parameter)
         {
          if(!strcasecmp(parameter,this_parameter->name))
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

  char *parameter The parameter to search for.
  ++++++++++++++++++++++++++++++++++++++*/

char *GetProcMeterRC2(char *module,char *output,char *parameter)
{
 ProcMeterModule m;
 ProcMeterOutput o;
 char section[sizeof(m.name)+sizeof(o.name)+2];

 strcpy(section,module);
 strcat(section,".");
 strcat(section,output);

 return(GetProcMeterRC(section,parameter));
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
