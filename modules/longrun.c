/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/longrun.c,v 1.1 2002-04-14 14:03:33 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3a.

  Transmeta longrun support.
  ******************/ /******************
  Written by Joey Hess (with heavy borrowing from longrun)

  This file Copyright 2002 Joey Hess
            Copyright 2001 Transmeta Corporation
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define __USE_UNIX98    /* for pread/pwrite */
#include <unistd.h>

#include "procmeter.h"

/* The canonical source for these defines is longrun.c in the longrun
 * utility. */
#define CPUID_DEVICE "/dev/cpu/0/cpuid"
#define CPUID_TMx86_LONGRUN_STATUS 0x80860007
#define CPUID_TMx86_VENDOR_ID 0x80860000
#define CPUID_TMx86_PROCESSOR_INFO 0x80860001
#define CPUID_TMx86_FEATURE_LONGRUN(x) ((x) & 0x02)

static int cpuid_fd = 0;

static void read_cpuid(long address, int *eax, int *ebx, int *ecx, int *edx) {
        uint32_t data[4];

        if (pread(cpuid_fd, &data, 16, address) != 16) {
                perror("error reading");
        }

        if (eax) *eax = data[0];
        if (ebx) *ebx = data[1];
        if (ecx) *ecx = data[2];
        if (edx) *edx = data[3];
}

/* The interface information.  */

/*+ The template for the longrun devices +*/
ProcMeterOutput _outputs[1]=
{
 {
  /* char  name[16];         */ "Longrun",
  /* char *description;      */ "current longrun performance level",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 %",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 20,
  /* char  graph_units[8];   */ "(%d%%)"
 },
};

static int ndevices=0;
static unsigned long *current=NULL,*previous=NULL;
static char **device=NULL;

static void add_device(void);

/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];           */ "Longrun",
 /* char *description;       */ "Transemta Crusoe longrun information"
};

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
 int eax, ebx, ecx, edx;

 outputs=(ProcMeterOutput**)malloc(sizeof(ProcMeterOutput*));
 outputs[0]=NULL;

 if ((cpuid_fd = open(CPUID_DEVICE, O_RDONLY)) < 0) {
	 fprintf(stderr, "Procmeter(%s): cannot open " CPUID_DEVICE ". make sure your kernel was compiled with CONFIG_X86_CPUID=y, and make sure the device is readable", __FILE__);
	 return outputs;
 }
 
 /* See if longrun is supported by this system. */
 read_cpuid(CPUID_TMx86_VENDOR_ID, &eax, &ebx, &ecx, &edx);
 if (ebx != 0x6e617254 || ecx != 0x55504361 || edx != 0x74656d73) {
	 fprintf(stderr, "Procmeter(%s): not a Transmeta x86 CPU.\n", __FILE__);
	 return outputs;
 }
 read_cpuid(CPUID_TMx86_PROCESSOR_INFO, &eax, &ebx, &ecx, &edx);
 if (!CPUID_TMx86_FEATURE_LONGRUN(edx)) {
	 fprintf(stderr, "Procmeter(%s): longrun unsupported.\n", __FILE__);
	 return outputs;
 }
 
 add_device();

 current =(long*)malloc(sizeof(long)*ndevices);
 previous=(long*)malloc(sizeof(long)*ndevices);

 return(outputs);
}

/*++++++++++++++++++++++++++++++++++++++
  Add a new device to the list.

  Currently we just support one CPU, so no parameters.
  ++++++++++++++++++++++++++++++++++++++*/

static void add_device(void)
{
 int nstats=1;
 int i;

 outputs=(ProcMeterOutput**)realloc((void*)outputs,(ndevices+nstats+1)*sizeof(ProcMeterOutput*));
 device=(char**)realloc((void*)device,(ndevices+nstats+1)*sizeof(char*));

 for(i=0;nstats;nstats--)
   {
    outputs[ndevices]=(ProcMeterOutput*)malloc(sizeof(ProcMeterOutput));
    device[ndevices]=(char*)malloc(1);

    *outputs[ndevices]=_outputs[i];
    sprintf(outputs[ndevices]->name,_outputs[i].name);
    outputs[ndevices]->description=(char*)malloc(strlen(_outputs[i].description)+4);
    sprintf(outputs[ndevices]->description,_outputs[i].description);

    strcpy(device[ndevices],"0");

    ndevices++;

    i++;
   }

 outputs[ndevices]=NULL;
}

/*++++++++++++++++++++++++++++++++++++++
  Perform an update on one of the statistics.

  int Update Returns 0 if OK, else -1.

  time_t now The current time.

  ProcMeterOutput *output The output that the value is wanted for.
  ++++++++++++++++++++++++++++++++++++++*/

int Update(time_t now,ProcMeterOutput *output)
{
 int percent;
 read_cpuid(CPUID_TMx86_LONGRUN_STATUS, 0, 0, &percent, 0);
 output[0].graph_value=PROCMETER_GRAPH_FLOATING(percent/output[0].graph_scale);
 sprintf(output->text_value,"%i %%",percent);
 return(0);
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
 if(current)
    free(current);
 if(previous)
    free(previous);
 if(device)
   {
    for(i=0;i<ndevices;i++)
       free(device[i]);
    free(device);
   }
 if (cpuid_fd)
   close(cpuid_fd);
}
