/***************************************
  ProcMeter - A system monitoring program for Linux - Version 3.5b.

  Interrupt statistics source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998-2010 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procmeter.h"

#define N_INTR 256

/* The interface information.  */

/*+ The total statistics +*/
ProcMeterOutput _output=
{
 /* char  name[];          */ "Interrupts",
 /* char *description;     */ "The total number of hardware interrupts per second.",
 /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
 /* short interval;        */ 1,
 /* char  text_value[];    */ "0 /s",
 /* long  graph_value;     */ 0,
 /* short graph_scale;     */ 100,
 /* char  graph_units[];   */ "(%d/s)"
};

/*+ The per interrupt statistics +*/
ProcMeterOutput _intr_output=
{
 /* char  name[];          */ "Interrupt%d",
 /* char *description;     */ "The number of hardware interrupts number %d (%s) per second.",
 /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
 /* short interval;        */ 1,
 /* char  text_value[];    */ "0 /s",
 /* long  graph_value;     */ 0,
 /* short graph_scale;     */ 100,
 /* char  graph_units[];   */ "(%d/s)"
};

/*+ The extra outputs with multiple interrupts +*/
ProcMeterOutput intr_outputs[N_INTR];

/*+ The outputs. +*/
ProcMeterOutput *outputs[N_INTR+1+1];

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];           */ "Stat-Intr",
 /* char *description;     */ "Interrupt statistics. [From /proc/stat]",
};


/* The line buffer */
static char *line=NULL;
static size_t length=0;

/*+ The number of interrupts. +*/
static int nintr=0;

/* The current and previous information about the interrupts */
static unsigned long long *current,*previous,values[2][N_INTR+1];


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
 FILE *f;
 int n=0;

 outputs[0]=NULL;

 current=values[0];
 previous=values[1];

 /* Verify the statistics from /proc/stat */

 f=fopen("/proc/stat","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/stat'.\n",__FILE__);
 else
   {
    /* cpu */
    if(!fgets_realloc(&line,&length,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/stat'.\n",__FILE__);
    else
      {
       unsigned long long intr;
       int i,p,pp;

       while(fgets_realloc(&line,&length,f)) /* cpu or disk or page or swap or intr */
          if(line[0]=='i' && line[1]=='n' && line[2]=='t' && line[3]=='r')
             break;

       if(!line[0])
          fprintf(stderr,"ProcMeter(%s): Unexpected 'intr' line in '/proc/stat'.\n"
                         "    expected: 'intr ...'\n"
                         "    found:    EOF",__FILE__);
       else if(sscanf(line,"intr %llu%n",&intr,&p)==1)
         {
          for(i=0;i<N_INTR;i++)
             if(sscanf(line+p,"%llu%n",&intr,&pp)==1)
               {
                char *type="unknown";
                FILE *f2;

                f2=fopen("/proc/interrupts","r");
                if(f2)
                  {
                   char *line2=NULL;
                   size_t length2=0;

                   while(fgets_realloc(&line2,&length2,f2))
                     {
                      int j,p2;

                      if(sscanf(line2,"%d: %*d%n",&j,&p2)==1 && j==nintr)
                        {
                         line2[strlen(line2)-1]=0;
                         while(line2[p2]!=0 && (line2[p2]==' ' || line2[p2]=='+'))
                            p2++;
                         type=line2+p2;
                         break;
                        }
                     }

                   if(line2)
                      free(line2);

                   fclose(f2);
                  }

                p+=pp;

                intr_outputs[nintr]=_intr_output;
                sprintf(intr_outputs[nintr].name,_intr_output.name,nintr);
                intr_outputs[nintr].description=(char*)malloc(strlen(_intr_output.description)+8+strlen(type));
                sprintf(intr_outputs[nintr].description,_intr_output.description,nintr,type);

                nintr++;
               }
             else
                break;

          outputs[n++]=&_output;

          for(i=0;i<nintr;i++)
             outputs[n++]=&intr_outputs[i];

          for(i=0;i<N_INTR+1;i++)
             current[i]=previous[i]=0;

          outputs[n]=NULL;
         }
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'intr' line in '/proc/stat'.\n"
                         "    expected: 'intr %%llu ...'\n"
                         "    found:    %s",__FILE__,line);
      }

    fclose(f);
   }

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
 static time_t last=0;
 int i;

 /* Get the statistics from /proc/stat */

 if(now!=last)
   {
    FILE *f;
    unsigned long long *temp;
    int p,pp;

    temp=current;
    current=previous;
    previous=temp;

    f=fopen("/proc/stat","r");
    if(!f)
       return(-1);

    while(fgets_realloc(&line,&length,f)) /* cpu or disk or page or swap or intr */
       if(line[0]=='i' && line[1]=='n' && line[2]=='t' && line[3]=='r')
          break;

    sscanf(line,"intr %llu%n",&current[0],&p);
    for(i=0;i<nintr;i++)
      {
       sscanf(line+p,"%llu%n",&current[i+1],&pp);
       p+=pp;
      }

    fclose(f);

    last=now;
   }

 for(i=0;i<=nintr;i++)
    if(output==outputs[i])
      {
       double value;

       if(previous[i]>current[i])
          value=0.0;
       else
          value=(double)(current[i]-previous[i])/output->interval;

       output->graph_value=PROCMETER_GRAPH_FLOATING(value/output->graph_scale);
       sprintf(output->text_value,"%.0f /s",value);

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

 for(i=0;i<nintr;i++)
    free(intr_outputs[i].description);

 if(line)
    free(line);
}
