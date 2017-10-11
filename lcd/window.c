/***************************************
  $Header: /home/amb/CVS/procmeter3/lcd/window.c,v 1.6 2010-02-28 10:07:46 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.5d.

  LCD driver daemon interface.
  ******************/ /******************
  Written by Andrew M. Bishop

  This file Copyright 1997-2010, 2017 Andrew M. Bishop
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

/*+ A fake widget for graphs. +*/
typedef struct _LCD_Graph
{
 unsigned int    grid_min;      /*+ The minimum number of graph grid lines. +*/
 unsigned int    grid_num;      /*+ The current number of graph grid lines. +*/

 unsigned short* data;          /*+ The data for the graph. +*/
 unsigned int    data_num;      /*+ The number of data points. +*/
 int             data_index;    /*+ An index into the array for the new value. +*/

 unsigned short* bars;          /*+ The bars for the graph. +*/
 unsigned int    bars_num;      /*+ The number of bars. +*/
 unsigned short  bar_max;       /*+ The maximum data value. +*/
}
LCD_Graph;

/*+ A fake widget for text outputs. +*/
typedef struct _LCD_Text
{
 int dummy;                     /*+ Not used. +*/
}
LCD_Text;

/*+ A fake widget for bar charts. +*/
typedef struct _LCD_Bar
{
 unsigned int    grid_min;      /*+ The minimum number of bar grid lines. +*/
 unsigned int    grid_num;      /*+ The current number of bar grid lines. +*/

 unsigned short  data[8];       /*+ The data for the bar. +*/
 unsigned short  data_index;    /*+ A pointer into the array +*/
 unsigned long   data_sum;      /*+ The average value of the last 8 data points. +*/
}
LCD_Bar;

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
static char *LCD_priority=NULL;         /*+ The display priority for each display. +*/


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
   {
    LCD_priority=(char*)malloc(strlen(string)+1);
    strcpy(LCD_priority,string);
   }

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
    fprintf(stderr,"ProcMeter: Failed to connect socket to '%s' port '%d'.\n",host,port);
    exit(1);
   }

 /* Say hello and parse the result */

 if(write(server,"hello\n",sizeof("hello"))!=sizeof("hello"))
   {
    fprintf(stderr,"ProcMeter: Failed to write socket to '%s' port '%d'.\n",host,port);
    exit(1);
   }

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

    if(output->type==PROCMETER_GRAPH)
      {
       LCD_Graph *graph=(LCD_Graph*)output->output_widget;

       free(graph->data);
       free(graph->bars);
      }

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
    if(output->type==PROCMETER_GRAPH)
       output->output_widget=(void*)malloc(sizeof(LCD_Graph));
    else if(output->type==PROCMETER_TEXT)
       output->output_widget=(void*)malloc(sizeof(LCD_Text));
    else if(output->type==PROCMETER_BAR)
       output->output_widget=(void*)malloc(sizeof(LCD_Bar));

    output->first=2;

    displayed=(Output*)realloc((void*)displayed,sizeof(Output)*(ndisplayed+1));
    displayed[ndisplayed]=output;
    ndisplayed++;

    if(output->type==PROCMETER_GRAPH)
       CreateGraph(output);
    else if(output->type==PROCMETER_TEXT)
       CreateText(output);
    else if(output->type==PROCMETER_BAR)
       CreateBar(output);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Create a graph output

  Output output The output to create.
  ++++++++++++++++++++++++++++++++++++++*/

void CreateGraph(Output output)
{
 char *string;
 LCD_Graph *graph=(LCD_Graph*)output->output_widget;
 int i;
 Output *outputp;
 Module *modulep,module=NULL;

 for(modulep=Modules;*modulep;modulep++)
   {
    for(outputp=(*modulep)->outputs;*outputp;outputp++)
       if(output==*outputp)
         {
          module=*modulep;
          break;
         }
    if(module)
       break;
   }

 if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-min")) ||
     (string=GetProcMeterRC(module->module->name,"grid-min")) ||
     (string=GetProcMeterRC("resources","grid-min"))))
    graph->grid_min=atoi(string);
 else
    graph->grid_min=1;

 graph->grid_num=-1;            /* force update */

 graph->bars_num=LCD_screen_width-4;
 graph->bars=(unsigned short*)calloc(graph->bars_num,sizeof(unsigned short));
 graph->bar_max=-1;             /* force update */

 graph->data_num=graph->bars_num*LCD_char_width;
 graph->data=(unsigned short*)calloc(graph->data_num,sizeof(unsigned short));
 graph->data_index=0;

 send_command("screen_add %s_g",output->output->name);
 send_command("screen_set %s_g -name {%s}",output->output->name,output->output->name);
 send_command("screen_set %s_g -duration %d",output->output->name,LCD_duration);
 if(LCD_priority)
    send_command("screen_set %s_g -priority %s",output->output->name,LCD_priority);

 send_command("widget_add %s_g title title",output->output->name);
 send_command("widget_set %s_g title {%s}",output->output->name,output->label);

 send_command("widget_add %s_g min string",output->output->name);
 send_command("widget_set %s_g min %d %d {  0}",output->output->name,graph->bars_num+1,LCD_screen_height);

 send_command("widget_add %s_g max string",output->output->name);
 send_command("widget_set %s_g max %d 2 {  ?}",output->output->name,graph->bars_num+1);

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
 send_command("screen_set %s_t -name {%s}",output->output->name,output->output->name);
 send_command("screen_set %s_t -duration %d",output->output->name,LCD_duration);
 if(LCD_priority)
    send_command("screen_set %s_t -priority %s",output->output->name,LCD_priority);

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
 LCD_Bar *bar=(LCD_Bar*)output->output_widget;
 char *string;
 Output *outputp;
 Module *modulep,module=NULL;
 int i;

 for(modulep=Modules;*modulep;modulep++)
   {
    for(outputp=(*modulep)->outputs;*outputp;outputp++)
       if(output==*outputp)
         {
          module=*modulep;
          break;
         }
    if(module)
       break;
   }

 if(((string=GetProcMeterRC2(module->module->name,output->output->name,"grid-min")) ||
     (string=GetProcMeterRC(module->module->name,"grid-min")) ||
     (string=GetProcMeterRC("resources","grid-min"))))
    bar->grid_min=atoi(string);
 else
    bar->grid_min=1;

 bar->grid_num=-1;              /* force update */

 for(i=0;i<8;i++)
    bar->data[i]=0;
 bar->data_sum=0;
 bar->data_index=0;

 send_command("screen_add %s_b",output->output->name);
 send_command("screen_set %s_b -name {%s}",output->output->name,output->output->name);
 send_command("screen_set %s_b -duration %d",output->output->name,LCD_duration);
 if(LCD_priority)
    send_command("screen_set %s_b -priority %s",output->output->name,LCD_priority);

 send_command("widget_add %s_b title title",output->output->name);
 send_command("widget_set %s_b title {%s}",output->output->name,output->label);

 send_command("widget_add %s_b value hbar",output->output->name);
 send_command("widget_set %s_b value 1 %d 0",output->output->name,1+LCD_screen_height/2);

 send_command("widget_add %s_b min string",output->output->name);
 if(LCD_screen_height>2)
    send_command("widget_set %s_b min 1 %d 0",output->output->name,LCD_screen_height);

 send_command("widget_add %s_b max string",output->output->name);
 send_command("widget_set %s_b max %d %d {  ?}",output->output->name,LCD_screen_width-4,LCD_screen_height);

 send_command("widget_add %s_b average string",output->output->name);
 if(LCD_screen_height>2)
    send_command("widget_set %s_b average 1 2 { }",output->output->name);
}


/*++++++++++++++++++++++++++++++++++++++
  Update a graph output.

  Output output The output to update.

  short value The new value.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateGraph(Output output,short value)
{
 LCD_Graph *graph=(LCD_Graph*)output->output_widget;
 unsigned short bar_max=0,scale1,scale2;
 int i;

 /* Add the new data point */

 graph->data[graph->data_index]=value;

 graph->data_index=(graph->data_index+1)%graph->data_num;

 /* Calculate the individual vertical bars */

 for(i=0;i<graph->bars_num;i++)
   {
    int j,sum=0;

    for(j=0;j<LCD_char_width;j++)
      {
       int index=(graph->data_index+i*LCD_char_width+j)%graph->data_num;
       sum+=graph->data[index];
      }

    graph->bars[i]=sum/LCD_char_width;

    if(graph->bars[i]>bar_max)
       bar_max=graph->bars[i];
   }

 /* Update the vertical scaling */

 if(bar_max!=graph->bar_max)
   {
    int new_grid_num=(bar_max+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;
    int maxval;
    char maxstr[6];

    if(new_grid_num<graph->grid_min)
       new_grid_num=graph->grid_min;

    graph->bar_max=bar_max;

    if(new_grid_num!=graph->grid_num)
       graph->grid_num=new_grid_num;

    maxval=graph->grid_num*output->output->graph_scale;
    if(maxval<1000)
       sprintf(maxstr,"%3d",maxval);
    else if(maxval<1000000)
       sprintf(maxstr,"%3dk",maxval/1000);
    else
       sprintf(maxstr,"%3dM",maxval/1000000);

    send_command("widget_set %s_g max %d 2 {%s}",output->output->name,graph->bars_num+1,maxstr);
   }

 /* Set the values */

 scale1=(LCD_screen_height-1)*LCD_char_height;
 scale2=PROCMETER_GRAPH_SCALE*graph->grid_num;

 for(i=0;i<graph->bars_num;i++)
   {
    short height;

    if(scale2>0)
       height=graph->bars[i]*scale1/scale2;
    else
       height=0;

    send_command("widget_set %s_g %d %d %d %d",output->output->name,i+1,i+1,LCD_screen_height,height);
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

 send_command("widget_set %s_t value %d %d {%s}",output->output->name,x+1,1+LCD_screen_height/2,value);
}


/*++++++++++++++++++++++++++++++++++++++
  Update a bar output.

  Output output The output to update.

  short value The new value.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateBar(Output output,short value)
{
 LCD_Bar *bar=(LCD_Bar*)output->output_widget;
 unsigned short old_value,new_grid_num,length,scale1,scale2;

 /* Add the new data point */

 bar->data_index++;
 if(bar->data_index==8)
    bar->data_index=0;

 old_value=bar->data[bar->data_index];
 bar->data[bar->data_index]=value;

 bar->data_sum=(bar->data_sum>>1)+value-(old_value>>8);

 /* Update the number of grid lines */

 if((bar->data_sum/2)>value)
    new_grid_num=((bar->data_sum/2)+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;
 else
    new_grid_num=(value+(PROCMETER_GRAPH_SCALE-1))/PROCMETER_GRAPH_SCALE;

 if(new_grid_num<bar->grid_min)
    new_grid_num=bar->grid_min;

 if(new_grid_num!=bar->grid_num)
   {
    int maxval;
    char maxstr[6];

    bar->grid_num=new_grid_num;

    maxval=bar->grid_num*output->output->graph_scale;
    if(maxval<1000)
       sprintf(maxstr,"%3d",maxval);
    else if(maxval<1000000)
       sprintf(maxstr,"%3dk",maxval/1000);
    else
       sprintf(maxstr,"%3dM",maxval/1000000);

    send_command("widget_set %s_b max %d %d {%s}",output->output->name,LCD_screen_width-4,LCD_screen_height,maxstr);
   }

 /* Set the values */

 if(LCD_screen_height==2)
    scale1=(LCD_screen_width-4)*LCD_char_width;
 else
    scale1=LCD_screen_width*LCD_char_width;
 scale2=PROCMETER_GRAPH_SCALE*bar->grid_num;

 length=value*scale1/scale2;

 send_command("widget_set %s_b value 1 %d %d",output->output->name,1+LCD_screen_height/2,length);

 if(LCD_screen_height>2)
   {
    length=bar->data_sum*scale1/(scale2*2*LCD_char_width);

    send_command("widget_set %s_b average %d 2 v",output->output->name,length+1);
   }
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

 if(write(server,buffer,len+1)!=len+1)
   {
    fprintf(stderr,"ProcMeter: Failed to write socket.\n");
    exit(1);
   }

 usleep(100);

 FD_ZERO(&readfd);

 FD_SET(server,&readfd);

 tv.tv_sec=tv.tv_usec=0;

 if(select(server+1,&readfd,NULL,NULL,&tv)==0)
    return;

 len=read(server,buffer,255);

 buffer[len]=0;

#if DEBUG
 printf("<- %s",buffer);
#endif

 return;
}
