/***************************************
  $Header: /home/amb/CVS/procmeter3/module.c,v 1.17 2009-12-01 18:38:22 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.5c.

  Module handling functions.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998-2009 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <dlfcn.h>

#include "procmeter.h"
#include "procmeterp.h"

/*+ The list of all of the modules. +*/
Module *Modules=NULL;


/*++++++++++++++++++++++++++++++++++++++
  Load in all of the modules in the library directory.
  ++++++++++++++++++++++++++++++++++++++*/

void LoadAllModules(void)
{
 DIR *dir;
 struct dirent* ent;
 struct stat buf;
 char lib[PATH_MAX+1],*libp;
 char *library;
 int i;

 Modules=(Module*)malloc(16*sizeof(Module));
 *Modules=NULL;

 /* set LC_NUMERIC to C, else 0.1 in /proc/loadavg may not be parsed */

 setlocale(LC_NUMERIC, "C");

 /* Load the library modules */

 if(!(library=GetProcMeterRC("library","path")))
    library=MOD_PATH;

 strcpy(lib,library);
 libp=lib+strlen(lib)-1;
 if(*libp!='/')
    *++libp='/';
 *++libp=0;

 if(stat(library,&buf) || !S_ISDIR(buf.st_mode))
   {
    fprintf(stderr,"The library directory '%s' does not exist or is not a directory.\n",library);
    exit(1);
   }

 dir=opendir(library);
 if(!dir)
   {
    fprintf(stderr,"The library directory '%s' cannot be opened.\n",library);
    exit(1);
   }

 while((ent=readdir(dir)))
    if(!strcmp(ent->d_name+strlen(ent->d_name)-3,".so"))
      {
       strcpy(libp,ent->d_name);
       LoadModule(lib);
      }

 closedir(dir);

 /* Load the other modules. */

 if((library=GetProcMeterRC("library","others")))
   {
    char *l=library;

    getcwd(lib,PATH_MAX);

    libp=lib+strlen(lib)-1;
    if(*libp!='/')
       *++libp='/';
    *++libp=0;

    while(*l && *l==' ')
       l++;

    while(*l)
      {
       char *r=l,pr;

       while(*r && *r!=' ')
          r++;

       pr=*r;
       *r=0;

       if(*l=='/')
          LoadModule(l);
       else
         {
          strcpy(libp,l);
          LoadModule(lib);
         }

       *r=pr;
       while(*r && *r==' ')
          r++;

       if(!*r)
          break;

       l=r;
      }
   }

 /* Add to the menus */

 for(i=0;Modules[i];i++)
    AddModuleToMenu(Modules[i]);

}


/*++++++++++++++++++++++++++++++++++++++
  Unload all of the modules.
  ++++++++++++++++++++++++++++++++++++++*/

void UnloadAllModules(void)
{
 while(*Modules)
   {
    UnloadModule(*Modules);
   }

 free(Modules);
}


/*++++++++++++++++++++++++++++++++++++++
  Load the specified module.

  Module LoadModule Returns a pointer to the new module structure, or NULL on error.

  char* filename The filename of the module to load.
  ++++++++++++++++++++++++++++++++++++++*/

Module LoadModule(char* filename)
{
 ProcMeterModule *(*Load)(void);
 ProcMeterOutput **(*Initialise)(char *),**outputs,*output;
 Module new=NULL;
 int noutputs,i,m,c;

 new=(Module)malloc(sizeof(struct _Module));

 new->filename=(char*)malloc(strlen(filename)+1);
 strcpy(new->filename,filename);

 new->dlhandle=dlopen(filename,RTLD_NOW);

 if(!new->dlhandle)
   {
    fprintf(stderr,"ProcMeter: Cannot open the module '%s' : %s\n",new->filename,dlerror());
    free(new->filename);
    free(new);
    return(NULL);
   }

 /* Get the Load() function and call it */

 Load=dlsym(new->dlhandle,"Load");

 if(!Load)
   {
    fprintf(stderr,"ProcMeter: Cannot get the symbol 'Load' from '%s' : %s\n",new->filename,dlerror());
    dlclose(new->dlhandle);
    free(new->filename);
    free(new);
    return(NULL);
   }

 if(!(new->module=(*Load)()))
   {
    fprintf(stderr,"ProcMeter: Error calling 'Load()' in module '%s'\n",new->filename);
    dlclose(new->dlhandle);
    free(new->filename);
    free(new);
    return(NULL);
   }

 for(m=-1,i=0;Modules[i];i++)
    if(!(c=strcmp(Modules[i]->module->name,new->module->name)))
      {
       fprintf(stderr,"ProcMeter: Duplicate module name '%s' ignoring '%s'.\n",new->module->name,new->filename);
       dlclose(new->dlhandle);
       free(new->filename);
       free(new);
       return(NULL);
      }
    else if(c<0)
       m=i;

 /* Get the Intialise() fuction and call it. */

 Initialise=dlsym(new->dlhandle,"Initialise");

 if(!Initialise)
   {
    fprintf(stderr,"ProcMeter: Cannot get the symbol 'Initialise' from '%s' : %s\n",new->filename,dlerror());
    dlclose(new->dlhandle);
    free(new->filename);
    free(new);
    return(NULL);
   }

 if(!(outputs=(*Initialise)(GetProcMeterRC(new->module->name,"options"))))
   {
    fprintf(stderr,"ProcMeter: Error Calling 'Initialise()' in module '%s'\n",new->filename);
    dlclose(new->dlhandle);
    free(new->filename);
    free(new);
    return(NULL);
   }

 /* Get the Update() function. */

 new->Update=dlsym(new->dlhandle,"Update");

 if(!new->Update)
   {
    fprintf(stderr,"ProcMeter: Cannot get the symbol 'Update' from '%s' : %s\n",new->filename,dlerror());
    dlclose(new->dlhandle);
    free(new->filename);
    free(new);
    return(NULL);
   }

 /* Insert the new module. */

 Modules=(Module*)realloc((void*)Modules,(i+2)*sizeof(Module));
 memmove(Modules+m+2, Modules+m+1, (i-m)*sizeof(Module));
 Modules[m+1]=new;

 /* Add the outputs */

 for(noutputs=0,i=0,output=outputs[i];output;output=outputs[++i])
   {
    int t=output->type;

    while(t)
      {
       if(t&1)
          noutputs++;
       t>>=1;
      }
   }

 new->outputs=(Output*)malloc((noutputs+1)*sizeof(Output));

 new->outputs[noutputs]=NULL;

 for(noutputs=0,i=0,output=outputs[i];output;output=outputs[++i])
   {
    int t=1;

    while(t<=output->type)
      {
       if(output->type&t)
         {
          char *string;

          if((string=GetProcMeterRC2(new->module->name,output->name,"update")) ||
             (string=GetProcMeterRC(new->module->name,"update")) ||
             (string=GetProcMeterRC("resources","update")))
             output->interval=atoi(string);

          if((string=GetProcMeterRC2(new->module->name,output->name,"graph-scale")) ||
             (string=GetProcMeterRC(new->module->name,"graph-scale")))
             output->graph_scale=atoi(string);

          new->outputs[noutputs]=(Output)malloc(sizeof(struct _Output));
          new->outputs[noutputs]->output=output;
          new->outputs[noutputs]->type=t;
          new->outputs[noutputs]->first=0;
          new->outputs[noutputs]->menu_item_widget=NULL;
          new->outputs[noutputs]->output_widget=NULL;

          if(!(string=GetProcMeterRC2(new->module->name,output->name,"run")))
             string=GetProcMeterRC(new->module->name,"run");
          ParseRunCommand(string,&new->outputs[noutputs]->menu_run);

          if(!(string=GetProcMeterRC2(new->module->name,output->name,"label")))
            {
             static char newstr[PROCMETER_NAME_LEN+1];
             int i;
             for(i=0;output->name[i];i++)
                if(output->name[i]=='_')
                   newstr[i]=' ';
                else
                   newstr[i]=output->name[i];
             newstr[i]=0;
             string=newstr;
            }

          strncpy(new->outputs[noutputs]->label,string,PROCMETER_NAME_LEN+1);
          new->outputs[noutputs]->label[PROCMETER_NAME_LEN]=0;

          noutputs++;
         }

       t<<=1;
      }
   }

 new->menu_item_widget=NULL;
 new->submenu_widget=NULL;

 return(new);
}


/*++++++++++++++++++++++++++++++++++++++
  Unload a module when we exit.

  Module module The module that is to be unloaded.
  ++++++++++++++++++++++++++++++++++++++*/

void UnloadModule(Module module)
{
 void (*Unload)(void);
 Module *modules;

 RemoveModuleFromMenu(module);

 Unload=dlsym(module->dlhandle,"Unload");

 if(Unload)
    (*Unload)();

 dlclose(module->dlhandle);

 /* Shuffle the modules down and tidy up. */

 modules=Modules;
 while(*modules)
   {
    if(*modules==module)
       while(*modules)
         {
          *modules=*(modules+1);
          modules++;
         }
    else
       modules++;
   }

 if(module->outputs)
   {
    int i;
    Output output;

    for(output=module->outputs[i=0];output;output=module->outputs[++i])
      {
       if(output->menu_run.command)
          free(output->menu_run.command);
       free(output);
      }
    free(module->outputs);
   }

 free(module->filename);
 free(module);
}


#define INCSIZE 256             /*+ The buffer increment size +*/

/*++++++++++++++++++++++++++++++++++++++
  Call fgets and realloc the buffer as needed to get a whole line.

  char *fgets_realloc Returns the modified buffer (NULL at the end of the file).

  char **buffer A pointer to the location of the buffer (pointer to NULL to intialise a new one).

  size_t *length A pointer to the current length of the buffer or the intial length if buffer is NULL.

  FILE *file The file to read from.
  ++++++++++++++++++++++++++++++++++++++*/

char *fgets_realloc(char **buffer,size_t *length,FILE *file)
{
 int n=0;
 char *buf;

 if(!*buffer)
   {
    if(!*length)
       *length=INCSIZE;

    *buffer=(char*)malloc(*length);

    if(!*buffer)
      {*length=0;return(NULL);}
   }

 while((buf=fgets(*buffer+n,*length-n,file)))
   {
    int s=strlen(buf);
    n+=s;

    if((*buffer)[n-1]=='\n')
       break;
    else
      {
       *length+=INCSIZE;
       *buffer=(char*)realloc(*buffer,*length);

       if(!*buffer)
         {*length=0;return(NULL);}
      }
   }

 if(!buf)
   {**buffer=0;return(NULL);}
 else
    return(*buffer);
}
