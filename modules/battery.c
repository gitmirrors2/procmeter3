/***************************************
  ProcMeter - A system monitoring program for Linux

  battery values from /sys/ (for example new-style ACPI)

  This file Copyright 2011 Bernhard R. Link
  It may be distributed under the GNU Public License, version 2, or
  any higher version.  See section COPYING of the GNU Public license
  for conditions under which this file may be redistributed.
  ***************************************/

#define _GNU_SOURCE 1
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#include "procmeter.h"

static bool debug_battery = false;

static ProcMeterModule module = {
 /* char name[];       */ "Battery",
 /* char *description; */ "battery information from /sys/class/power_supply/",
};

ProcMeterModule *Load(void) {
	return(&module);
}

#ifndef __USE_ATFILE
#warning sysbattery module not available, as most likely not supported (__USE_ATFILE not defined)

void Unload(void) {
}

ProcMeterOutput **Initialise(char *options) {
	static ProcMeterOutput *empty_list[] = { NULL };

	return empty_list;
}

int Update(time_t now,ProcMeterOutput *output) {
	return -1;
}
#else

const static struct battery_field {
	const char *name;
	enum batteryfieldtype {
		bft_yesno, bft_string, bft_count,
		bft_muA, bft_muAh, bft_v }
			fieldtype;
	bool variable, required;
	const char *description, *name_template;
} fields[] = {
#define FIELD_OFS_STATUS 0
	{ "status", bft_string, true, true,
		"status (Charging/Discharging)", "%s state"},
#define FIELD_OFS_CURRENT 1
	{ "current_now", bft_muA, true, true,
		"current charging/discharging rate", "%s rate"},
#define FIELD_OFS_CHARGE 2
	{ "charge_now", bft_muAh, true, true,
		"current charge of the battery", "%s charge"},
#define FIELD_OFS_FULL 3
	{ "charge_full", bft_muAh, false, true,
		"last full charge of the battery", "%s last full"},
	{ "charge_full_design", bft_muAh, false, false,
		"designed full charge of the battery", "%s design full"},
//	{ "cycle_count", bft_count, false, false,
//		"content of the 'cycle_count' file", "%s cycle count"},
	{ "manufacturer", bft_string, false, false,
		"manufacturer", "%s manufacturer"},
	{ "model_name", bft_string, false, false,
		"model name", "%s model name"},
	{ "serial_number", bft_string, false, false,
		"serial number", "%s serial number"},
	{ "technology", bft_string, false, false,
		"technology", "%s technology"},
	{ "voltage_min_design", bft_v, false, false,
		"minimum design voltage", "%s min voltage"},
	{ "voltage_now", bft_v, true, false,
		"current voltage", "%s voltage"},
	{ NULL, 0}
};

#define BAT_OUTPUT_COUNT 15

static struct battery {
	struct battery *next;
	char *name;
	char *directory;
	int fd;
	int output_count;
	enum battery_state {
		/* not present (or can no longer be read) */
		battery_unavailable,
		/* present, but unread since available */
		battery_unread,
		/* present and charging */
		battery_charging,
		/* present and discharging */
		battery_discharging,
		/* present but strange as neither of the above */
		battery_unknown
	} state;
	time_t lastupdated;
	long long lastcurrent, lastcharge, lastfull;
	struct field {
		struct battery *parent;
		const struct battery_field *field;
		time_t lastupdated;
		ProcMeterOutput output;
	} fields[BAT_OUTPUT_COUNT];
} *batteries = NULL;

static void free_batteries(struct battery *b) {
	while (b != NULL) {
		struct battery *h = b;
		b = h->next;

		close(h->fd);
		free(h->directory);
		free(h);
	}
}

static char *dirconcat(const char *dir, const char *name) {
	size_t dl, nl;
	char *r;

	dl = strlen(dir);
	nl = strlen(name);
	if (dl > 0 && dir[dl-1] == '/')
		dl--;
	r = malloc(dl + nl + 2);
	if (r == NULL) {
		fputs("Out of Memory\n", stderr);
		return NULL;
	}
	memcpy(r, dir, dl);
	r[dl] = '/';
	memcpy(r + dl + 1, name, nl+1);
	return r;
}

/* this assumes there are no other symlinks than the given one
 * (otherwise the ../ shortening does not work) */
static char *canonify_link(const char *basedir, int dirfd, const char *linkname) {
	char *r, *buffer = 0;
	size_t bufsize = 512;
	ssize_t got;
	const char *e, *s;

	do {
		char *h;
		h = realloc(buffer, bufsize);
		if (h == NULL) {
			free(buffer);
			return dirconcat(basedir, linkname);
			fputs("Out of Memory\n", stderr);
		}
		buffer = h;
		memset(buffer, 0, bufsize);

		got = readlinkat(dirfd, linkname, buffer, bufsize - 1);
		if (got < 0) {
			if (debug_battery) {
				fprintf(stderr, "Error %d reading link '%s' in '%s': %s\n",
						errno, linkname, basedir,
						strerror(errno));
			}
			free(buffer);
			return dirconcat(basedir, linkname);
		}
		if (got >= bufsize) {
			bufsize = got + 2;
			continue;
		}
	} while (false);
	e = basedir + strlen(basedir);
	if (e <= basedir) {
		free(buffer);
		if (debug_battery) {
			fputs("Confused in pathname canonisation!\n", stderr);
		}
		return dirconcat(basedir, linkname);
	}
	e--;
	while (e > basedir && *e == '/')
		e--;
	s = buffer;
	while (s[0] == '.') {
		if (s[1] == '/') {
			s += 2;
			continue;
		}
		if (s[1] != '.' || s[2] != '/')
			break;
		s += 3;
		while (e > basedir && *e != '/')
			e--;
		if (*e != '/') {
			free(buffer);
			if (debug_battery) {
				fputs("Confused in pathname canonisation!\n", stderr);
			}
			return dirconcat(basedir, linkname);
		}
		while (e > basedir && *e == '/')
			e--;
	}
	e++;
	r = malloc((e-basedir) + strlen(s) + 2);
	if (r == NULL) {
		free(buffer);
		fputs("Out of Memory\n", stderr);
		return dirconcat(basedir, linkname);
	}
	memcpy(r, basedir, e-basedir);
	r[e-basedir] = '/';
	strcpy(r + (e-basedir) + 1, s);
	free(buffer);
	return r;
}

static bool read_file(int dfd, const char *dirname, const char *filename, char *buffer, size_t buflen) {
	int fd;
	ssize_t got;

	fd = openat(dfd, filename, O_NOFOLLOW|O_RDONLY);
	if (fd < 0) {
		if (debug_battery) {
			fprintf(stderr, "Error %d opening file '%s' in '%s': %s\n",
					errno, filename, dirname,
					strerror(errno));
		}
		return false;
	}
	memset(buffer, 0, buflen);
	/* TODO: are /sys file garanteed to be read in one read? */
	got = read(fd, buffer, buflen-1);
	close(fd);
	if (got < 0 || got >= buflen) {
		if (debug_battery) {
			fprintf(stderr, "Error %d reading file '%s' in '%s': %s\n",
					errno, filename, dirname,
					strerror(errno));
		}
		return false;
	}
	while (got > 0 && buffer[got-1] == '\n')
		buffer[--got] = '\0';
	return true;
}



static bool is_battery(int dirfd, const char *dirname) {
	char value[9];

	if (!read_file(dirfd, dirname, "type", value, sizeof(value)))
		return false;
	return memcmp(value, "Battery", 8) == 0;
}

static struct field *new_output(struct battery *bat, const char *name, const char *description, char type, int interval) {
	struct field *out;

	out = &bat->fields[bat->output_count++];
	assert ( bat->output_count <= BAT_OUTPUT_COUNT);
	memset(out, 0, sizeof(ProcMeterOutput));

	out->parent = bat;
	snprintf(out->output.name, PROCMETER_NAME_LEN, name, bat->name);
	out->output.description = (char *)description;
	out->output.type = type;
	out->output.interval = interval;
	out->output.graph_scale = 1;
	strcpy(out->output.text_value, "not available");
	return out;
}

static bool fill_outputs(struct battery *bat, int dirfd) {
	struct field *f;
	const struct battery_field *bf;

	memset(bat->fields, 0, sizeof(bat->fields));
	bat->output_count = 0;

#define OUTPUT_OFS_PERCENT 0
	f = new_output(bat, "%s percent", "", PROCMETER_TEXT|PROCMETER_BAR, 10);
	if (f == NULL)
		return false;
	strcpy(f->output.text_value, "??%");
	strcpy(f->output.graph_units, "10%");
#define OUTPUT_OFS_TIMELEFT 1
	f = new_output(bat, "%s remaining", "Time left till charged or discharged", PROCMETER_TEXT, 10);
	if (f == NULL)
		return false;
	strcpy(f->output.text_value, "??:??:??");

#define OUTPUT_OFS_FIELDS 2
	for (bf = fields ; bf->name != NULL ; bf++) {
		int fd;

		fd = openat(dirfd, bf->name, O_NOFOLLOW|O_RDONLY);
		if (fd < 0) {
			if (bf->required)
				return false;
			continue;
		}
		close(fd);
		f = new_output(bat, bf->name_template,
			bf->description, PROCMETER_TEXT, 5);
		f->field = bf;
		if (f == NULL)
			return false;
		switch (bf->fieldtype) {
			case bft_muAh:
				f->output.type = PROCMETER_TEXT|PROCMETER_BAR;
				strcpy(f->output.graph_units, "(%d mAh)");
				f->output.graph_scale = 100;
				break;
			case bft_muA:
				f->output.type = PROCMETER_TEXT|PROCMETER_BAR|PROCMETER_GRAPH;
				strcpy(f->output.graph_units, "(%dmA)");
				f->output.graph_scale = 100;
				break;
			case bft_v:
				f->output.type = PROCMETER_TEXT|PROCMETER_BAR|PROCMETER_GRAPH;
				/* what units does this have? */
				strcpy(f->output.graph_units, "?V");
				break;
			default:
				break;
		}
	}
	return true;
};

static struct battery *find_batteries(const char *basepath, bool presentonly) {
	struct battery *list = NULL;
	struct dirent *ent;
	DIR *dir;
	int dir_fd;

	dir = opendir(basepath);
	if (basepath == NULL)
		return NULL;
	dir_fd = dirfd(dir);
	while ((ent = readdir(dir)) != NULL) {
		struct battery *n;
		char *dirname;
		int fd;
		char value[9];
		bool present;

		if (ent->d_name[0] == '.')
			continue;
		/* /sys should have d_type set, so no lstat dance */
		if (ent->d_type != DT_LNK)
			continue;
		dirname = canonify_link(basepath, dir_fd, ent->d_name);
		fd = open(dirname, O_RDONLY|O_DIRECTORY);
		if (fd < 0) {
			free(dirname);
			continue;
		}
		if (!is_battery(fd, dirname)) {
			close(fd);
			free(dirname);
			continue;
		}

		if (!read_file(fd, dirname, "present", value, sizeof(value))) {
			close(fd);
			free(dirname);
			continue;
		}
		present = value[0] == '1' && value[1] == '\0';
		if (presentonly && !present) {
			close(fd);
			free(dirname);
			continue;
		}
		n = calloc(sizeof(struct battery), 1);
		if (n == NULL) {
			close(fd);
			free(dirname);
			continue;
		}
		n->directory = dirname;
		dirname = NULL;
		n->name = strdup(ent->d_name);
		n->fd = fd;
		n->state = present ? battery_unread : battery_unavailable;
		if (!fill_outputs(n, fd)) {
			free_batteries(n);
			continue;
		}
		n->next = list;
		list = n;
	}
	closedir(dir);
	return list;
}

static ProcMeterOutput **output_list = NULL;

ProcMeterOutput **Initialise(char *options) {
	struct battery *b;
	ProcMeterOutput **o;
	int count;
	bool only_present = false;

	if (options != NULL && strstr(options, "debug") != NULL) {
		debug_battery = true;
		fprintf(stderr, "enabled debug mode\n");
	}
	if (options != NULL && strstr(options, "onlypresent") != NULL) {
		only_present = true;
	}
	free_batteries(batteries);
	batteries = find_batteries("/sys/class/power_supply", only_present);
	count = 0;
	for (b = batteries ; b != NULL ; b = b->next)
		count += b->output_count;
	output_list = calloc(sizeof(ProcMeterOutput *), count + 1);
	if (output_list == NULL)
		return NULL;
	o = output_list;
	for (b = batteries ; b != NULL ; b = b->next) {
		for (count = 0 ; count < b->output_count ; count++ ) {
			*(o++) = &b->fields[count].output;
		}
	}
	return(output_list);
}

static inline void battery_setstate(struct battery *bat, enum battery_state state) {
	if (bat->state == state)
		return;
	bat->state = state;
	if (state == battery_unavailable) {
		/* everything unavailable anyway... */
		return;
	} else if (state == battery_unread) {
		int i;

		for (i = 0 ; i < bat->output_count ; i++) {
			bat->fields[i].lastupdated = 0;
		}
	} else
		/* don't use an old rate if switching between
		 * charging and discharging or back */
		bat->fields[OUTPUT_OFS_FIELDS + FIELD_OFS_CURRENT].lastupdated = 0;
}

void update_presence(time_t timenow, struct battery *bat) {
	char value[9];
	bool present;

	if (bat->lastupdated != 0 && bat->lastupdated == timenow)
		return;
	bat->lastupdated = timenow;
	if (bat->fd < 0 || !read_file(bat->fd, bat->directory, "present", value, sizeof(value))) {
		if (bat->fd >= 0)
			close(bat->fd);
		bat->fd = open(bat->directory, O_RDONLY|O_DIRECTORY);
		if (bat->fd < 0 || !read_file(bat->fd, bat->directory, "present", value, sizeof(value))) {
			battery_setstate(bat, battery_unavailable);
			return;
		}
	}
	present = value[0] == '1' && value[1] == '\0';
	if (present) {
		if (bat->state == battery_unavailable) {
			/* battery newly available, invalidate all data */
			battery_setstate(bat, battery_unread);
		}
	} else {
		battery_setstate(bat, battery_unavailable);
	}
}

static int update_field(time_t timenow, struct field *f) {
	struct battery *bat = f->parent;

	update_presence(timenow, bat);
	if (bat->state == battery_unavailable) {
		f->output.graph_value = 0;
		strcpy(f->output.text_value, "not available");
		return 0;
	}
	if (f->lastupdated != 0) {
		if (f->lastupdated <= timenow && f->lastupdated + 5 > timenow)
			return 0;
		if (!f->field->variable && f->lastupdated + 60 > timenow)
			return 0;
	}

	if (f->field != NULL) {
		const struct battery_field *bf = f->field;
		char text[PROCMETER_TEXT_LEN + 3];
		long long l;

		if (!read_file(bat->fd, bat->directory,
					bf->name, text, PROCMETER_TEXT_LEN))
			return -1;

		switch (bf->fieldtype) {
			case bft_string:
				if ((f - bat->fields) == OUTPUT_OFS_FIELDS +
						FIELD_OFS_STATUS) {
					if (strcmp(text, "Discharging") == 0)
						battery_setstate(bat,
							battery_discharging);
					else if (strcmp(text, "Charging") == 0)
						battery_setstate(bat,
							battery_charging);
					else
						battery_setstate(bat,
							battery_unknown);

				}
				memcpy(f->output.text_value, text, PROCMETER_TEXT_LEN);
				f->lastupdated = timenow;
				return 0;
			default:
				break;
		}
		l = atoll(text);
		if (f - bat->fields == OUTPUT_OFS_FIELDS + FIELD_OFS_CURRENT) {
			bat->lastcurrent = l;
		} else if (f - bat->fields == OUTPUT_OFS_FIELDS + FIELD_OFS_CHARGE) {
			bat->lastcharge = l;
		} else if (f - bat->fields == OUTPUT_OFS_FIELDS + FIELD_OFS_FULL) {
			bat->lastfull = l;
		}
		switch (bf->fieldtype) {
			case bft_string:
				assert (false);
				break;
			case bft_muAh:
				f->output.graph_value = (l*PROCMETER_GRAPH_SCALE)/100000;
				snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
						"%lu mAh", (unsigned long)(l / 1000));
				break;
			case bft_muA:
				f->output.graph_value = (l*PROCMETER_GRAPH_SCALE)/100000;
				snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
						"%lu mA", (unsigned long)(l / 1000));
				break;
			case bft_v:
				f->output.graph_value = (l*PROCMETER_GRAPH_SCALE)/100000;
				snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
						"%lu mV", (unsigned long)l);
				break;
			case bft_yesno:
				f->output.graph_value = l;
				if (l == 0)
					strcpy(f->output.text_value, "no");
				else if (l == 1)
					strcpy(f->output.text_value, "yes");
				else
					snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
						"unknown (%lld)", l);
				break;
			case bft_count:
				f->output.graph_value = l;
				memcpy(f->output.text_value, text, PROCMETER_TEXT_LEN);
				break;
		}
	} else if (f - bat->fields == OUTPUT_OFS_PERCENT) {
		unsigned int percent;

		if (update_field(timenow, bat->fields + OUTPUT_OFS_FIELDS
				+ FIELD_OFS_FULL) != 0)
			return -1;
		if (update_field(timenow, bat->fields + OUTPUT_OFS_FIELDS
				+ FIELD_OFS_CHARGE) != 0)
			return -1;
		if (bat->lastfull == 0) {
			strcpy(f->output.text_value, "not available");
			return 0;
		}
		percent = ((PROCMETER_GRAPH_SCALE*100)*bat->lastcharge)/
					bat->lastfull;
		snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
				"%u%%", (unsigned int)(percent /
					PROCMETER_GRAPH_SCALE));
		f->output.graph_value = percent/10;
	} else if (f - bat->fields == OUTPUT_OFS_TIMELEFT) {
		long seconds, minutes, hours;

		if (update_field(timenow, bat->fields + OUTPUT_OFS_FIELDS
				+ FIELD_OFS_STATUS) != 0)
			return -1;
		if (bat->state != battery_charging &&
				bat->state != battery_discharging) {
			strcpy(f->output.text_value, "not available");
			return 0;
		}
		if (update_field(timenow, bat->fields + OUTPUT_OFS_FIELDS
					+ FIELD_OFS_CURRENT) != 0)
			return -1;
		if (bat->lastcurrent == 0) {
			strcpy(f->output.text_value, "never");
			return 0;
		}
		if (update_field(timenow, bat->fields + OUTPUT_OFS_FIELDS
				+ FIELD_OFS_CHARGE) != 0)
			return -1;
		if (bat->state == battery_discharging) {
			seconds = (3600 * bat->lastcharge) / bat->lastcurrent;
		} else {
			long long left;
			if (update_field(timenow, bat->fields + OUTPUT_OFS_FIELDS
						+ FIELD_OFS_FULL) != 0)
				return -1;
			left = bat->lastfull - bat->lastcharge;
			seconds = (3600 * left) / bat->lastcurrent;
		}
		minutes = seconds / 60;
		seconds = seconds % 60;
		hours = minutes / 60;
		minutes = minutes % 60;
		snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
				"%2lu:%02lu:%02lu", hours, minutes, seconds);
	}

	return 0;
}

#define get_container(container, field, pointer) (struct container *)((char *)pointer - (((char *)&((struct container *)NULL)->field)-(char*)NULL))

int Update(time_t timenow, ProcMeterOutput *o) {
	/* ugly trick around ProcMeterOutput not having a privdata */
	return update_field(timenow, get_container(field, output, o));
}

void Unload(void)
{
	free_batteries(batteries);
	batteries = NULL;
	free(output_list);
	output_list = NULL;
}
#endif