/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/stat-disk.c,v 1.10 2003-06-21 18:41:05 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4.

  Disk statistics source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998,99,2000,2002 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <linux/major.h>

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
  /* char  name[];          */ "Disk",
  /* char *description;     */ "The total number of disk blocks that are accessed per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The disk blocks read per second +*/
 {
  /* char  name[];          */ "Disk_Read",
  /* char *description;     */ "The number of disk blocks that are read per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The disk blocks write per second +*/
 {
  /* char  name[];          */ "Disk_Write",
  /* char *description;     */ "The number of disk blocks that are written per second.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 }
};

/*+ The per disk statistics +*/
ProcMeterOutput _disk_outputs[N_OUTPUTS]=
{
 /*+ The disk blocks accessed per second +*/
 {
  /* char  name[];          */ "Disk%d",
  /* char *description;     */ "The total number of disk blocks that are accessed per second on disk %d.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The disk blocks read per second +*/
 {
  /* char  name[];          */ "Disk%d_Read",
  /* char *description;     */ "The number of disk blocks that are read per second on disk %d.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The disk blocks write per second +*/
 {
  /* char  name[];          */ "Disk%d_Write",
  /* char *description;     */ "The number of disk blocks that are written per second on disk %d.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 }
};

/*+ The per disk statistics +*/
ProcMeterOutput _disk_outputs_240[N_OUTPUTS]=
{
 /*+ The disk blocks accessed per second +*/
 {
  /* char  name[];          */ "Disk_%s",
  /* char *description;     */ "The total number of disk blocks that are accessed per second on disk %s.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The disk blocks read per second +*/
 {
  /* char  name[];          */ "Disk_%s_R",
  /* char *description;     */ "The number of disk blocks that are read per second on disk %s.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 },
 /*+ The disk blocks write per second +*/
 {
  /* char  name[];          */ "Disk_%s_W",
  /* char *description;     */ "The number of disk blocks that are written per second on disk %s.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 /s",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 25,
  /* char  graph_units[];   */ "(%d/s)"
 }
};

/*+ The extra outputs with multiple disks +*/
ProcMeterOutput *disk_outputs=NULL;

/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[];            */ "Stat-Disk",
 /* char *description;      */ "Disk usage statistics. [From /proc/stat]",
};


static int add_disk_240(char *devname);

static unsigned long *current,*previous,*values[2];

static unsigned *majors=NULL,*indexes=NULL;

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
 char line[BUFFLEN+1],*l;
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
    l=fgets(line,256,f); /* cpu */
    if(!l)
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/stat'.\n",__FILE__);
    else
      {
       while(l && !(line[0]=='d' && line[1]=='i' && line[2]=='s' && line[3]=='k'))
          l=fgets(line,BUFFLEN,f); /* cpu or disk or page or swap or intr or disk_io */

       if(!l)
          fprintf(stderr,"ProcMeter(%s): Unexpected 'disk' line in '/proc/stat'.\n"
                         "    expected: 'disk ...' or 'disk_io ...'\n"
                         "    found:    EOF",__FILE__);
       else if(!strncmp(line,"disk ",5)) /* kernel version < ~2.4.0-test4 */
         {
          if(sscanf(line,"disk %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK],&current[N_OUTPUTS*2+DISK],
                                                &current[N_OUTPUTS*3+DISK],&current[N_OUTPUTS*4+DISK])==4)
            {
             int i,j;
             int read_avail=0,write_avail=0;

             current[DISK]=current[N_OUTPUTS*1+DISK]+current[N_OUTPUTS*2+DISK]+
                           current[N_OUTPUTS*3+DISK]+current[N_OUTPUTS*4+DISK];

             l=fgets(line,BUFFLEN,f); /* disk_* or page */
             while(l && line[0]=='d') /* kernel version > ~1.3.0 */
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
                l=fgets(line,BUFFLEN,f); /* disk_* or page */
               }

             if(read_avail && write_avail)
                kernel_version_130=1;

             for(i=0;i<N_OUTPUTS;i++)
                for(j=0;j<ndisks;j++)
                  {
                   disk_outputs[i+j*N_OUTPUTS]=_disk_outputs[i];
                   snprintf(disk_outputs[i+j*N_OUTPUTS].name, PROCMETER_NAME_LEN+1, _disk_outputs[i].name, j);
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
             fprintf(stderr,"ProcMeter(%s): Unexpected 'disk' line in '/proc/stat'.\n"
                            "    expected: 'disk %%u %%u %%u %%u'\n"
                            "    found:    %s",__FILE__,line);
         }
       else if(!strncmp(line,"disk_io: ",9)) /* kernel version > ~2.4.0-test4 */
         {
          int maj,min,idx,num=8,nm,nr;
          unsigned long d0,d1,d2,d3,d4;
          DIR *devdir,*devdiscsdir;
          char devname[PATH_MAX];
          struct dirent *ent;
          struct stat buf;

          kernel_version_240=1;

          majors =(unsigned*)malloc((ndisks+1)*sizeof(unsigned));
          indexes=(unsigned*)malloc((ndisks+1)*sizeof(unsigned));

          ndisks=0;

          devdir=opendir("/dev");
          devdiscsdir=opendir("/dev/discs");

          while((nr=sscanf(line+num," (%d,%d):(%lu,%lu,%lu,%lu,%lu)%n",&maj,&idx,&d0,&d1,&d2,&d3,&d4,&nm))==7 ||
                (nr=sscanf(line+num," (%d,%d):(%lu,%lu,%lu,%lu)%n",&maj,&idx,&d0,&d1,&d2,&d3,&nm))==6)
            {
             int done=0;

             num+=nm;

             kernel_version_240=nr;

             /* This switch statement is the inverse of the one in /usr/include/linux/genhd.h */

             switch (maj)
               {
               case DAC960_MAJOR+0:
                min=idx<<3;
                break;
               case SCSI_DISK0_MAJOR:
                min=idx<<4;
                break;
               case IDE0_MAJOR:      /* same as HD_MAJOR */
               case XT_DISK_MAJOR:
                min=idx<<6;
                break;
               case IDE1_MAJOR:
                min=(idx-2)<<6;
                break;
               default:
                min=0;
               }

             if(devdiscsdir)
               {
                rewinddir(devdiscsdir);
                while((ent=readdir(devdiscsdir)))
                  {
                   sprintf(devname,"/dev/discs/%s/disc",ent->d_name);
                   if(!lstat(devname,&buf) && S_ISBLK(buf.st_mode) && !S_ISLNK(buf.st_mode) &&
                      major(buf.st_rdev)==maj && minor(buf.st_rdev)==min)
                      done=add_disk_240(devname);
                  }
               }
             if(!done && devdir)
               {
                rewinddir(devdir);
                while((ent=readdir(devdir)))
                  {
                   sprintf(devname,"/dev/%s",ent->d_name);
                   if(!lstat(devname,&buf) && S_ISBLK(buf.st_mode) && !S_ISLNK(buf.st_mode) &&
                      major(buf.st_rdev)==maj && minor(buf.st_rdev)==min)
                      done=add_disk_240(devname);
                  }
               }
             if(!done)
               {
                fprintf(stderr,"ProcMeter(%s): Cannot find disk in /dev or /dev/discs for device %d:%d in '/proc/stat'.\n",__FILE__,maj,min);
                continue;
               }
            }

          if(devdir)
             closedir(devdir);
          if(devdiscsdir)
             closedir(devdiscsdir);
         }
       else
          fprintf(stderr,"ProcMeter(%s): Unexpected 'disk' line in '/proc/stat'.\n"
                         "    expected: 'disk ...' or 'disk_io ...'\n"
                         "    found:    %s",__FILE__,line);
      }

    fclose(f);
   }

 /* Add in the options from the config file. */

 if(options && kernel_version_240)
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

       if(!add_disk_240(l))
          fprintf(stderr,"ProcMeter(%s): Cannot find device for disk %s.\n",__FILE__,l);

       *r=pr;
       while(*r && *r==' ')
          r++;

       if(!*r)
          break;

       l=r;
      }
   }

 if(kernel_version_240)
   {
    int i,j;

    for(i=0;i<N_OUTPUTS;i++)
       outputs[n++]=&_outputs[i];

    for(j=0;j<ndisks;j++)
       for(i=0;i<N_OUTPUTS;i++)
          outputs[n++]=&disk_outputs[i+j*N_OUTPUTS];

    outputs[n]=NULL;

    current[DISK]=current[DISK_READ]=current[DISK_WRITE]=0;
    for(j=0;j<ndisks;j++)
       current[DISK+(j+1)*N_OUTPUTS]=current[DISK_READ+(j+1)*N_OUTPUTS]=current[DISK_WRITE+(j+1)*N_OUTPUTS]=0;
   }

 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a disk (kernel version > ~2.4.0-test4).

  int add_disk_240 Returns 1 if a disk was added.

  char *devname The name of the device to add.
  ++++++++++++++++++++++++++++++++++++++*/

static int add_disk_240(char *devname)
{
 struct stat buf;
 char *diskname=devname+strlen(devname);
 int maj,min,idx,i;

 if(stat(devname,&buf) || !S_ISBLK(buf.st_mode))
    return(0);

 maj=major(buf.st_rdev);
 min=minor(buf.st_rdev);

 /* This switch statement is the one in /usr/include/linux/genhd.h */

 switch (maj)
   {
   case DAC960_MAJOR+0:
    idx = (min & 0x00f8) >> 3;
    break;
   case SCSI_DISK0_MAJOR:
    idx = (min & 0x00f0) >> 4;
    break;
   case IDE0_MAJOR:	/* same as HD_MAJOR */
   case XT_DISK_MAJOR:
    idx = (min & 0x0040) >> 6;
    break;
   case IDE1_MAJOR:
    idx = ((min & 0x0040) >> 6) + 2;
    break;
   default:
    idx=0;
   }

 while(diskname>devname && *diskname!='/')
    diskname--;

 if(diskname==devname)
    return(0);

 diskname++;

 if(ndisks>=3)
   {
    disk_outputs=(ProcMeterOutput*)realloc((void*)disk_outputs,(N_OUTPUTS*(ndisks+1))*sizeof(ProcMeterOutput));

    outputs=(ProcMeterOutput**)realloc((void*)outputs,(N_OUTPUTS*(ndisks+1+1)+1)*sizeof(ProcMeterOutput*));

    values[0]=(unsigned long*)realloc((void*)values[0],(N_OUTPUTS*(ndisks+1+1))*sizeof(unsigned long));
    values[1]=(unsigned long*)realloc((void*)values[1],(N_OUTPUTS*(ndisks+1+1))*sizeof(unsigned long));

    current=values[0];
    previous=values[1];

    majors =(unsigned*)realloc((void*)majors ,(ndisks+1)*sizeof(unsigned));
    indexes=(unsigned*)realloc((void*)indexes,(ndisks+1)*sizeof(unsigned));
   }

 for(i=0;i<N_OUTPUTS;i++)
   {
    disk_outputs[i+ndisks*N_OUTPUTS]=_disk_outputs_240[i];
    snprintf(disk_outputs[i+ndisks*N_OUTPUTS].name, PROCMETER_NAME_LEN, _disk_outputs_240[i].name, diskname);
    disk_outputs[i+ndisks*N_OUTPUTS].description=(char*)malloc(strlen(_disk_outputs_240[i].description)+strlen(devname));
    sprintf(disk_outputs[i+ndisks*N_OUTPUTS].description,_disk_outputs_240[i].description,devname);
   }

 majors[ndisks] =maj;
 indexes[ndisks]=idx;

 ndisks++;

 return(1);
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
    char line[BUFFLEN+1],*l;
    long *temp;

    temp=current;
    current=previous;
    previous=temp;

    f=fopen("/proc/stat","r");
    if(!f)
       return(-1);

    while((l=fgets(line,BUFFLEN,f))) /* cpu or disk or page or swap or intr or disk_io */
       if(line[0]=='d' && line[1]=='i' && line[2]=='s' && line[3]=='k')
          break;

    if(!kernel_version_240) /* kernel version < ~2.4.0-test4 */
      {
       if(!kernel_version_130)
         {
          sscanf(line,"disk %lu %lu %lu %lu",&current[N_OUTPUTS*1+DISK],&current[N_OUTPUTS*2+DISK],
                                             &current[N_OUTPUTS*3+DISK],&current[N_OUTPUTS*4+DISK]);
          current[DISK]=current[N_OUTPUTS*1+DISK]+current[N_OUTPUTS*2+DISK]+
                        current[N_OUTPUTS*3+DISK]+current[N_OUTPUTS*4+DISK];
         }

       l=fgets(line,BUFFLEN,f); /* disk_* or page */

       while(l && line[0]=='d') /* kernel version > ~1.3.0 */
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
       int j;

       current[DISK_READ]=0;
       current[DISK_WRITE]=0;

       while(1)
         {
          unsigned maj,idx;
          unsigned long d1,d3;

          if(kernel_version_240==6)
             nr=sscanf(line+num," (%d,%d):(%*u,%lu,%*u,%lu)%n",&maj,&idx,&d1,&d3,&nm);
          else if(kernel_version_240==7)
             nr=sscanf(line+num," (%d,%d):(%*u,%lu,%*u,%lu,%*u)%n",&maj,&idx,&d1,&d3,&nm);

          if(nr!=4)
             break;

          for(j=0;j<ndisks;j++)
             if(majors[j]==maj && indexes[j]==idx)
               {
                current[N_OUTPUTS*(j+1)+DISK_READ]=d1;
                current[N_OUTPUTS*(j+1)+DISK_WRITE]=d3;

                current[N_OUTPUTS*(j+1)+DISK]=d1+d3;
               }

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
