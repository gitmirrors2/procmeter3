/***************************************
  ProcMeter - A system monitoring program for Linux

  battery values from /sys/ (for example new-style ACPI)

  This file Copyright 2011, 2012 Bernhard R. Link
  Modified 2012, 2015 by Andrew M. Bishop.

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
                bft_muA, bft_muAh, bft_v,
                bft_uW, bft_uWh, bft_percent }
                        fieldtype;
        bool variable, required;
        const char *description, *name_template;
} fields[] = {
#define FIELD_OFS_STATUS 0
        { "status", bft_string, true, true,
                "Status (charging/discharging).", "%s_state"},
#define FIELD_OFS_CURRENT 1
        { "current_now", bft_muA, true, false,
                "Current charging/discharging rate.", "%s_rate"},
#define FIELD_OFS_CHARGE 2
        { "charge_now", bft_muAh, true, false,
                "Current charge of the battery.", "%s_charge"},
#define FIELD_OFS_CHARGE_FULL 3
        { "charge_full", bft_muAh, false, false,
                "Last full charge of the battery.", "%s_last_full"},
#define FIELD_OFS_CHARGE_DESIGN_FULL 4
        { "charge_full_design", bft_muAh, false, false,
                "Designed full charge of the battery.", "%s_design_full"},
#define FIELD_OFS_POWER 5
        { "power_now", bft_uW, true, false,
                "Current power consumption from the battery.", "%s_power"},
#define FIELD_OFS_ENERGY 6
        { "energy_now", bft_uWh, true, false,
                "Current energy of the battery.", "%s_energy"},
#define FIELD_OFS_ENERGY_FULL 7
        { "energy_full", bft_uWh, false, false,
                "Last full energy of the battery.", "%s_last_full"},
#define FIELD_OFS_ENERGY_DESIGN_FULL 8
        { "energy_full_design", bft_uWh, false, false,
                "Designed full energy of the battery.", "%s_design_full"},
        { "capacity", bft_percent, true, false,
                "Percentage capacity of the battery.", "%s_capacity"},
//      { "cycle_count", bft_count, false, false,
//              "Content of the 'cycle_count' file.", "%s_cycle count"},
        { "manufacturer", bft_string, false, false,
                "Manufacturer.", "%s_manufacturer"},
        { "model_name", bft_string, false, false,
                "Model name.", "%s_model_name"},
        { "serial_number", bft_string, false, false,
                "Serial number.", "%s_serial_number"},
        { "technology", bft_string, false, false,
                "Technology.", "%s_technology"},
        { "voltage_min_design", bft_v, false, false,
                "Minimum design voltage.", "%s_min_voltage"},
        { "voltage_now", bft_v, true, false,
                "Current voltage.", "%s_voltage"},
#define FIELD_OFS_CHARGE_PERCENT 16
        { NULL, 0},
#define FIELD_OFS_ENERGY_PERCENT 17
        { NULL, 0},
#define FIELD_OFS_CHARGE_TIMELEFT 18
        { NULL, 0},
#define FIELD_OFS_ENERGY_TIMELEFT 19
        { NULL, 0}
};

#define BAT_OUTPUT_COUNT (sizeof(fields)/sizeof(struct battery_field))

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
        long long lastcurrent, lastpower;
        long long lastcharge, lastchargefull, designchargefull;
        long long lastenergy, lastenergyfull, designenergyfull;
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
        int i;

        memset(bat->fields, 0, sizeof(bat->fields));
        bat->output_count = 0;

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
                               bf->description, PROCMETER_TEXT, bf->variable ? 5 : 60);
                f->field = bf;
                if (f == NULL)
                        return false;
                switch (bf->fieldtype) {
                        case bft_muAh:
                                if(bf->variable)
                                        f->output.type = PROCMETER_TEXT|PROCMETER_BAR|PROCMETER_GRAPH;
                                strcpy(f->output.graph_units, "(%dmAh)");
                                f->output.graph_scale = 100;
                                break;
                        case bft_muA:
                                f->output.type = PROCMETER_TEXT|PROCMETER_BAR|PROCMETER_GRAPH;
                                strcpy(f->output.graph_units, "(%dmA)");
                                f->output.graph_scale = 100;
                                break;
                        case bft_v:
                                if(bf->variable)
                                        f->output.type = PROCMETER_TEXT|PROCMETER_BAR|PROCMETER_GRAPH;
                                strcpy(f->output.graph_units, "(%dV)");
				f->output.graph_scale = 1;
                                break;
                        case bft_uWh:
                                if(bf->variable)
                                        f->output.type = PROCMETER_TEXT|PROCMETER_BAR|PROCMETER_GRAPH;
                                strcpy(f->output.graph_units, "(%dWh)");
				f->output.graph_scale = 10;
                                break;
                        case bft_uW:
                                if(bf->variable)
                                        f->output.type = PROCMETER_TEXT|PROCMETER_BAR|PROCMETER_GRAPH;
                                strcpy(f->output.graph_units, "(%dW)");
				f->output.graph_scale = 1;
                                break;
                        case bft_percent:
                                if(bf->variable)
                                        f->output.type = PROCMETER_TEXT|PROCMETER_BAR|PROCMETER_GRAPH;
                                strcpy(f->output.graph_units, "(%d%%)");
				f->output.graph_scale = 20;
                                break;
                        default:
                                break;
                }
        }

        for(i=0;i<BAT_OUTPUT_COUNT;i++)
          {
           int charge_fields=0,energy_fields=0;

           for(i=0;i<BAT_OUTPUT_COUNT;i++)
              if(bat->fields[i].field)
                {
                 if(bat->fields[i].field==&fields[FIELD_OFS_STATUS] ||
                    bat->fields[i].field==&fields[FIELD_OFS_CURRENT] ||
                    bat->fields[i].field==&fields[FIELD_OFS_CHARGE] ||
                    bat->fields[i].field==&fields[FIELD_OFS_CHARGE_FULL])
                    charge_fields++;

                 if(bat->fields[i].field==&fields[FIELD_OFS_STATUS] ||
                    bat->fields[i].field==&fields[FIELD_OFS_POWER] ||
                    bat->fields[i].field==&fields[FIELD_OFS_ENERGY] ||
                    bat->fields[i].field==&fields[FIELD_OFS_ENERGY_FULL])
                    energy_fields++;
                }

           if(charge_fields==4)
             {
              f = new_output(bat, "%s_percent", "The percentage of design charge in the battery.", PROCMETER_TEXT|PROCMETER_BAR|PROCMETER_GRAPH, 10);
              if (f == NULL)
                 return false;
              f->field = &fields[FIELD_OFS_CHARGE_PERCENT];
              strcpy(f->output.text_value, "??%");
              strcpy(f->output.graph_units, "(%d%%)");
              f->output.graph_scale = 20;

              f = new_output(bat, "%s_remaining", "Time left till charged or discharged.", PROCMETER_TEXT, 10);
              if (f == NULL)
                 return false;
              f->field = &fields[FIELD_OFS_CHARGE_TIMELEFT];
              strcpy(f->output.text_value, "??:??:??");
             }

           else if(energy_fields==4)
             {
              f = new_output(bat, "%s_percent", "The percentage of design energy in the battery.", PROCMETER_TEXT|PROCMETER_BAR|PROCMETER_GRAPH, 10);
              if (f == NULL)
                 return false;
              f->field = &fields[FIELD_OFS_ENERGY_PERCENT];
              strcpy(f->output.text_value, "??%");
              strcpy(f->output.graph_units, "(%d%%)");
              f->output.graph_scale = 20;

              f = new_output(bat, "%s_remaining", "Time left till charged or discharged.", PROCMETER_TEXT, 10);
              if (f == NULL)
                 return false;
              f->field = &fields[FIELD_OFS_ENERGY_TIMELEFT];
              strcpy(f->output.text_value, "??:??:??");
             }
          }

        return true;
};

static struct battery *find_batteries(const char *basepath, bool presentonly) {
        struct battery *list = NULL;
        struct dirent *ent;
        DIR *dir;
        int dir_fd;

        if (basepath == NULL)
                return NULL;

        dir = opendir(basepath);
        if (dir == NULL)
                return NULL;

        dir_fd = dirfd(dir);
        if (dir_fd == -1)
                return NULL;

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
          {
           int i;

           /* don't use an old rate if switching between
            * charging and discharging or back */

           for(i=0;i<BAT_OUTPUT_COUNT;i++)
              if(bat->fields[i].field)
                 if(bat->fields[i].field==&fields[FIELD_OFS_CURRENT] ||
                    bat->fields[i].field==&fields[FIELD_OFS_POWER])
                    bat->fields[i].lastupdated = 0;
          }
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

        if (f->field != NULL && f->field->name != NULL) {
                const struct battery_field *bf = f->field;
                char text[PROCMETER_TEXT_LEN + 3];
                long long l;

                if (!read_file(bat->fd, bat->directory,
                                        bf->name, text, PROCMETER_TEXT_LEN))
                        return -1;

                switch (bf->fieldtype) {
                        case bft_string:
                                if (f->field == &fields[FIELD_OFS_STATUS]) {
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

                if (f->field == &fields[FIELD_OFS_CURRENT]) {
                        bat->lastcurrent = l;
                } else if (f->field == &fields[FIELD_OFS_CHARGE]) {
                        bat->lastcharge = l;
                } else if (f->field == &fields[FIELD_OFS_CHARGE_FULL]) {
                        bat->lastchargefull = l;
		} else if (f->field == &fields[FIELD_OFS_CHARGE_DESIGN_FULL]) {
			bat->designchargefull = l;
                } else if (f->field == &fields[FIELD_OFS_POWER]) {
                        bat->lastpower = l;
                } else if (f->field == &fields[FIELD_OFS_ENERGY]) {
                        bat->lastenergy = l;
                } else if (f->field == &fields[FIELD_OFS_ENERGY_FULL]) {
                        bat->lastenergyfull = l;
		} else if (f->field == &fields[FIELD_OFS_ENERGY_DESIGN_FULL]) {
			bat->designenergyfull = l;
                }
                switch (bf->fieldtype) {
                        case bft_string:
                                assert (false);
                                break;
                        case bft_muAh:
                                f->output.graph_value = (l*PROCMETER_GRAPH_SCALE)/100000;
                                snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
						"%.0f mAh", (double)l / 1000.0);
                                break;
                        case bft_muA:
                                f->output.graph_value = (l*PROCMETER_GRAPH_SCALE)/100000;
                                snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
						"%.0f mA", (double)l / 1000.0);
                                break;
                        case bft_v:
                                f->output.graph_value = (l*PROCMETER_GRAPH_SCALE)/100000;
                                snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
                                                "%.1f V", (double)l / 1000000.0);
                                break;
                        case bft_uWh:
                                f->output.graph_value = (l*PROCMETER_GRAPH_SCALE)/100000;
                                snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
                                                "%.1f Wh", (double)l / 1000000.0);
                                break;
                        case bft_uW:
                                f->output.graph_value = (l*PROCMETER_GRAPH_SCALE)/100000;
                                snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
                                                "%.1f W", (double)l / 1000000.0);
                                break;
                        case bft_percent:
                                f->output.graph_value = (l*PROCMETER_GRAPH_SCALE)/20;
                                snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
                                                "%.1f %%", (double)l);
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
        } else if (f->field == &fields[FIELD_OFS_CHARGE_PERCENT]) {
                unsigned int percent,i;

                for(i=0;i<BAT_OUTPUT_COUNT;i++)
                   if(bat->fields[i].field)
                      if(bat->fields[i].field==&fields[FIELD_OFS_CHARGE] ||
                         bat->fields[i].field==&fields[FIELD_OFS_CHARGE_DESIGN_FULL])
                         if (update_field(timenow, &bat->fields[i]) != 0)
                            return -1;

                if (bat->designchargefull == 0) {
                        strcpy(f->output.text_value, "not available");
                        return 0;
                }
                percent = (100*bat->lastcharge)/bat->designchargefull;
                snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
                                "%u%%", percent);
                f->output.graph_value = PROCMETER_GRAPH_FLOATING(percent)/f->output.graph_scale;
        } else if (f->field == &fields[FIELD_OFS_ENERGY_PERCENT]) {
                unsigned int percent,i;

                for(i=0;i<BAT_OUTPUT_COUNT;i++)
                   if(bat->fields[i].field)
                      if(bat->fields[i].field==&fields[FIELD_OFS_ENERGY] ||
                         bat->fields[i].field==&fields[FIELD_OFS_ENERGY_DESIGN_FULL])
                         if (update_field(timenow, &bat->fields[i]) != 0)
                            return -1;

                if (bat->designenergyfull == 0) {
                        strcpy(f->output.text_value, "not available");
                        return 0;
                }
                percent = (100*bat->lastenergy)/bat->designenergyfull;
                snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
                         "%u%%", percent);
                f->output.graph_value = PROCMETER_GRAPH_FLOATING(percent)/f->output.graph_scale;
        } else if (f->field == &fields[FIELD_OFS_CHARGE_TIMELEFT]) {
                long seconds, minutes, hours, i;

                for(i=0;i<BAT_OUTPUT_COUNT;i++)
                   if(bat->fields[i].field)
                      if(bat->fields[i].field==&fields[FIELD_OFS_STATUS] ||
                         bat->fields[i].field==&fields[FIELD_OFS_CURRENT] ||
                         bat->fields[i].field==&fields[FIELD_OFS_CHARGE] ||
                         bat->fields[i].field==&fields[FIELD_OFS_CHARGE_FULL])
                         if (update_field(timenow, &bat->fields[i]) != 0)
                            return -1;

                if (bat->state != battery_charging &&
                                bat->state != battery_discharging) {
                        strcpy(f->output.text_value, "not available");
                        return 0;
                }
                if (bat->lastcurrent == 0) {
                        strcpy(f->output.text_value, "never");
                        return 0;
                }
                if (bat->state == battery_discharging) {
                        seconds = (3600 * bat->lastcharge) / abs(bat->lastcurrent);
                } else {
                        seconds = (3600 * (bat->lastchargefull - bat->lastcharge)) / bat->lastcurrent;
                }
                minutes = seconds / 60;
                seconds = seconds % 60;
                hours = minutes / 60;
                minutes = minutes % 60;
                snprintf(f->output.text_value, PROCMETER_TEXT_LEN,
                                "%2lu:%02lu:%02lu", hours, minutes, seconds);
        } else if (f->field == &fields[FIELD_OFS_ENERGY_TIMELEFT]) {
                long seconds, minutes, hours, i;

                for(i=0;i<BAT_OUTPUT_COUNT;i++)
                   if(bat->fields[i].field)
                      if(bat->fields[i].field==&fields[FIELD_OFS_STATUS] ||
                         bat->fields[i].field==&fields[FIELD_OFS_POWER] ||
                         bat->fields[i].field==&fields[FIELD_OFS_ENERGY] ||
                         bat->fields[i].field==&fields[FIELD_OFS_ENERGY_FULL])
                         if (update_field(timenow, &bat->fields[i]) != 0)
                            return -1;

                if (bat->state != battery_charging &&
                                bat->state != battery_discharging) {
                        strcpy(f->output.text_value, "not available");
                        return 0;
                }
                if (bat->lastpower == 0) {
                        strcpy(f->output.text_value, "never");
                        return 0;
                }
                if (bat->state == battery_discharging) {
                        seconds = (3600 * bat->lastenergy) / bat->lastpower;
                } else {
                        seconds = (3600 * (bat->lastenergyfull - bat->lastenergy)) / bat->lastpower;
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
