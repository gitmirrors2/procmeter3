/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/stat-disk.c,v 1.14 2008-05-05 18:45:36 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.5b.

  Disk statistics source file.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1998-2008 Andrew M. Bishop
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

/*+ The per disk statistics for kernels older than 2.4.0 +*/
ProcMeterOutput _disk_outputs_pre_240[N_OUTPUTS]=
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
ProcMeterOutput _disk_outputs[N_OUTPUTS]=
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


/* The line buffer */
static char *line=NULL;
static size_t length=0;

/* The current and previous information about the disk */
static unsigned long long *current,*previous,*values[2];

/* The information about the disks */
static int ndisks=4;
static unsigned *majors=NULL,*minors=NULL,*indexes=NULL;

/* The estimated kernel version based on the file format */
static int kernel_version_130=0;
static int kernel_version_240=0;
static int kernel_version_260=0;

/* A function to add a disk */
static int add_disk(char *devname);


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

 disk_outputs=(ProcMeterOutput*)malloc((N_OUTPUTS*ndisks)*sizeof(ProcMeterOutput));

 outputs=(ProcMeterOutput**)malloc((N_OUTPUTS*(ndisks+1)+1)*sizeof(ProcMeterOutput*));

 outputs[0]=NULL;

 /* Check for /proc/diskstats in kernel 2.6.x */

 f=fopen("/proc/diskstats","r");
 if(!f)
   {
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
          while(fgets_realloc(&line,&length,f)) /* cpu or disk or page or swap or intr or disk_io */
             if(line[0]=='d' && line[1]=='i' && line[2]=='s' && line[3]=='k')
                break;

          if(!line[0])
             fprintf(stderr,"ProcMeter(%s): Unexpected 'disk' line in '/proc/stat'.\n"
                     "    expected: 'disk ...' or 'disk_io ...'\n"
                     "    found:    EOF\n",__FILE__);
          else if(!strncmp(line,"disk ",5)) /* kernel version < ~2.4.0-test4 */
            {
             unsigned long long d1,d2,d3,d4;

             if(sscanf(line,"disk %llu %llu %llu %llu",&d1,&d2,&d3,&d4)==4)
               {
                int i,j;
                int read_avail=0,write_avail=0;
                int n=0;

                /* disk_* or page */
                while(fgets_realloc(&line,&length,f) && line[0]=='d') /* kernel version > ~1.3.0 */
                  {
                   if(sscanf(line,"disk_rblk %llu %llu %llu %llu",&d1,&d2,&d3,&d4)==4)
                      read_avail=1;
                   if(sscanf(line,"disk_wblk %llu %llu %llu %llu",&d1,&d2,&d3,&d4)==4)
                      write_avail=1;
                  }

                if(read_avail && write_avail)
                   kernel_version_130=1;

                for(i=0;i<N_OUTPUTS;i++)
                   for(j=0;j<ndisks;j++)
                     {
                      disk_outputs[i+j*N_OUTPUTS]=_disk_outputs_pre_240[i];
                      snprintf(disk_outputs[i+j*N_OUTPUTS].name, PROCMETER_NAME_LEN+1, _disk_outputs_pre_240[i].name, j);
                      disk_outputs[i+j*N_OUTPUTS].description=(char*)malloc(strlen(_disk_outputs_pre_240[i].description)+8);
                      sprintf(disk_outputs[i+j*N_OUTPUTS].description,_disk_outputs_pre_240[i].description,j);
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
             unsigned long long d1,d2,d3,d4,d5;
             DIR *devdir,*devdiscsdir,*devmapperdir;
             char devname[PATH_MAX];
             struct dirent *ent;
             struct stat buf;

             kernel_version_240=1;

             ndisks=0;

             devdir=opendir("/dev");
             devdiscsdir=opendir("/dev/discs");
             devmapperdir=opendir("/dev/mapper");

             while((nr=sscanf(line+num," (%d,%d):(%llu,%llu,%llu,%llu,%llu)%n",&maj,&idx,&d1,&d2,&d3,&d4,&d5,&nm))==7 ||
                   (nr=sscanf(line+num," (%d,%d):(%llu,%llu,%llu,%llu)%n",&maj,&idx,&d1,&d2,&d3,&d4,&nm))==6)
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

                if(devmapperdir)
                  {
                   rewinddir(devmapperdir);
                   while((ent=readdir(devmapperdir)))
                     {
                      sprintf(devname,"/dev/mapper/%s",ent->d_name);
                      if(!lstat(devname,&buf) && S_ISBLK(buf.st_mode) && !S_ISLNK(buf.st_mode) &&
                         major(buf.st_rdev)==maj && minor(buf.st_rdev)==min)
                         done=add_disk(devname);
                     }
                  }
                if(!done && devdiscsdir)
                  {
                   rewinddir(devdiscsdir);
                   while((ent=readdir(devdiscsdir)))
                     {
                      sprintf(devname,"/dev/discs/%s/disc",ent->d_name);
                      if(!lstat(devname,&buf) && S_ISBLK(buf.st_mode) && !S_ISLNK(buf.st_mode) &&
                         major(buf.st_rdev)==maj && minor(buf.st_rdev)==min)
                         done=add_disk(devname);
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
                         done=add_disk(devname);
                     }
                  }
                if(!done)
                  {
                   fprintf(stderr,"ProcMeter(%s): Cannot find disk in /dev, /dev/mapper, or /dev/discs for device %d:%d in '/proc/stat'.\n",__FILE__,maj,min);
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
   }
 else /* Found /proc/diskstats in kernel 2.6.x */
   {
    if(!fgets_realloc(&line,&length,f))
       fprintf(stderr,"ProcMeter(%s): Could not read '/proc/diskstats'.\n",__FILE__);
    else
      {
       int maj,min,nr;
       unsigned long long d1,d2,d3,d4,d5;
       DIR *devdir,*devdiscsdir,*devmapperdir;
       char devname[PATH_MAX];
       struct dirent *ent;
       struct stat buf;

       kernel_version_260=1;

       ndisks=0;

       devdir=opendir("/dev");
       devdiscsdir=opendir("/dev/discs");
       devmapperdir=opendir("/dev/mapper");

       do
         {
          int done=0;

          nr=sscanf(line,"%d %d %*s %llu %llu %llu %llu %llu",&maj,&min,&d1,&d2,&d3,&d4,&d5);

          if(nr<6)
             break;

          if(devmapperdir)
            {
              rewinddir(devmapperdir);
              while((ent=readdir(devmapperdir)))
                {
                  sprintf(devname,"/dev/mapper/%s",ent->d_name);
                  if(!lstat(devname,&buf) && S_ISBLK(buf.st_mode) && !S_ISLNK(buf.st_mode) &&
                     major(buf.st_rdev)==maj && minor(buf.st_rdev)==min)
                    done=add_disk(devname);
                }
            }
          if(!done && devdiscsdir)
            {
              rewinddir(devdiscsdir);
              while((ent=readdir(devdiscsdir)))
                {
                  sprintf(devname,"/dev/discs/%s/disc",ent->d_name);
                  if(!lstat(devname,&buf) && S_ISBLK(buf.st_mode) && !S_ISLNK(buf.st_mode) &&
                     major(buf.st_rdev)==maj && minor(buf.st_rdev)==min)
                    done=add_disk(devname);
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
                   done=add_disk(devname);
               }
            }
          if(!done)
            {
             fprintf(stderr,"ProcMeter(%s): Cannot find disk in /dev, /dev/mapper, or /dev/discs for device %d:%d in '/proc/diskstats'.\n",__FILE__,maj,min);
             continue;
            }
         }
       while(fgets_realloc(&line,&length,f));

       if(devdir)
          closedir(devdir);
       if(devdiscsdir)
          closedir(devdiscsdir);
      }
   }

 /* Add in the options from the config file. */

 if(options && (kernel_version_240 || kernel_version_260))
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

       if(!add_disk(l))
          fprintf(stderr,"ProcMeter(%s): Cannot find device for disk %s.\n",__FILE__,l);

       *r=pr;
       while(*r && *r==' ')
          r++;

       if(!*r)
          break;

       l=r;
      }
   }

 if(kernel_version_240 || kernel_version_260)
   {
    int i,j,n=0;

    for(i=0;i<N_OUTPUTS;i++)
       outputs[n++]=&_outputs[i];

    for(j=0;j<ndisks;j++)
       for(i=0;i<N_OUTPUTS;i++)
          outputs[n++]=&disk_outputs[i+j*N_OUTPUTS];

    outputs[n]=NULL;
   }

 values[0]=(unsigned long long*)calloc((N_OUTPUTS*(ndisks+1)),sizeof(unsigned long long));
 values[1]=(unsigned long long*)calloc((N_OUTPUTS*(ndisks+1)),sizeof(unsigned long long));

 current=values[0];
 previous=values[1];

 return(outputs);
}


/*++++++++++++++++++++++++++++++++++++++
  Add a disk (kernel version > ~2.4.0-test4).

  int add_disk Returns 1 if a disk was added.

  char *devname The name of the device to add.
  ++++++++++++++++++++++++++++++++++++++*/

static int add_disk(char *devname)
{
 struct stat buf;
 char *diskname=devname+strlen(devname);
 int maj,min,idx=0,i;

 if(stat(devname,&buf) || !S_ISBLK(buf.st_mode))
    return(0);

 maj=major(buf.st_rdev);
 min=minor(buf.st_rdev);

 /* This switch statement is the one in /usr/include/linux/genhd.h for kernels < 2.6.0 */

 if(kernel_version_240)
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

 majors =(unsigned*)realloc((void*)majors ,(ndisks+1)*sizeof(unsigned));
 minors =(unsigned*)realloc((void*)minors ,(ndisks+1)*sizeof(unsigned));
 indexes=(unsigned*)realloc((void*)indexes,(ndisks+1)*sizeof(unsigned));

 if(ndisks>=3)
   {
    disk_outputs=(ProcMeterOutput*)realloc((void*)disk_outputs,(N_OUTPUTS*(ndisks+1))*sizeof(ProcMeterOutput));

    outputs=(ProcMeterOutput**)realloc((void*)outputs,(N_OUTPUTS*(ndisks+1+1)+1)*sizeof(ProcMeterOutput*));
   }

 for(i=0;i<N_OUTPUTS;i++)
   {
    disk_outputs[i+ndisks*N_OUTPUTS]=_disk_outputs[i];
    snprintf(disk_outputs[i+ndisks*N_OUTPUTS].name, PROCMETER_NAME_LEN, _disk_outputs[i].name, diskname);
    disk_outputs[i+ndisks*N_OUTPUTS].description=(char*)malloc(strlen(_disk_outputs[i].description)+strlen(devname));
    sprintf(disk_outputs[i+ndisks*N_OUTPUTS].description,_disk_outputs[i].description,devname);
   }

 majors[ndisks] =maj;
 minors[ndisks] =min;
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
    unsigned long long *temp;

    temp=current;
    current=previous;
    previous=temp;

    if(kernel_version_260)
      {
       f=fopen("/proc/diskstats","r");
       if(!f)
          return(-1);

       current[DISK_READ]=0;
       current[DISK_WRITE]=0;

       while(fgets_realloc(&line,&length,f))
         {
          int maj,min,nr;
          unsigned long long d1,d2,d3,d4,d5,dr,dw;
          int j;

          nr=sscanf(line,"%d %d %*s %llu %llu %llu %llu %llu",&maj,&min,&d1,&d2,&d3,&d4,&d5);

          if(nr==7)
             dr=d1,dw=d5;
          else if(nr==6)
             dr=d1,dw=d3;
          else
             break;

          for(j=0;j<ndisks;j++)
             if(majors[j]==maj && minors[j]==min)
               {
                current[N_OUTPUTS*(j+1)+DISK_READ]=dr;
                current[N_OUTPUTS*(j+1)+DISK_WRITE]=dw;

                current[N_OUTPUTS*(j+1)+DISK]=dr+dw;
               }

          if(nr==7) /* Count the disks, not the partitions */
            {
             current[DISK_READ] +=dr;
             current[DISK_WRITE]+=dw;
            }
         }

       current[DISK]=current[DISK_READ]+current[DISK_WRITE];

       fclose(f);
      }
    else
      {
       f=fopen("/proc/stat","r");
       if(!f)
          return(-1);

       while(fgets_realloc(&line,&length,f)) /* cpu or disk or page or swap or intr or disk_io */
          if(line[0]=='d' && line[1]=='i' && line[2]=='s' && line[3]=='k')
             break;

       if(!kernel_version_240) /* kernel version < ~2.4.0-test4 */
         {
          if(!kernel_version_130)
            {
             sscanf(line,"disk %llu %llu %llu %llu",&current[N_OUTPUTS*1+DISK],&current[N_OUTPUTS*2+DISK],
                                                    &current[N_OUTPUTS*3+DISK],&current[N_OUTPUTS*4+DISK]);
             current[DISK]=current[N_OUTPUTS*1+DISK]+current[N_OUTPUTS*2+DISK]+
                           current[N_OUTPUTS*3+DISK]+current[N_OUTPUTS*4+DISK];
            }

          /* disk_* or page */
          while(fgets_realloc(&line,&length,f) && line[0]=='d') /* kernel version > ~1.3.0 */
            {
             if(sscanf(line,"disk_rblk %llu %llu %llu %llu",&current[N_OUTPUTS*1+DISK_READ],&current[N_OUTPUTS*2+DISK_READ],
                                                            &current[N_OUTPUTS*3+DISK_READ],&current[N_OUTPUTS*4+DISK_READ])==4)
                current[DISK_READ]=current[N_OUTPUTS*1+DISK_READ]+current[N_OUTPUTS*2+DISK_READ]+
                                   current[N_OUTPUTS*3+DISK_READ]+current[N_OUTPUTS*4+DISK_READ];
             if(sscanf(line,"disk_wblk %llu %llu %llu %llu",&current[N_OUTPUTS*1+DISK_WRITE],&current[N_OUTPUTS*2+DISK_WRITE],
                                                            &current[N_OUTPUTS*3+DISK_WRITE],&current[N_OUTPUTS*4+DISK_WRITE])==4)
                current[DISK_WRITE]=current[N_OUTPUTS*1+DISK_WRITE]+current[N_OUTPUTS*2+DISK_WRITE]+
                                    current[N_OUTPUTS*3+DISK_WRITE]+current[N_OUTPUTS*4+DISK_WRITE];
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
             unsigned long long d1,d3;

             if(kernel_version_240==6)
                nr=sscanf(line+num," (%d,%d):(%*u,%llu,%*u,%llu)%n",&maj,&idx,&d1,&d3,&nm);
             else if(kernel_version_240==7)
                nr=sscanf(line+num," (%d,%d):(%*u,%llu,%*u,%llu,%*u)%n",&maj,&idx,&d1,&d3,&nm);

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
            }

          current[DISK]=current[DISK_READ]+current[DISK_WRITE];
         }

       fclose(f);
      }

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

 if(line)
    free(line);
}
