#include "db_handler.h"
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <rrd.h>

int db_create(const char *dbName) {
    // Arguments for rrd_create_r
    const char *ds_args[] = {
        "DS:latitude:GAUGE:120:U:U",
        "DS:longitude:GAUGE:120:U:U",
        "DS:elevation:GAUGE:120:U:U",
        "DS:sqmModel:GAUGE:120:U:U",
        "DS:sqmSerial:GAUGE:120:U:U",
        "DS:mpsqa:GAUGE:120:U:U",
        "DS:sensorTemp:GAUGE:120:U:U",
        "DS:siteTemp:GAUGE:120:U:U",
        "DS:sitePressure:GAUGE:120:U:U",
        "DS:siteHumidity:GAUGE:120:U:U",
        "RRA:AVERAGE:0.5:1:1440"
    };
    int ds_argc = sizeof(ds_args) / sizeof(ds_args[0]);
    optind = 0;
    rrd_clear_error();
    if (rrd_create_r(dbName, 60, 0, ds_argc, ds_args) == -1) {
        fprintf(stderr, "RRD create error: %s\n", rrd_get_error());
        return -1;
    }
    return 0;
}

int db_add_entry(const char *dbName, const DBEntry *entry) {
    // Prepare timestamp (date + time to UNIX timestamp)
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    strptime(entry->date, "%Y-%m-%d", &tm);
    strptime(entry->time, "%H:%M:%S", &tm);
    time_t t = mktime(&tm);
    char timestamp[32];
    snprintf(timestamp, sizeof(timestamp), "%ld", (long)t);
    // Prepare update string
    char update[512];
    snprintf(update, sizeof(update), "%s:%.8f:%.8f:%.8f:%d:%d:%.8f:%.8f:%.8f:%.8f:%.8f",
        timestamp,
        entry->latitude,
        entry->longitude,
        entry->elevation,
        entry->sqmModel,
        entry->sqmSerial,
        entry->mpsqa,
        entry->sensorTemp,
        entry->siteTemp,
        entry->sitePressure,
        entry->siteHumidity);
    const char *upd_args[] = { update };
    int upd_argc = 1;
    optind = 0;
    rrd_clear_error();
    if (rrd_update_r(dbName, NULL, upd_argc, upd_args) == -1) {
        fprintf(stderr, "RRD update error: %s\n", rrd_get_error());
        return -1;
    }
    return 0;
}

int db_fetch_entries(const char *dbName, time_t start, time_t end, char ***ds_names, unsigned long *step, unsigned long *nrows, rrd_value_t **data) {
    // Use rrd_fetch to get data between start and end
    const char *cf = "AVERAGE";
    optind = 0;
    rrd_clear_error();
    *ds_names = NULL;
    *data = NULL;
    int ret = rrd_fetch_r(dbName, cf, &start, &end, step, nrows, ds_names, data);
    if (ret != 0) {
        fprintf(stderr, "RRD fetch error: %s\n", rrd_get_error());
        return -1;
    }
    return 0;
}

int db_delete_entry(const char *dbName, const char *date, const char *time) {
    // Not supported by rrdtool; would require export, edit, and re-import
    printf("[STUB] Delete entry from %s at %s %s\n", dbName, date, time);
    return -1;
}

int db_delete(const char *dbName) {
    // Delete the RRD file using remove()
    if (remove(dbName) == 0) return 0;
    return -1;
}
