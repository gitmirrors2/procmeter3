/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/stat-disk.c,v 1.5 2000-12-13 17:32:49 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.2b.

  Disk statistics source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2000 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "procmeter.h"

#define DISK        0
#define DISK_READ   1
#define DISK_WRITE  2
#define N_OUTPUTS   3

/*+ The length of the buffer for reading in lines. +*/
#define BUFFLEN 2048

/* The interface information.  */

/*+ The total disk statistics +*/
ProcMeterOutput _outputs[N_OUTPUTS]=
{
 /*+ The disk blocks accessed per second +*/
 {
  /* char  name[16];         */ "Disk",
  /* char *description;      */ "The total number of disk blocks that are accessed per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks read per second +*/
 {
  /* char  name[16];         */ "Disk_Read",
  /* char *description;      */ "The number of disk blocks that are read per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks write per second +*/
 {
  /* char  name[16];         */ "Disk_Write",
  /* char *description;      */ "The number of disk blocks that are written per second.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 }
};

/*+ The per disk statistics +*/
ProcMeterOutput _disk_outputs[N_OUTPUTS]=
{
 /*+ The disk blocks accessed per second +*/
 {
  /* char  name[16];         */ "Disk%d",
  /* char *description;      */ "The total number of disk blocks that are accessed per second on disk %d.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks read per second +*/
 {
  /* char  name[16];         */ "Disk%d_Read",
  /* char *description;      */ "The number of disk blocks that are read per second on disk %d.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks write per second +*/
 {
  /* char  name[16];         */ "Disk%d_Write",
  /* char *description;      */ "The number of disk blocks that are written per second on disk %d.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 }
};

/*+ The per disk statistics +*/
ProcMeterOutput _disk_outputs_240[N_OUTPUTS]=
{
 /*+ The disk blocks accessed per second +*/
 {
  /* char  name[16];         */ "Disk_%s",
  /* char *description;      */ "The total number of disk blocks that are accessed per second on disk /dev/%s.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks read per second +*/
 {
  /* char  name[16];         */ "Disk_%s_R",
  /* char *description;      */ "The number of disk blocks that are read per second on disk /dev/%s.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 },
 /*+ The disk blocks write per second +*/
 {
  /* char  name[16];         */ "Disk_%s_W",
  /* char *description;      */ "The number of disk blocks that are written per second on disk /dev/%s.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 /s",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 25,
  /* char  graph_units[8];   */ "(%d/s)"
 }
};

/*+ The extra outputs with multiple disks +*/
ProcMeterOutput *disk_outputs=NULL;

/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];           */ "Stat-Disk",
 /* char *description;       */ "Disk usage statistics. [From /proc/stat]",
};


static unsigned long *current,*previous,*values[2];

static int ndisks=4;

static int kernel_version_130=0;
static int kernel_version_240=0;


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
 char line[BUFFLEN+1];
 int n=0;

 disk_outputs=(ProcMeterOutput*)malloc((N_OUTPUTS*ndisks)*sizeof(ProcMeterOutput));

 outputs=(ProcMeterOutput**)malloc((N_OUTPUTS*(ndisks+1)+1)*sizeof(ProcMeterOutput*));

 values[0]=(unsigned long*)malloc((N_OUTPUTS*(ndisks+1))*sizeof(unsigned long));
 values[1]=(unsigned long*)malloc((N_OUTPUTS*(ndisks+1))*sizeof(unsigned long));

 outputs[0]=NULL;

 current=values[0];
 previous=values[1];

 /* Verify the statistics from /proc/stat */

 f=fopen("/proc/stat","r");
 if(!f)
    fprintf(stderr,"ProcMeter(%s): Could not open '/proc/stat'.\n",__FILE__);
 else
   {
    if(!fgets(line,BUFFLEN,f)) /* cpu */
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/stat'.\n",__FILE__);
    else
      {
       while(line[0]=='c') /* kernel version > ~2.1.84 */
          fgets(line,BUFFLEN,f); /* cpu or disk or page */

       if(!strncmp(line,"disk ",5)) /* kernel version < ~2.4.0-test4 */
         {
          if(sscanf(line,"disk %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK],&current[N_OUTPUTS*2+DISK],
                                                &current[N_OUTPUTS*3+DISK],&current[N_OUTPUTS*4+DISK])==4)
            {
             int i,j;
             int read_avail=0,write_avail=0;

             current[DISK]=current[N_OUTPUTS*1+DISK]+current[N_OUTPUTS*2+DISK]+
                           current[N_OUTPUTS*3+DISK]+current[N_OUTPUTS*4+DISK];

             fgets(line,BUFFLEN,f); /* disk_* or page */
             while(line[0]=='d') /* kernel version > ~1.3.0 */
               {
                if(sscanf(line,"disk_rblk %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK_READ],&current[N_OUTPUTS*2+DISK_READ],
                                                           &current[N_OUTPUTS*3+DISK_READ],&current[N_OUTPUTS*4+DISK_READ])==4)
                  {
                   read_avail=1;
                   current[DISK_READ]=current[N_OUTPUTS*1+DISK_READ]+current[N_OUTPUTS*2+DISK_READ]+
                                      current[N_OUTPUTS*3+DISK_READ]+current[N_OUTPUTS*4+DISK_READ];
                  }
                if(sscanf(line,"disk_wblk %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK_WRITE],&current[N_OUTPUTS*2+DISK_WRITE],
                                                           &current[N_OUTPUTS*3+DISK_WRITE],&current[N_OUTPUTS*4+DISK_WRITE])==4)
                  {
                   write_avail=1;
                   current[DISK_WRITE]=current[N_OUTPUTS*1+DISK_WRITE]+current[N_OUTPUTS*2+DISK_WRITE]+
                                       current[N_OUTPUTS*3+DISK_WRITE]+current[N_OUTPUTS*4+DISK_WRITE];
                  }
                fgets(line,BUFFLEN,f); /* disk_* or page */
               }

             if(read_avail && write_avail)
                kernel_version_130=1;

             for(i=0;i<N_OUTPUTS;i++)
                for(j=0;j<ndisks;j++)
                  {
                   disk_outputs[i+j*N_OUTPUTS]=_disk_outputs[i];
                   sprintf(disk_outputs[i+j*N_OUTPUTS].name,_disk_outputs[i].name,j);
                   disk_outputs[i+j*N_OUTPUTS].description=(char*)malloc(strlen(_disk_outputs[i].description)+8);
                   sprintf(disk_outputs[i+j*N_OUTPUTS].description,_disk_outputs[i].description,j);
                  }

             for(i=0;i<N_OUTPUTS;i++)
                if(i==DISK || (i==DISK_READ && read_avail) || (i==DISK_WRITE && write_avail))
                   outputs[n++]=&_outputs[i];

             for(j=0;j<ndisks;j++)
                for(i=0;i<N_OUTPUTS;i++)
                   if(i==DISK || (i==DISK_READ && read_avail) || (i==DISK_WRITE && write_avail))
                      outputs[n++]=&disk_outputs[i+j*N_OUTPUTS];

             outputs[n]=NULL;
            }
          else
             fprintf(stderr,"ProcMeter(%s): Unexpected 'disk' line in '/proc/stat'.\n",__FILE__);
         }
       else
         {
          fgets(line,BUFFLEN,f); /* swap */

          fgets(line,BUFFLEN,f); /* intr */

          fgets(line,BUFFLEN,f); /* disk */

          if(!strncmp(line,"disk_io: ",9)) /* kernel version > ~2.4.0-test4 */
            {
             int maj,min,num=8,nm,nr;
             unsigned long d0,d1,d2,d3,d4;
             int i;
             DIR *dir;
             struct dirent *ent;
             struct stat buf;
             char here[PATH_MAX+1];

             kernel_version_240=1;
             ndisks=0;

             for(i=0;i<N_OUTPUTS;i++)
                outputs[n++]=&_outputs[i];

             current[DISK_READ]=0;
             current[DISK_WRITE]=0;

             getcwd(here,PATH_MAX);
             chdir("/dev");
             dir=opendir(".");

             while((nr=sscanf(line+num," (%d,%d):(%lu,%lu,%lu,%lu,%lu)%n",&maj,&min,&d0,&d1,&d2,&d3,&d4,&nm))==7 ||
                   (nr=sscanf(line+num," (%d,%d):(%lu,%lu,%lu,%lu)%n",&maj,&min,&d0,&d1,&d2,&d3,&nm))==6)
               {
                char diskname[9];

                kernel_version_240=nr;

                current[DISK_READ] +=d1;
                current[DISK_WRITE]+=d3;

                if(ndisks>=4)
                  {
                   disk_outputs=(ProcMeterOutput*)realloc((void*)disk_outputs,(N_OUTPUTS*(ndisks+1))*sizeof(ProcMeterOutput));

                   outputs=(ProcMeterOutput**)realloc((void*)outputs,(N_OUTPUTS*(ndisks+1+1)+1)*sizeof(ProcMeterOutput*));

                   values[0]=(unsigned long*)realloc((void*)values[0],(N_OUTPUTS*(ndisks+1+1))*sizeof(unsigned long));
                   values[1]=(unsigned long*)realloc((void*)values[1],(N_OUTPUTS*(ndisks+1+1))*sizeof(unsigned long));

                   current=values[0];
                   previous=values[1];
                  }

                diskname[0]=0;
                rewinddir(dir);
                while((ent=readdir(dir)))
                  {
                   if(!stat(ent->d_name,&buf) && S_ISBLK(buf.st_mode) &&
                      major(buf.st_rdev)==maj && minor(buf.st_rdev)==min)
                     {
                      strncpy(diskname,ent->d_name,8);
                      diskname[8]=0;
                      break;
                     }
                  }

                for(i=0;i<N_OUTPUTS;i++)
                  {
                   disk_outputs[i+ndisks*N_OUTPUTS]=_disk_outputs_240[i];
                   sprintf(disk_outputs[i+ndisks*N_OUTPUTS].name,_disk_outputs_240[i].name,diskname);
                   disk_outputs[i+ndisks*N_OUTPUTS].description=(char*)malloc(strlen(_disk_outputs_240[i].description)+8);
                   sprintf(disk_outputs[i+ndisks*N_OUTPUTS].description,_disk_outputs_240[i].description,diskname);
                  }

                for(i=0;i<N_OUTPUTS;i++)
                   outputs[n++]=&disk_outputs[i+ndisks*N_OUTPUTS];

                num+=nm;
                ndisks++;
               }

             closedir(dir);
             chdir(here);

             current[DISK]=current[DISK_READ]+current[DISK_WRITE];
            }
          else
             fprintf(stderr,"ProcMeter(%s): Unexpected 'disk' line in '/proc/stat'.\n",__FILE__);
         }
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
    char line[BUFFLEN+1];
    long *temp;

    temp=current;
    current=previous;
    previous=temp;

    f=fopen("/proc/stat","r");
    if(!f)
       return(-1);

    fgets(line,BUFFLEN,f); /* cpu */

    while(line[0]=='c') /* kernel version > ~2.1.84 */
       fgets(line,BUFFLEN,f); /* cpu or disk or page */

    if(!kernel_version_240) /* kernel version < ~2.4.0-test4 */
      {
       fgets(line,BUFFLEN,f); /* disk */

       if(!kernel_version_130)
         {
          sscanf(line,"disk %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK],&current[N_OUTPUTS*2+DISK],
                                             &current[N_OUTPUTS*3+DISK],&current[N_OUTPUTS*4+DISK]);
          current[DISK]=current[N_OUTPUTS*1+DISK]+current[N_OUTPUTS*2+DISK]+
                        current[N_OUTPUTS*3+DISK]+current[N_OUTPUTS*4+DISK];
         }

       fgets(line,BUFFLEN,f); /* disk_* or page */

       while(line[0]=='d') /* kernel version > ~1.3.0 */
         {
          if(sscanf(line,"disk_rblk %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK_READ],&current[N_OUTPUTS*2+DISK_READ],
                                                     &current[N_OUTPUTS*3+DISK_READ],&current[N_OUTPUTS*4+DISK_READ])==4)
             current[DISK_READ]=current[N_OUTPUTS*1+DISK_READ]+current[N_OUTPUTS*2+DISK_READ]+
                                current[N_OUTPUTS*3+DISK_READ]+current[N_OUTPUTS*4+DISK_READ];
          if(sscanf(line,"disk_wblk %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK_WRITE],&current[N_OUTPUTS*2+DISK_WRITE],
                                                    &current[N_OUTPUTS*3+DISK_WRITE],&current[N_OUTPUTS*4+DISK_WRITE])==4)
             current[DISK_WRITE]=current[N_OUTPUTS*1+DISK_WRITE]+current[N_OUTPUTS*2+DISK_WRITE]+
                                 current[N_OUTPUTS*3+DISK_WRITE]+current[N_OUTPUTS*4+DISK_WRITE];
          fgets(line,BUFFLEN,f); /* disk_* or page */
         }

       if(kernel_version_130)
         {
          int j;
          for(j=1;j<=ndisks;j++)
             current[N_OUTPUTS*j+DISK]=current[N_OUTPUTS*j+DISK_READ]+current[N_OUTPUTS*j+DISK_WRITE];
          current[DISK]=current[DISK_READ]+current[DISK_WRITE];
         }
      }
    else /* kernel version > ~2.4.0-test4 */
      {
       int num=8,nm,nr=0;
       int j=1;

       fgets(line,BUFFLEN,f); /* swap */

       fgets(line,BUFFLEN,f); /* intr */

       fgets(line,BUFFLEN,f); /* disk */

       current[DISK_READ]=0;
       current[DISK_WRITE]=0;

       while(1)
         {
          unsigned long d1,d3;

          if(kernel_version_240==6)
             nr=sscanf(line+num," (%*d,%*d):(%*u,%lu,%*u,%lu)%n",&d1,&d3,&nm);
          else if(kernel_version_240==7)
             nr=sscanf(line+num," (%*d,%*d):(%*u,%lu,%*u,%lu,%*u)%n",&d1,&d3,&nm);

          if(nr!=2)
             break;

          current[N_OUTPUTS*j+DISK_READ]=d1;
          current[N_OUTPUTS*j+DISK_WRITE]=d3;

          current[N_OUTPUTS*j+DISK]=d1+d3;

          current[DISK_READ] +=d1;
          current[DISK_WRITE]+=d3;

          num+=nm;
          j++;
         }

       current[DISK]=current[DISK_READ]+current[DISK_WRITE];
      }

    fclose(f);

    last=now;
   }

 for(i=0;i<(ndisks+1)*N_OUTPUTS;i++)
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
 int i,j;

 for(i=0;i<N_OUTPUTS;i++)
    for(j=0;j<ndisks;j++)
       free(disk_outputs[i+j*N_OUTPUTS].description);

 free(disk_outputs);

 free(outputs);

 free(values[0]);
 free(values[1]);
}
