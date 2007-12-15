/***************************************
  $Header: /home/amb/CVS/procmeter3/modules/acpi.c,v 1.15 2007-12-15 19:32:53 amb Exp $

  ProcMeter - A system monitoring program for Linux - Version 3.4g.

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
#include "apm.h"
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "acpi.h"

#define PROC_ACPI "/proc/acpi"
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

/* These are the strings used in the ACPI shipped with the 2.4 kernels */
char *acpi_labels_old[] = {
	"info",
	"status",
	"battery",
	"ac_adapter",
	"on-line",
	"Design Capacity:",
	"Present:",
	"Remaining Capacity:",
	"Present Rate:",
	"State:",
#if ACPI_THERMAL
	"thermal",
#endif
	"Status:",
	NULL
};

/* These are the strings used in ACPI in the 2.5 kernels, circa version
 * 20020214 */
char *acpi_labels_20020214[] = {
	"info",
	"state",
	"battery",
	"ac_adapter",
	"on-line",
	"design capacity:",
	"present:",
	"remaining capacity:",
	"present rate:",
	"charging state:",
#if ACPI_THERMAL
	"thermal_zone",
#endif
	"state:",
	"last full capacity:",
	NULL
};

char **acpi_labels = NULL;

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
	buf[end-1] = '\0';
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
		if (sscanf(ptr + strlen(key), "%255s", ret) == 1)
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

/* Returns the maximum capacity of a battery.
 *
 * Note that this returns the highest possible capacity for the battery,
 * even if it can no longer charge that fully. So normally it uses the
 * design capacity. While the last full capacity of the battery should
 * never exceed the design capacity, some silly hardware might report
 * that it does. So if the last full capacity is greater, it will be
 * returned.
 */
int get_acpi_batt_capacity(int battery) {
	int dcap, lcap;
	char *dcaps=get_acpi_value(acpi_batt_info[battery], acpi_labels[label_design_capacity]);
	char *lcaps=get_acpi_value(acpi_batt_info[battery], acpi_labels[label_last_full_capacity]);
	if (dcaps == NULL)
		dcap=0; /* battery not present */
	else
		dcap=atoi(dcaps);
	/* This is ACPI's broken way of saying that there is no battery. */
	if (dcap == 655350)
		return 0;
	if (lcaps != NULL) {
		lcap=atoi(lcaps);
		if (lcap > dcap)
			return lcap;
	}
	return dcap;
}

/* Comparison function for qsort. */
int _acpi_compare_strings (const void *a, const void *b) {
	const char **pa = (const char **)a;
	const char **pb = (const char **)b;
	return strcasecmp((const char *)*pa, (const char *)*pb);
}

/* Find something (batteries, ac adpaters, etc), and set up a string array
 * to hold the paths to info and status files of the things found.
 * Returns the number of items found. */
int find_items (char *itemname, char infoarray[ACPI_MAXITEM][128],
		                char statusarray[ACPI_MAXITEM][128]) {
	DIR *dir;
	struct dirent *ent;
	int num_devices=0;
	int i;
	char **devices = malloc(ACPI_MAXITEM * sizeof(char *));
	
	char pathname[128];

	sprintf(pathname,PROC_ACPI "/%s",itemname);

	dir = opendir(pathname);
	if (dir == NULL)
		return 0;
	while ((ent = readdir(dir))) {
		if (!strcmp(".", ent->d_name) || 
		    !strcmp("..", ent->d_name))
			continue;

		devices[num_devices]=strdup(ent->d_name);
		num_devices++;
		if (num_devices >= ACPI_MAXITEM)
			break;
	}
	closedir(dir);
	
	/* Sort, since readdir can return in any order. /proc/ does
	 * sometimes list BATT2 before BATT1. */
	qsort(devices, num_devices, sizeof(char *), _acpi_compare_strings);

	for (i = 0; i < num_devices; i++) {
		sprintf(infoarray[i], PROC_ACPI "/%s/%s/%s", itemname, devices[i],
			acpi_labels[label_info]);
		sprintf(statusarray[i], PROC_ACPI "/%s/%s/%s", itemname, devices[i],
			acpi_labels[label_status]);
		free(devices[i]);
	}

	return num_devices;
}

/* Find batteries, return the number, and set acpi_batt_count to it as well. */
int find_batteries(void) {
	int i;
	acpi_batt_count = find_items(acpi_labels[label_battery], acpi_batt_info, acpi_batt_status);
	for (i = 0; i < acpi_batt_count; i++)
		acpi_batt_capacity[i] = get_acpi_batt_capacity(i);
	return acpi_batt_count;
}

/* Find AC power adapters, return the number found, and set acpi_ac_count to it
 * as well. */
int find_ac_adapters(void) {
	acpi_ac_count = find_items(acpi_labels[label_ac_adapter], acpi_ac_adapter_info, acpi_ac_adapter_status);
	return acpi_ac_count;
}

#if ACPI_THERMAL
/* Find thermal information sources, return the number found, and set
 * thermal_count to it as well. */
int find_thermal(void) {
	acpi_thermal_count = find_items(acpi_labels[label_thermal], acpi_thermal_info, acpi_thermal_status);
	return acpi_thermal_count;
}
#endif

/* Returns true if the system is on ac power. Call find_ac_adapters first. */
int on_ac_power (void) {
	int i;
	for (i = 0; i < acpi_ac_count; i++) {
                char *online=get_acpi_value(acpi_ac_adapter_status[i], acpi_labels[label_ac_state]);
		if (online && strcmp(acpi_labels[label_online], online) == 0)
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
	DIR *dir;
	int num;

	if (!(dir = opendir(PROC_ACPI))) {
		return 0;
	}
	closedir(dir);
	
	/* If kernel is 2.6.21 or newer, version is in
	   /sys/module/acpi/parameters/acpica_version */
	
	version = get_acpi_file("/sys/module/acpi/parameters/acpica_version");
	if (version == NULL) {
		version = get_acpi_value(PROC_ACPI "/info", "ACPI-CA Version:");
		if (version == NULL) {
			/* 2.5 kernel acpi */
			version = get_acpi_value(PROC_ACPI "/info", "version:");
		}
	}
	if (version == NULL) {
		return 0;
	}
	num = atoi(version);
	if (num < ACPI_VERSION) {
		fprintf(stderr, "ACPI subsystem %s too is old, consider upgrading to %i.\n",
				version, ACPI_VERSION);
		return 0;
	}
	else if (num >= 20020214) {
		acpi_labels = acpi_labels_20020214;
	}
	else {
		acpi_labels = acpi_labels_old;
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
	
	if (acpi_batt_count == 0) {
		info->battery_percentage = 0;
		info->battery_time = 0;
		info->battery_status = BATTERY_STATUS_ABSENT;
		acpi_batt_capacity[battery] = 0;
		/* Where else would the power come from, eh? ;-) */
		info->ac_line_status = 1;
		return 0;
	}
	
	/* Internally it's zero indexed. */
	battery--;
	
	buf = get_acpi_file(acpi_batt_status[battery]);
	if (buf == NULL) {
		fprintf(stderr, "unable to read %s\n", acpi_batt_status[battery]);
		perror("read");
		exit(1);
	}

	info->ac_line_status = 0;
	info->battery_flags = 0;
	info->using_minutes = 1;
	
	/* Work out if the battery is present, and what percentage of full
	 * it is and how much time is left. */
	if (strcmp(scan_acpi_value(buf, acpi_labels[label_present]), "yes") == 0) {
		int pcap = scan_acpi_num(buf, acpi_labels[label_remaining_capacity]);
		int rate = scan_acpi_num(buf, acpi_labels[label_present_rate]);

		if (rate) {
			/* time remaining = (current_capacity / discharge rate) */
			info->battery_time = (float) pcap / (float) rate * 60;
		}
		else {
			char *rate_s = scan_acpi_value(buf, acpi_labels[label_present_rate]);
			if (! rate_s) {
				/* Time remaining unknown. */
				info->battery_time = 0;
			}
			else {
				/* a zero or unknown in the file; time 
				 * unknown so use a negative one to
				 * indicate this */
				info->battery_time = -1;
			}
		}

		state = scan_acpi_value(buf, acpi_labels[label_charging_state]);
		if (state) {
			if (state[0] == 'd') { /* discharging */
				info->battery_status = BATTERY_STATUS_CHARGING;
				/* Expensive ac power check used here
				 * because AC power might be on even if a
				 * battery is discharging in some cases. */
				info->ac_line_status = on_ac_power();
			}
			else if (state[0] == 'c' && state[1] == 'h') { /* charging */
				info->battery_status = BATTERY_STATUS_CHARGING;
				info->ac_line_status = 1;
				info->battery_flags = info->battery_flags | BATTERY_FLAGS_CHARGING;
				if (rate)
					info->battery_time = -1 * (float) (acpi_batt_capacity[battery] - pcap) / (float) rate * 60;
				else
					info->battery_time = 0;
				if (abs(info->battery_time) < 0.5)
					info->battery_time = 0;
			}
			else if (state[0] == 'o') { /* ok */
				/* charged, on ac power */
				info->battery_status = BATTERY_STATUS_HIGH;
				info->ac_line_status = 1;
			}
			else if (state[0] == 'c') { /* not charging, so must be critical */
				info->battery_status = BATTERY_STATUS_CRITICAL;
				/* Expensive ac power check used here
				 * because AC power might be on even if a
				 * battery is critical in some cases. */
				info->ac_line_status = on_ac_power();
			}
			else if (rate == 0) {
				/* if rate is null, battery charged, on
				 * ac power */
				info->battery_status = BATTERY_STATUS_HIGH;
				info->ac_line_status = 1;
			}
			else {
				fprintf(stderr, "unknown battery state: %s\n", state);
			}
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
		else if (pcap > acpi_batt_capacity[battery]) {
			/* Battery is somehow charged to greater than max
			 * capacity. Rescan for a new max capacity. */
			find_batteries();
		}
		
		if (pcap && acpi_batt_capacity[battery]) {
			/* percentage = (current_capacity / max capacity) * 100 */
			info->battery_percentage = (float) pcap / (float) acpi_batt_capacity[battery] * 100;
		}
		else {
			info->battery_percentage = -1;
		}

	}
	else {
		info->battery_percentage = 0;
		info->battery_time = 0;
		info->battery_status = BATTERY_STATUS_ABSENT;
		acpi_batt_capacity[battery] = 0;
		if (acpi_batt_count == 0) {
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
  /* char  name[];          */ "Thermal%i",
  /* char *description;     */ "Temperature output %i.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 10,
  /* char  graph_units[];   */ "unknown", /* filled out later */
 },
 /*+ Status +*/
 {
  /* char  name[];          */ "Thermal%i_State",
  /* char *description;     */ "State of thermal outputs %i.",
  /* char  type;            */ PROCMETER_TEXT,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ -1,
  /* short graph_scale;     */ 0,
  /* char  graph_units[];   */ "n/a",
 }
};

/*+ The template for ACPI battery info. +*/
#define N_BATT_OUTPUTS 5
ProcMeterOutput _batt_outputs[N_BATT_OUTPUTS]=
{
 /*+ Battery charge percent. +*/
 {
  /* char  name[];          */ "Batt%i_Charge",
  /* char *description;     */ "The percent that battery %i is charged.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0%",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 10,
  /* char  graph_Units[];   */ "(%d%%)"
 },
 /*+ Rate of charge/discharge. +*/
 {
  /* char  name[];          */ "Batt%i_Rate",
  /* char *description;     */ "The present rate of battery %i discharge or charge, in mA.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "0 mA",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 1000,
  /* char  graph_Units[];   */ "(%d mA)"
 },
 /*+ The battery status output +*/
 {
  /* char  name[];          */ "Batt%i_Status",
  /* char *description;     */ "The estimated status of the battery, one of the states unknown, critical, low or high",
  /* char  type;            */ PROCMETER_TEXT,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ -1,
  /* short graph_scale;     */ 0,
  /* char  graph_units[];   */ "n/a",
 },
 /*+ The battery remaining time output. +*/
 {
  /* char  name[];          */ "Batt%i_Time",
  /* char *description;     */ "The current estimated lifetime remaining for battery %i.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 30,
  /* char  graph_units[];   */ "(%d min)"
 },
 /*+ The time till charge. +*/
 {
  /* char  name[];          */ "Batt%i_ChargeTime",
  /* char *description;     */ "The current estimated time until battery %i is fully charged.",
  /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
  /* short interval;        */ 1,
  /* char  text_value[];    */ "unknown",
  /* long  graph_value;     */ 0,
  /* short graph_scale;     */ 30,
  /* char  graph_units[];   */ "(%d min)"
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
 /* char name[];            */ "ACPI",
 /* char *description;      */ "ACPI information [From /proc/acpi]. These outputs are only available if you have configured the kernel "
                               "with ACPI support. "
                               "(Use 'options=C' or 'options=F' in the configuration file to specify preferred units of temperature.)"
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
		
		if (strcmp(scan_acpi_value(buf, acpi_labels[label_present]), "yes") == 0) {
			pcap = scan_acpi_num(buf, acpi_labels[label_remaining_capacity]);
			rate = scan_acpi_num(buf, acpi_labels[label_present_rate]);
			
			if (rate) {
				/* time remaining till empty = current_capacity / discharge rate) */
				timeleft = (float) pcap / (float) rate * 60;
			}
			else {
				char *rate_s = scan_acpi_value(buf, acpi_labels[label_present_rate]);
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
			status = scan_acpi_value(buf, acpi_labels[label_charging_state]);
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
		sprintf(batt_outputs[index + 1].text_value,"%i mA", rate);
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
