#ifndef DB_HANDLER_H
#define DB_HANDLER_H

#include "../main.h"
#include <stdint.h>
#include <time.h>
#include <rrd.h>

// Structure for a database entry
typedef struct {
    char date[16];           // YYYY-MM-DD
    char time[16];           // HH:MM:SS
    char siteName[256];
    float latitude;
    float longitude;
    float elevation;
    int sqmModel;
    int sqmSerial;
    float mpsqa;
    float sensorTemp;
    float siteTemp;
    float sitePressure;
    float siteHumidity;
} DBEntry;

// Create a new RRD database with the given name (site.dbName)
int db_create(const char *dbName);
// Add an entry to the database
int db_add_entry(const char *dbName, const DBEntry *entry);
// Fetch entries using rrd_fetch()
int db_fetch_entries(const char *dbName, time_t start, time_t end, char ***ds_names, unsigned long *step, unsigned long *nrows, rrd_value_t **data);
// Delete an entry (by date/time or index)
int db_delete_entry(const char *dbName, const char *date, const char *time);
// Delete the entire database
int db_delete(const char *dbName);

#endif // DB_HANDLER_H
