/***************************************
  $Header: /home/amb/CVS/procmeter3/lcd/window.c,v 1.1 2002-11-30 19:22:48 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4.

  LCD driver daemon interface.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1997,98,99,2000,02 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include <netdb.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "procmeter.h"
#include "procmeterp.h"

#define DEBUG 0

/* Local functions */

void CreateGraph(Output output);
void CreateText(Output output);
void CreateBar(Output output);

void send_command(char *format, ...);

/*+ A list of the outputs that are currently visible. +*/
static Output *displayed=NULL;
static int ndisplayed=0;

/*+ The file descriptor of the socket for talking to the server. +*/
static int server;

/* Variables to hold the screen information. */
static int LCD_screen_width=20;         /*+ The width of the LCD in characters +*/
static int LCD_screen_height=4;         /*+ The height of the LCD in characters +*/
static int LCD_char_width=5;            /*+ The width of the character in pixels +*/
static int LCD_char_height=8;           /*+ The height of the character in pixels +*/
static int LCD_duration=8;              /*+ How long each display stays on screen. +*/
static int LCD_priority=128;            /*+ The display priority for each display. +*/


/*++++++++++++++++++++++++++++++++++++++
  Start the LCD driver daemon connection.

  int *argc The number of command line arguments.

  char **argv The actual command line arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void Start(int *argc,char **argv)
{
 char *string;
 char *host;
 int port=0;
 char reply[128],*p;
 int replylen;
 struct sockaddr_in sockaddr;
 struct hostent* hp;

 /* Get the host and port number */

 string=GetProcMeterRC("LCD","host");

 if(string)
    host=string;
 else
    host="localhost";

 string=GetProcMeterRC("LCD","port");

 if(string)
    port=atoi(string);
 if(port<=0 || port>65535)
    port=13666;

 fprintf(stderr,"ProcMeter: Connecting to LCDd on %s:%d.\n",host,port);

 string=GetProcMeterRC("LCD","duration");

 if(string)
    LCD_duration=atoi(string);

 string=GetProcMeterRC("LCD","priority");

 if(string)
    LCD_priority=atoi(string);

 /* Make the socket connection */

 sockaddr.sin_family=AF_INET;
 sockaddr.sin_port=htons((unsigned short)port);

 hp=gethostbyname(host);
 if(!hp)
   {
    unsigned long int addr=inet_addr(host);
    if(addr!=-1)
       hp=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);

    if(!hp)
      {
       fprintf(stderr,"ProcMeter: Cannot get host information for '%s'.\n",host);
       exit(1);
      }
   }

 memcpy((char*)&sockaddr.sin_addr,(char*)hp->h_addr,sizeof(sockaddr.sin_addr));

 server=socket(PF_INET,SOCK_STREAM,0);
 if(server==-1)
   {
    fprintf(stderr,"ProcMeter: Failed to create socket.\n");
    exit(1);
   }

 if(connect(server,(struct sockaddr*)&sockaddr,sizeof(sockaddr))==-1)
   {
    close(server);

    fprintf(stderr,"ProcMeter: Failed to connect socket to '%s' port '%d'.\n",host,port);
    exit(1);
   }

 /* Say hello and parse the result */

 write(server,"hello\n",sizeof("hello"));
 replylen=read(server,reply,128);

 reply[replylen-1]=0;

 fprintf(stderr,"ProcMeter: LCDd says '%s'.\n",reply);

 if(!(p=strstr(reply," wid ")))
   {
    fprintf(stderr,"ProcMeter: Cannot parse screen width from LCDd reply.\n");
    exit(1);
   }
 LCD_screen_width=atoi(p+4);

 if(!(p=strstr(reply," hgt ")))
   {
    fprintf(stderr,"ProcMeter: Cannot parse screen height from LCDd reply.\n");
    exit(1);
   }
 LCD_screen_height=atoi(p+4);

 if(!(p=strstr(reply," cellwid ")))
   {
    fprintf(stderr,"ProcMeter: Cannot parse character width from LCDd reply.\n");
    exit(1);
   }
 LCD_char_width=atoi(p+8);

 if(!(p=strstr(reply," cellhgt ")))
   {
    fprintf(stderr,"ProcMeter: Cannot parse character height from LCDd reply.\n");
    exit(1);
   }
 LCD_char_height=atoi(p+8);

 send_command("client_set -name ProcMeter3");
}


/*++++++++++++++++++++++++++++++++++++++
  Stop the LCD driver daemon connection.
  ++++++++++++++++++++++++++++++++++++++*/

void Stop(void)
{
 close(server);
}


/*++++++++++++++++++++++++++++++++++++++
  Sleep for the specified interval in seconds.

  time_t until The time to sleep until.
  ++++++++++++++++++++++++++++++++++++++*/

void Sleep(time_t until)
{
 struct timeval now;
 struct timespec delay;

 /* Sleep */

 gettimeofday(&now,NULL);

 delay.tv_sec=until-now.tv_sec-1;
 delay.tv_nsec=1000*(1000000-now.tv_usec);

 if(delay.tv_sec>=0)
    nanosleep(&delay,NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Add or remove an output

  Output output The output to be added or removed.
  ++++++++++++++++++++++++++++++++++++++*/

void AddRemoveOutput(Output output)
{
 if(output->output_widget)
   {
    int i,found=0;

    free(output->output_widget);
    output->output_widget=NULL;

    for(i=0;i<ndisplayed;i++)
       if(displayed[i]==output)
          found=1;
       else if(found)
          displayed[i-1]=displayed[i];
    ndisplayed--;
   }
 else
   {
    output->output_widget=(void*)calloc(LCD_screen_width*LCD_char_width,sizeof(int));
    output->first=2;

    displayed=(Output*)realloc((void*)displayed,sizeof(Output)*(ndisplayed+1));
    displayed[ndisplayed]=output;
    ndisplayed++;

    if(output->type==PROCMETER_GRAPH)
       CreateGraph(output);
    else if(output->type==PROCMETER_TEXT)
       CreateText(output);
    if(output->type==PROCMETER_BAR)
       CreateBar(output);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Create a graph output

  Output output The output to create.
  ++++++++++++++++++++++++++++++++++++++*/

void CreateGraph(Output output)
{
 int i;

 send_command("screen_add %s_g",output->output->name);

 send_command("screen_set %s_g -name %s -duration %d -priority %d",output->output->name,output->output->name,LCD_duration,LCD_priority);

 send_command("widget_add %s_g title title",output->output->name);

 send_command("widget_set %s_g title {%s}",output->output->name,output->label);

 for(i=1;i<=LCD_screen_width;i++)
   {
    send_command("widget_add %s_g %d vbar",output->output->name,i);

    send_command("widget_set %s_g %d %d %d 0",output->output->name,i,i,LCD_screen_height);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Create a text output

  Output output The output to create.
  ++++++++++++++++++++++++++++++++++++++*/

void CreateText(Output output)
{
 send_command("screen_add %s_t",output->output->name);

 send_command("screen_set %s_t -name %s -duration %d -priority %d",output->output->name,output->output->name,LCD_duration,LCD_priority);

 send_command("widget_add %s_t title title",output->output->name);

 send_command("widget_set %s_t title {%s}",output->output->name,output->label);

 send_command("widget_add %s_t value string",output->output->name);

 send_command("widget_set %s_t value %d %d ?",output->output->name,LCD_screen_width/2,1+LCD_screen_height/2);
}


/*++++++++++++++++++++++++++++++++++++++
  Create a bar output

  Output output The output to create.
  ++++++++++++++++++++++++++++++++++++++*/

void CreateBar(Output output)
{
 send_command("screen_add %s_b",output->output->name);

 send_command("screen_set %s_b -name %s -duration %d -priority %d",output->output->name,output->output->name,LCD_duration,LCD_priority);

 send_command("widget_add %s_b title title",output->output->name);

 send_command("widget_set %s_b title {%s}",output->output->name,output->label);

 send_command("widget_add %s_b value hbar",output->output->name);

 send_command("widget_set %s_b value 1 %d 0",output->output->name,1+LCD_screen_height/2);
}


/*++++++++++++++++++++++++++++++++++++++
  Update a graph output.

  Output output The output to update.

  short value The new value.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateGraph(Output output,short value)
{
 int i;
 int *values=(int*)output->output_widget;

 for(i=1;i<LCD_screen_width*LCD_char_width;i++)
    values[i-1]=values[i];

 values[i-1]=value;

 for(i=1;i<=LCD_screen_width;i++)
   {
    int j,sum=0,height;

    for(j=0;j<LCD_char_width;j++)
       sum+=values[(i-1)*LCD_char_width+j];

    sum/=LCD_char_width;

    height=sum/200;

    send_command("widget_set %s_g %d %d %d %d",output->output->name,i,i,LCD_screen_height,height);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Update a text output.

  Output output The output to update.

  char *value The new value.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateText(Output output,char *value)
{
 int len=strlen(value);
 int x=(LCD_screen_width-len)/2;

 send_command("widget_set %s_t value %d %d {%s}",output->output->name,x,1+LCD_screen_height/2,value);
}


/*++++++++++++++++++++++++++++++++++++++
  Update a bar output.

  Output output The output to update.

  short value The new value.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateBar(Output output,short value)
{
 int length;

 length=value/100;

 send_command("widget_set %s_b value 1 %d %d",output->output->name,1+LCD_screen_height/2,length);
}


/*++++++++++++++++++++++++++++++++++++++
  Write a string to the LCDproc server.

  char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void send_command(char *format, ...)
{
 static char buffer[256];
 int len;
 struct timeval tv;
 fd_set readfd;
 va_list ap;

 va_start(ap,format);
 len=vsprintf(buffer,format,ap);

 va_end(ap);

 buffer[len]='\n';

#if DEBUG
 buffer[len+1]=0;

 printf("-> %s",buffer);
#endif

 write(server,buffer,len+1);

 usleep(1000);

 FD_ZERO(&readfd);

 FD_SET(server,&readfd);

 tv.tv_sec=tv.tv_usec=0;

 if(select(server+1,&readfd,NULL,NULL,&tv)==0)
    return;

 len=read(server,buffer,256);

 buffer[len]=0;

#if DEBUG
 printf("<- %s",buffer);
#endif

 return;
}
