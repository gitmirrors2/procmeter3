/***************************************
  $Header: /home/amb/CVS/procmeter3/module.c,v 1.9 1999-12-04 16:55:45 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.2.

  Module handling functions.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

 Modules=(Module*)malloc(16*sizeof(Module));
 *Modules=NULL;

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

 ent=readdir(dir);  /* skip .  */
 if(ent)
   {
    ent=readdir(dir);  /* skip .. */

    while((ent=readdir(dir)))
       if(!strcmp(ent->d_name+strlen(ent->d_name)-3,".so"))
         {
          strcpy(libp,ent->d_name);
          LoadModule(lib);
         }
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
 Module *modules;
 Module new=NULL;
 int noutputs,i;

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

 if(Modules)
    for(modules=Modules;*modules;modules++)
       if(!strcmp((*modules)->module->name,new->module->name))
         {
          fprintf(stderr,"ProcMeter: Duplicate module name '%s' ignoring '%s'.\n",new->module->name,new->filename);
          dlclose(new->dlhandle);
          free(new->filename);
          free(new);
          return(NULL);
         }

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

 modules=Modules;
 while(*modules)
    modules++;

 Modules=(Module*)realloc((void*)Modules,(modules-Modules+2)*sizeof(Module));

 modules=Modules;
 while(*modules)
    modules++;

 *modules=new;
 *++modules=NULL;

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

          if(!(string=GetProcMeterRC2(new->module->name,output->name,"run")))
             string=GetProcMeterRC(new->module->name,"run");
          ParseRunCommand(string,&new->outputs[noutputs]->menu_run);

          if(!(string=GetProcMeterRC2(new->module->name,output->name,"label")))
            {
             static char newstr[16];
             int i;
             for(i=0;output->name[i];i++)
                if(output->name[i]=='_')
                   newstr[i]=' ';
                else
                   newstr[i]=output->name[i];
             newstr[i]=0;
             string=newstr;
            }

          strncpy(new->outputs[noutputs]->label,string,16);
          new->outputs[noutputs]->label[15]=0;

          noutputs++;
         }

       t<<=1;
      }
   }

 /* Add to the menus */

 AddModuleToMenu(new);

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
