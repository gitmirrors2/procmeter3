/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/acpi.c,v 1.3 2002-06-02 09:54:57 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.3b.

  ACPI source file.
  ******************/ /******************
  Written by Joey Hess.

  This file Copyright 2001 Joey Hess
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/

#define ACPI_THERMAL 1

/****************** Begin acpi.c **********************************/

/* 
 * A not-yet-general-purpose ACPI library, by Joey Hess <joey@kitenet.net>
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#ifdef ACPI_APM
#include <apm.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "acpi.h"

#define ACPI_MAXITEM 8

int acpi_batt_count = 0;
/* Filenames of the battery info files for each system battery. */
char acpi_batt_info[ACPI_MAXITEM][128];
/* Filenames of the battery status files for each system battery. */
char acpi_batt_status[ACPI_MAXITEM][128];
/* Stores battery capacity, or 0 if the battery is absent. */
int acpi_batt_capacity[ACPI_MAXITEM];

int acpi_ac_count = 0;
char acpi_ac_adapter_info[ACPI_MAXITEM][128];
char acpi_ac_adapter_status[ACPI_MAXITEM][128];

#if ACPI_THERMAL
int acpi_thermal_count = 0;
char acpi_thermal_info[ACPI_MAXITEM][128];
char acpi_thermal_status[ACPI_MAXITEM][128];
#endif

/* Read in an entire ACPI proc file (well, the first 1024 bytes anyway), and
 * return a statically allocated array containing it. */
inline char *get_acpi_file (const char *file) {
	int fd;
	int end;
	static char buf[1024];
	fd = open(file, O_RDONLY);
	if (fd == -1) return NULL;
	end = read(fd, buf, sizeof(buf));
	buf[end] = '\0';
	close(fd);
	return buf;
}

/* Given a buffer holding an acpi file, searches for the given key in it,
 * and returns the numeric value. 0 is returned on failure. */
inline int scan_acpi_num (const char *buf, const char *key) {
	char *ptr;
	int ret;
	if ((ptr = strstr(buf, key))) {
		if (sscanf(ptr + strlen(key), "%d", &ret) == 1)
			return ret;
	}
	return 0;
}

/* Given a buffer holding an acpi file, searches for the given key in it,
 * and returns its value in a statically allocated string. */
inline char *scan_acpi_value (const char *buf, const char *key) {
	char *ptr;
	static char ret[256];
	if ((ptr = strstr(buf, key))) {
		if (sscanf(ptr + strlen(key), "%s", ret) == 1)
			return ret;
	}
	return NULL;
}

/* Read an ACPI proc file, pull out the requested piece of information, and
 * return it (statically allocated string). Returns NULL on error, This is 
 * the slow, dumb way, fine for initialization or if only one value is needed
 * from a file, slow if called many times. */
char *get_acpi_value (const char *file, const char *key) {
	char *buf = get_acpi_file(file);
	if (! buf) return NULL;
	return scan_acpi_value(buf, key);
}

/* Returns the last full capacity of a battery. */
int get_acpi_batt_capacity(int battery) {
	int cap;
	cap = atoi(get_acpi_value(acpi_batt_info[battery], "Last Full Capacity:"));
	/* This is ACPI's broken way of saying that there is no battery. */
	if (cap == 655350)
		return 0;
	return cap;
}

/* Find something (batteries, ac adpaters, etc), and set up a string array
 * to hold the paths to info and status files of the things found. Must be 
 * in /proc/acpi to call this. Returns the number of items found. */
int find_items (char *itemname, char infoarray[ACPI_MAXITEM][128],
		                char statusarray[ACPI_MAXITEM][128]) {
	DIR *dir;
	struct dirent *ent;
	int count = 0;
	
	dir = opendir(itemname);
	if (dir == NULL)
		return 0;

	while ((ent = readdir(dir))) {
		if (!strncmp(".", ent->d_name, 1) || 
		    !strncmp("..", ent->d_name, 2))
			continue;

		sprintf(infoarray[count], "%s/%s/%s", itemname, ent->d_name, "info");
		sprintf(statusarray[count], "%s/%s/%s", itemname, ent->d_name, "status");
		count++;
		if (count > ACPI_MAXITEM)
			break;
	}

	closedir(dir);
	return count;
}

/* Find batteries, return the number, and set acpi_batt_count to it as well. */
int find_batteries(void) {
	int i;
	acpi_batt_count = find_items("battery", acpi_batt_info, acpi_batt_status);
	/* Read in the last charged capacity of the batteries. */
	for (i = 0; i < acpi_batt_count; i++)
		acpi_batt_capacity[i] = get_acpi_batt_capacity(i);
	return acpi_batt_count;
}

/* Find AC power adapters, return the number found, and set acpi_ac_count to it
 * as well. */
int find_ac_adapters(void) {
	acpi_ac_count = find_items("ac_adapter", acpi_ac_adapter_info, acpi_ac_adapter_status);
	return acpi_ac_count;
}

#if ACPI_THERMAL
/* Find thermal information sources, return the number found, and set
 * thermal_count to it as well. */
int find_thermal(void) {
	acpi_thermal_count = find_items("thermal", acpi_thermal_info, acpi_thermal_status);
	return acpi_thermal_count;
}
#endif

/* Returns true if the system is on ac power. Call find_ac_adapters first. */
int on_ac_power (void) {
	int i;
	for (i = 0; i < acpi_ac_count; i++) {
		if (strcmp("on-line", get_acpi_value(acpi_ac_adapter_status[i], "Status:")))
			return 1;
		else
			return 0;
	}
	return 0;
}

/* See if we have ACPI support and check version. Also find batteries and
 * ac power adapters. */
int acpi_supported (void) {
	char *version;

	if (chdir("/proc/acpi") == -1) {
		return 0;
	}
	
	version = get_acpi_value("info", "ACPI-CA Version:");
	if (version == NULL) {
		return 0;
	}
	if (atoi(version) < ACPI_VERSION) {
		fprintf(stderr, "ACPI subsystem %s too is old, consider upgrading to %i.\n",
				version, ACPI_VERSION);
		return 0;
	}
	
	find_batteries();
	find_ac_adapters();
#if ACPI_THERMAL
	find_thermal();
#endif
	
	return 1;
}

#ifdef ACPI_APM
/* Read ACPI info on a given power adapter and battery, and fill the passed
 * apm_info struct. */
int acpi_read (int battery, apm_info *info) {
	char *buf, *state;
	
	/* Internally it's zero indexed. */
	battery--;
	
	buf = get_acpi_file(acpi_batt_status[battery]);

	info->ac_line_status = 0;
	info->battery_flags = 0;
	info->using_minutes = 1;
	
	/* Work out if the battery is present, and what pqercentage of full
	 * it is and how much time is left. */
	if (strcmp(scan_acpi_value(buf, "Present:"), "yes") == 0) {
		int pcap = scan_acpi_num(buf, "Remaining Capacity:");
		int rate = scan_acpi_num(buf, "Present Rate:");

		if (rate) {
			/* time remaining = (current_capacity / discharge rate) */
			info->battery_time = (float) pcap / (float) rate * 60;
		}
		else {
			char *rate_s = scan_acpi_value(buf, "Present Rate:");
			if (! rate_s) {
				/* Time remaining unknown. */
				info->battery_time = 0;
			}
			/* This is a hack for my picturebook. If
			 * the battery is not present, ACPI still
			 * says it is Present, but sets this to
			 * unknown. I don't know if this is the
			 * correct way to do it. */
			else if (strcmp(rate_s, "unknown") == 0) {
				goto NOBATT;
			}
		}

		state = scan_acpi_value(buf, "State:");
		if (state) {
			if (state[0] == 'd') { /* discharging */
				info->battery_status = BATTERY_STATUS_CHARGING;
			}
			else if (state[0] == 'c' && state[1] == 'h') { /* charging */
				info->battery_status = BATTERY_STATUS_CHARGING;
				info->ac_line_status = 1;
				info->battery_flags = info->battery_flags | BATTERY_FLAGS_CHARGING;
				info->battery_time = -1 * (float) (acpi_batt_capacity[battery] - pcap) / (float) rate * 60;
			}
			else if (state[0] == 'o') { /* ok */
				/* charged, on ac power */
				info->battery_status = BATTERY_STATUS_HIGH;
				info->ac_line_status = 1;
			}
			else if (state[0] == 'c') { /* not charging, so must be critical */
				info->battery_status = BATTERY_STATUS_CRITICAL;
			}
			else
				fprintf(stderr, "unknown battery state: %s\n", state);
		}
		else {
			/* Battery state unknown. */
			info->battery_status = BATTERY_STATUS_ABSENT;
		}
		
		if (acpi_batt_capacity[battery] == 0) {
			/* The battery was absent, and now is present.
			 * Well, it might be a different battery. So
			 * re-probe the battery. */
			/* NOTE that this invalidates buf. No accesses of
			 * buf below this point! */
			acpi_batt_capacity[battery] = get_acpi_batt_capacity(battery);
		}
		
		if (pcap) {
			/* percentage = (current_capacity / last_full_capacity) * 100 */
			info->battery_percentage = (float) pcap / (float) acpi_batt_capacity[battery] * 100;
		}
		else {
			info->battery_percentage = -1;
		}

	}
	else {
NOBATT:
		info->battery_percentage = 0;
		info->battery_time = 0;
		info->battery_status = BATTERY_STATUS_ABSENT;
		acpi_batt_capacity[battery] = 0;
		if (acpi_batt_count == 1) {
			/* Where else would the power come from, eh? ;-) */
			info->ac_line_status = 1;
		}
		else {
			/* Expensive ac power check. */
			info->ac_line_status = on_ac_power();
		}
	}
	
	return 0;
}
#endif

/****************** End acpi.c ************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procmeter.h"

/* The interface information.  */

/*+ The template for ACPI thermal info. +*/
#define N_THERMAL_OUTPUTS 2
ProcMeterOutput _thermal_outputs[N_THERMAL_OUTPUTS]=
{
 /*+ Temp. +*/
 {
  /* char  name[16];         */ "Thermal%i",
  /* char *description;      */ "Temperature output %i.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "unknown",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 10,
  /* char  graph_units[8];   */ "unknown", /* filled out later */
 },
 /*+ Status +*/
 {
  /* char  name[16];         */ "Thermal%i_Stat",
  /* char *description;      */ "State of thermal outputs %i.",
  /* char  type;             */ PROCMETER_TEXT,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "unknown",
  /* long  graph_value;      */ -1,
  /* short graph_scale;      */ 0,
  /* char  graph_units[8];   */ "n/a",
 }
};

/*+ The template for ACPI battery info. +*/
#define N_BATT_OUTPUTS 5
ProcMeterOutput _batt_outputs[N_BATT_OUTPUTS]=
{
 /*+ Battery charge percent. +*/
 {
  /* char  name[16];         */ "Batt%i_Charge",
  /* char *description;      */ "The percent that battery %i is charged.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0%",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 10,
  /* char  graph_Units[8];   */ "(%d%%)"
 },
 /*+ Rate of charge/discharge. +*/
 {
  /* char  name[16];         */ "Batt%i_Rate",
  /* char *description;      */ "The present rate of battery %i discharge or charge, in mW.",
  /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;         */ 1,
  /* char  text_value[16];   */ "0 mW",
  /* long  graph_value;      */ 0,
  /* short graph_scale;      */ 1000,
  /* char  graph_Units[8];   */ "(%d mW)"
 },
 /*+ The battery status output +*/
 {
   /* char  name[16];         */ "Batt%i_Status",
   /* char *description;      */ "The estimated status of the battery, one of the states unknown, critical, low or high",
   /* char  type;             */ PROCMETER_TEXT,
   /* short interval;         */ 1,
   /* char  text_value[16];   */ "unknown",
   /* long  graph_value;      */ -1,
   /* short graph_scale;      */ 0,
   /* char  graph_units[8];   */ "n/a",
 },
 /*+ The battery remaining time output. +*/
 {
 /* char  name[16];         */ "Batt%i_Time",
 /* char *description;      */ "The current estimated lifetime remaining for battery %i.",
 /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
 /* short interval;         */ 1,
 /* char  text_value[16];   */ "unknown",
 /* long  graph_value;      */ 0,
 /* short graph_scale;      */ 30,
 /* char  graph_units[8];   */ "(%d min)"
 },
 /*+ The time till charge. +*/
 {
 /* char  name[16];         */ "Batt%i_ChrgTm",
 /* char *description;      */ "The current estimated time until battery %i is fully charged.",
 /* char  type;             */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
 /* short interval;         */ 1,
 /* char  text_value[16];   */ "unknown",
 /* long  graph_value;      */ 0,
 /* short graph_scale;      */ 30,
 /* char  graph_units[8];   */ "(%d min)"
 }
};

/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The battery outputs +*/
ProcMeterOutput *batt_outputs=NULL;
/*+ The thermal outputs +*/
ProcMeterOutput *thermal_outputs=NULL;

/* Use celcius or farenheight? */
int use_celcius = 1;

/*+ The module. +*/
ProcMeterModule module=
{
 /* char name[16];           */ "ACPI",
 /* char *description;       */ "ACPI information [From /proc/acpi]. These outputs are only available if you have configured the kernel "
                                "with ACPI support. "
                                "(Use 'options=C' or 'options=F' in the configuration file to specify preferred units of temperature."
};

static int last_batt_update[ACPI_MAXITEM];
static int last_thermal_update[ACPI_MAXITEM];

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
 int i, j;
 int n=0;
 
 /* Parse options. */
 if(options) {
	 char *l=options;

	 while(*l && *l==' ')
		 l++;

	 if(*l) {
		 char *r=l,pr;

		 while(*r && *r!=' ')
			 r++;

		 pr=*r;
		 *r=0;

		 if (strcmp(l, "C") == 0) {
			 use_celcius=1;
		 }
		 else if (strcmp(l, "F") == 0) {
			 use_celcius=0;
		 }
		 else {
			 fprintf(stderr,"ProcMeter(%s): unknown options \"%s\"\n",__FILE__, l);
		 }
	}
 }
 
 outputs=(ProcMeterOutput**)malloc(sizeof(ProcMeterOutput*));
 outputs[0]=NULL;

 if (! acpi_supported())
	 return outputs;

 /* Iterate over the batteries and thermal information sources and set up
  * outputs for both. */
 outputs=(ProcMeterOutput**)realloc((void*)outputs,(N_BATT_OUTPUTS * acpi_batt_count + N_THERMAL_OUTPUTS * acpi_thermal_count + 1)*sizeof(ProcMeterOutput*));
 batt_outputs=(ProcMeterOutput*)realloc((void*)batt_outputs, acpi_batt_count * N_BATT_OUTPUTS * sizeof(ProcMeterOutput));
 for (i = 0; i < acpi_batt_count; i++) {
	 last_batt_update[i] = 0;
	 for (j = 0; j < N_BATT_OUTPUTS; j++) {
		 int index = i * N_BATT_OUTPUTS + j;
		 batt_outputs[index] = _batt_outputs[j];
		 sprintf(batt_outputs[index].name, _batt_outputs[j].name, i + 1);
		 batt_outputs[index].description=(char*)malloc(strlen(_batt_outputs[j].description)+8);
		 sprintf(batt_outputs[index].description,_batt_outputs[j].description,i + 1);
		 outputs[n++]=&batt_outputs[index];
	 }
 }
 thermal_outputs=(ProcMeterOutput*)realloc((void*)thermal_outputs, acpi_thermal_count * N_THERMAL_OUTPUTS * sizeof(ProcMeterOutput));
 for (i = 0; i < acpi_thermal_count; i++) {
	 last_thermal_update[i] = 0;
	 for (j = 0; j < N_THERMAL_OUTPUTS; j++) {
		 int index = i * N_THERMAL_OUTPUTS + j;
		 thermal_outputs[index] = _thermal_outputs[j];
		 sprintf(thermal_outputs[index].name, _thermal_outputs[j].name, i + 1);
		 thermal_outputs[index].description=(char*)malloc(strlen(_thermal_outputs[j].description)+8);
		 sprintf(thermal_outputs[index].description,_thermal_outputs[j].description,i + 1);
		 if (j == 0) {
			 /* Fill in units. */
			 sprintf(thermal_outputs[index].graph_units,"%%d%s", use_celcius ? "C" : "F");
			 /* The scale needs to be larger, for farenheight. */
			 if (! use_celcius)
				 thermal_outputs[index].graph_scale = 20;
		 }
		 outputs[n++]=&thermal_outputs[index];
	 }
 }
 outputs[n] = NULL;
 
 return(outputs);
}

/*++++++++++++++++++++++++++++++++++++++
  Perform an update on one of the statistics.

  int Update Returns 0 if OK, else -1.

  time_t now The current time.

  ProcMeterOutput *output The output that the value is wanted for.
  ++++++++++++++++++++++++++++++++++++++*/

int Update(time_t now, ProcMeterOutput *output)
{
 char *buf;
 int index = output - batt_outputs;
 
 /* Is this a battery or a thermal output? */
 if (index >= 0 && index < acpi_batt_count * N_BATT_OUTPUTS) {
	/* Battery. */
	index = (output - batt_outputs) / N_BATT_OUTPUTS;

	/* Only update every ten seconds, because acpi has a lot of
	 * overhead, both here and in the kernel to generate those pretty
	 * proc files. It also doesn't change very fast.. */
	if (now - last_batt_update[index] >= 10) {
		float percent = 0, timeleft = 0, timefull = 0;
		int rate = 0, pcap = 0;
		char *status;
		
		last_batt_update[index] = now;
		
		buf = get_acpi_file(acpi_batt_status[index]);
		if (! buf) return(-1);
		
		if (strcmp(scan_acpi_value(buf, "Present:"), "yes") == 0) {
			pcap = scan_acpi_num(buf, "Remaining Capacity:");
			rate = scan_acpi_num(buf, "Present Rate:");
			
			if (rate) {
				/* time remaining till empty = current_capacity / discharge rate) */
				timeleft = (float) pcap / (float) rate * 60;
			}
			else {
				char *rate_s = scan_acpi_value(buf, "Present Rate:");
				/* If the battery is not present, ACPI may
				 * still say it is but sets rate to unknown
				 * (on my picturebook, anyway). I don't
				 * know if this is the correct way to do
				 * it. */
				if (rate_s && strcmp(rate_s, "unknown") == 0) {
					goto NOBATT;
				}
			}
			
			/* time remaining till full */
			timefull = (float)(acpi_batt_capacity[index] - pcap) / (float) rate * 60;

			/* Status. */
			status = scan_acpi_value(buf, "State:");
			sprintf(batt_outputs[index + 2].text_value, "%s", status);
			
			if (strcmp(status, "charging") == 0) {
				/* Kill time left until empty. */
				batt_outputs[index + 3].graph_value = PROCMETER_GRAPH_FLOATING(0);
				sprintf(batt_outputs[index + 3].text_value,"n/a");
				
				/* Time left till full. */
				batt_outputs[index + 4].graph_value = PROCMETER_GRAPH_FLOATING(timefull/batt_outputs[index + 4].graph_scale);
				sprintf(batt_outputs[index + 4].text_value,"%i:%02i", (int) timefull / 60, (int) timefull % 60);
			}
			else {
				/* Time left till empty. */
				batt_outputs[index + 3].graph_value = PROCMETER_GRAPH_FLOATING(timeleft/batt_outputs[index + 3].graph_scale);
				sprintf(batt_outputs[index + 3].text_value,"%i:%02i", (int) timeleft / 60, (int) timeleft % 60);
				
				/* Kill time left until full. */
				batt_outputs[index + 4].graph_value = PROCMETER_GRAPH_FLOATING(0);
				sprintf(batt_outputs[index + 4].text_value,"n/a");
			}
			
			if (acpi_batt_capacity[index] == 0) {
				/* The battery was absent, and now is
				 * present. Well, it might be a different
				 * battery. So re-probe the battery. */
				/* NOTE that this invalidates buf. */
				acpi_batt_capacity[index] = get_acpi_batt_capacity(index);
			}

			if (pcap) {
				/* percentage = (current_capacity / last_full_capacity) * 100 */
				percent = (float) pcap / (float) acpi_batt_capacity[index] * 100;
			}
			else {
				percent = 0;
			}
		}
		else {
NOBATT:		
			acpi_batt_capacity[index] = 0; 
			
			/* Kill time left until empty. */
			batt_outputs[index + 3].graph_value = PROCMETER_GRAPH_FLOATING(0);
			sprintf(batt_outputs[index + 3].text_value,"n/a");
				
			/* Kill time left till full. */
			batt_outputs[index + 4].graph_value = PROCMETER_GRAPH_FLOATING(0);
			sprintf(batt_outputs[index + 4].text_value,"n/a");
		}
		
		/* Percent charged. */
		batt_outputs[index + 0].graph_value = PROCMETER_GRAPH_FLOATING(percent/batt_outputs[index + 0].graph_scale);
		sprintf(batt_outputs[index + 0].text_value,"%.0f%%", percent);
	
		/* Charge/discharge rate. */
		batt_outputs[index + 1].graph_value = PROCMETER_GRAPH_FLOATING((float)rate/batt_outputs[index + 1].graph_scale);
		sprintf(batt_outputs[index + 1].text_value,"%i mW", rate);
	}

	return(0);
 }
 else {
	/* Thermal */
	index = (output - thermal_outputs) / N_THERMAL_OUTPUTS;

	if (now - last_thermal_update[index] >= 10) {
		float temp;
		char *state;

		last_thermal_update[index] = now;
		
		buf = get_acpi_file(acpi_thermal_status[index]);
		if (! buf) return(-1);

		/* Acpi reports in dK. */
		temp = (float) scan_acpi_num(buf, "temperature:") / 100;
		if (! use_celcius)
			temp = temp * 1.8 + 32;
		thermal_outputs[index + 0].graph_value=PROCMETER_GRAPH_FLOATING(temp/thermal_outputs[index + 0].graph_scale);
		sprintf(thermal_outputs[index + 0].text_value," %.2f %s", temp, use_celcius ? "C" : "F");
		 
		state = scan_acpi_value(buf, "state:");
		if (! state)
			sprintf(thermal_outputs[index + 1].text_value, "unknown");
		else
			sprintf(thermal_outputs[index + 1].text_value, "%s", state);
	}
	
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
 
 if(batt_outputs)
   {
    for(i=0; i < acpi_batt_count * N_BATT_OUTPUTS; i++)
       free(batt_outputs[i].description);
    free(batt_outputs);
   }
 if(thermal_outputs)
   {
    for(i=0; i < acpi_thermal_count * N_THERMAL_OUTPUTS; i++)
       free(thermal_outputs[i].description);
    free(thermal_outputs);
   }
  if(outputs)
    free(outputs);
}