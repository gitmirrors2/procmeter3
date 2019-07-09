/***************************************
  ProcMeter - A system monitoring program for Linux - Version 3.6a.

  lm_sensors module source file.
  ******************/ /******************
  Written by Mike T. Liang

  Original file Copyright 2009, 2010 Mike T. Liang
  Parts of this file Copyright 2019 Andrew M. Bishop
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/

#ifndef INCLUDED
#define FANSPEED
const char *FILE__=__FILE__;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sensors/sensors.h>
#include <sensors/error.h>

#include "procmeter.h"

#if !defined(SENSORS_API_VERSION) || ( (SENSORS_API_VERSION & 0xf00) != 0x400 ) && ( (SENSORS_API_VERSION & 0xf00) != 0x500 )
#error "This module requires libsensors version 4 or 5"
#else

/* The interface information.  */

/*+ The template output +*/
ProcMeterOutput template_output=
{
 /* char  name[];          */ "unknown",
#if defined(FANSPEED)
 /* char *description;     */ "Fan speed. [From %s %s]",
#elif defined(TEMPERATURE)
 /* char *description;     */ "Temperature. [From %s %s]",
#elif defined(VOLTAGE)
 /* char *description;     */ "Voltage level. [From %s %s]",
#else
#error Undefined module.
#endif
 /* char  type;            */ PROCMETER_GRAPH|PROCMETER_TEXT|PROCMETER_BAR,
 /* short interval;        */ 1,
 /* char  text_value[];    */ "unknown",
 /* long  graph_value;     */ 0,
#if defined(FANSPEED)
 /* short graph_scale;     */ 1000,
 /* char  graph_units[];   */ "(%d RPM)"
#elif defined(TEMPERATURE)
 /* short graph_scale;     */ 10,
 /* char  graph_units[];   */ "(%d C)"
#elif defined(VOLTAGE)
 /* short graph_scale;     */ 1,
 /* char  graph_units[];   */ "(%d V)"
#else
#error Undefined module.
#endif
};

/*+ The outputs. +*/
ProcMeterOutput **outputs=NULL;

/*+ The module. +*/
ProcMeterModule module=
{
#if defined(FANSPEED)
 /* char name[];           */ "FanSpeed",
#elif defined(TEMPERATURE)
 /* char name[];           */ "Temperature",
#elif defined(VOLTAGE)
 /* char name[];           */ "Voltage",
#else
#error Undefined module.
#endif
 /* char *description;     */ "Hardware sensor information from libsensors.",
};

typedef struct {
    char *description;
    const sensors_chip_name *chip;
    char *label;
    int number;
} Sensor;

int count;
Sensor *sensorv;
ProcMeterOutput *outputv;

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
    int error;
    int i=0;

    /* Uninitialized libsensors will report no detected chips. */
    if (!sensors_get_detected_chips(NULL, &i)) {
	FILE *input=NULL;
	if (options) {
	    input=fopen(options, "r");
	    if (!input) {
		fprintf(stderr,"ProcMeter(%s): %s: %s\n", FILE__, options, strerror(errno));
	    }
	}
	error=sensors_init(input);
	if (input) fclose(input);
	if (error) {
	    fprintf(stderr,"ProcMeter(%s): %s\n", FILE__, sensors_strerror(error));
	    sensors_cleanup();
	}
    }

    const sensors_chip_name *chip;
    for (count=0, i=0; (chip=sensors_get_detected_chips(NULL, &i));) {

	char name[1024];
	error=sensors_snprintf_chip_name(name, sizeof(name), chip);
	if (error<0) {
	    fprintf(stderr,"ProcMeter(%s): %s\n", FILE__, sensors_strerror(error));
	    strcpy(name, "unknown");
	}

	const sensors_feature *feature;
	int f=0;
	while ((feature=sensors_get_features(chip, &f))) {
	    const sensors_subfeature *subfeature;

#if defined(FANSPEED)
	    if (feature->type!=SENSORS_FEATURE_FAN) continue;
	    subfeature=sensors_get_subfeature(chip, feature, SENSORS_SUBFEATURE_FAN_INPUT);

#elif defined(TEMPERATURE)
	    if (feature->type!=SENSORS_FEATURE_TEMP) continue;
	    subfeature=sensors_get_subfeature(chip, feature, SENSORS_SUBFEATURE_TEMP_INPUT);

#elif defined(VOLTAGE)
	    if (feature->type==SENSORS_FEATURE_IN) {
		subfeature=sensors_get_subfeature(chip, feature, SENSORS_SUBFEATURE_IN_INPUT);
	    } else if (feature->type==SENSORS_FEATURE_VID) {
		subfeature=sensors_get_subfeature(chip, feature, SENSORS_SUBFEATURE_VID);
	    } else {
		continue;
	    }

#else
#error Undefined module.
#endif

	    if (!subfeature) continue;

	    char *label=sensors_get_label(chip, feature);
	    char *description=malloc(strlen(template_output.description)+strlen(name)+strlen(label));
	    sprintf(description, template_output.description, name, label);

	    char *s;
	    while ((s=strchr(label, ' '))) *s='_';

	    if (!count) sensorv=malloc(sizeof(*sensorv));
	    else sensorv=realloc(sensorv, (count+1)*sizeof(*sensorv));

	    sensorv[count].description=description;
	    sensorv[count].chip=chip;
	    sensorv[count].label=label;
	    sensorv[count].number=subfeature->number;
	    ++count;
	}
    }

    if (count) outputv=malloc(count*sizeof(*outputv));
    outputs=malloc((count+1)*sizeof(*outputs));
    outputs[count]=0;

    for (i=0; i<count; ++i) {
	outputv[i]=template_output;
	snprintf(outputv[i].name, sizeof(outputv[i].name), "%s", sensorv[i].label);
	outputv[i].description=sensorv[i].description;
	outputs[i]=&outputv[i];
    }

    return outputs;
}


/*++++++++++++++++++++++++++++++++++++++
  Perform an update on one of the statistics.

  int Update Returns 0 if OK, else -1.

  time_t now The current time.

  ProcMeterOutput *output The output that the value is wanted for.
  ++++++++++++++++++++++++++++++++++++++*/

int Update(time_t now,ProcMeterOutput *output)
{
    int i=output-outputv;
    if ((i<0)||(i>=count)) return -1;

    double value;
    int error=sensors_get_value(sensorv[i].chip, sensorv[i].number, &value);
    if (error<0) {
	fprintf(stderr,"ProcMeter(%s): %s\n", FILE__, sensors_strerror(error));
	return -1;
    }

#if defined(FANSPEED)
    sprintf(output->text_value, "%.0lf RPM", value);
#elif defined(TEMPERATURE)
    sprintf(output->text_value, "%.1lf C", value);
#elif defined(VOLTAGE)
    sprintf(output->text_value, "%.3lf V", value);
#else
#error Undefined module.
#endif
    output->graph_value=PROCMETER_GRAPH_FLOATING(value/output->graph_scale);

    return 0;
}


/*++++++++++++++++++++++++++++++++++++++
  Tidy up and prepare to have the module unloaded.
  ++++++++++++++++++++++++++++++++++++++*/

void Unload(void)
{
    free(outputs);

    int i;
    for (i=0; i<count; ++i) {
	free(sensorv[i].description);
	free(sensorv[i].label);
    }

    if (count) {
	free(outputv);
	free(sensorv);
    }

    /* Multiple modules use libsensors, so don't call sensors_cleanup(). */
}

#endif
