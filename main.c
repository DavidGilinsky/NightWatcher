/*
 * Project: NightWatcher
 * File: main.c
 * Author: David Gilinsky - gilinsky@gilinskyresearch.com
 * Date: 5 June 2025
 */
#define _GNU_SOURCE
#include "main.h"
#include "sqm-le/sqm_le.h"
#include <stdio.h>
#include <string.h>
#include "config_file_handler/config_file_handler.h"
#include "db_handler/db_handler.h"
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

/*
 * Handles SIGHUP (hangup) signal.
 * Parameter: signum - the signal number.
 */
void handle_sighup(int signum) {
    (void)signum;
    printf("Received SIGHUP (reload configuration or reinitialize as needed).\n");
    // Add logic to reload configuration or reinitialize if needed
}

/*
 * Handles SIGTERM (termination) signal.
 * Parameter: signum - the signal number.
 */
void handle_sigterm(int signum) {
    (void)signum;
    printf("Received SIGTERM (shutting down gracefully).\n");
    // Add logic to clean up and exit gracefully
    exit(0);
}

// Struct to pass to thread
typedef struct {

    SQM_LE_Device dev;
    GlobalConfig site;
} ThreadArgs;

/*
 * Thread function to perform a reading from the SQM-LE device and add the result to the database.
 * Parameters: arg - pointer to ThreadArgs containing device and site configuration.
 * Returns: NULL.
 */
void* sqm_reading_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    SQM_LE_Device* dev = &args->dev;
    GlobalConfig* site = &args->site;

    int ret = getReading(dev, site);
    if (ret == 0) {
        printf("Site name: %s\n", site->siteName);
        printf("SQM-LE Reading: %s\n", dev->last_reading);
        printf("Magnitude per square arc second (mpsqa): %f\n", dev->mpsqa);
        printf("Sensor temperature (C): %f\n", dev->sensorTemp);

        // Prepare DBEntry and add to database
        DBEntry entry = {0};
        // Set date and time to current system time
        time_t now = time(NULL);
        struct tm *tm_now = localtime(&now);
        strftime(entry.date, sizeof(entry.date), "%Y-%m-%d", tm_now);
        strftime(entry.time, sizeof(entry.time), "%H:%M:%S", tm_now);
        strncpy(entry.siteName, site->siteName, sizeof(entry.siteName));
        entry.latitude = site->latitude;
        entry.longitude = site->longitude;
        entry.elevation = site->elevation;
        entry.sqmModel = site->sqmModel;
        entry.sqmSerial = site->sqmSerial;
        entry.mpsqa = dev->mpsqa;
        entry.sensorTemp = dev->sensorTemp;
        entry.siteTemp = 0.0f;      // Placeholder, set as needed
        entry.sitePressure = 0.0f;  // Placeholder, set as needed
        entry.siteHumidity = 0.0f;  // Placeholder, set as needed
        if (db_add_entry(site->dbName, &entry) != 0) {
            printf("Failed to add entry to database\n");
        }
    } else {
        printf("Failed to get reading, error code: %d\n", ret);
    }
    return NULL;
}

char default_config_file[] = "./conf/nwconf.conf";

/*
 * Loads the site configuration from the default config file into the provided GlobalConfig struct.
 * Parameters: site - pointer to GlobalConfig struct to populate.
 * Returns: 0 on success, nonzero on error.
 */
int load_site_config(GlobalConfig *site) {
    return read_config(site, default_config_file);
}

/*
 * Main entry point for the NightWatcher application.
 * Loads configuration, creates the database if needed, and launches the reading thread with timeout.
 * Returns: 0 on success, nonzero on error.
 */
int main(void) {

    // Register signal handlers for SIGHUP and SIGTERM
    signal(SIGHUP, handle_sighup);
    signal(SIGTERM, handle_sigterm);

    SQM_LE_Device dev;
    GlobalConfig site;
    // Load site configuration from file
    if (load_site_config(&site) != 0) {
        printf("Failed to load site configuration from %s\n", default_config_file);
        return 1;
    }
    // Assign dev struct elements from site struct
    strncpy(dev.ip, site.sqmIP, sizeof(dev.ip));
    dev.port = site.sqmPort;
    dev.socket_fd = -1;
    dev.last_reading[0] = '\0';

    // Check if site.enableReadOnStartup is true, then set site.enableSQMread to true
    site.enableSQMread = site.enableReadOnStartup;  

    // Create the database if it does not exist
    if (access(site.dbName, F_OK) != 0) {
        if (db_create(site.dbName) != 0) {
            printf("Failed to create database: %s\n", site.dbName);
            return 1;
        }
    }

    // Get unit information before starting reading thread
    getUnitInformation(&dev, &site);

    // Only create the reading thread if the SQM is healthy after unit information retrieval
    // and if site.enableSQMread is true
    if (site.sqmHealthy == true && site.enableSQMread == true) {
        ThreadArgs args;
        args.dev = dev;
        args.site = site;
        pthread_t tid;
        pthread_create(&tid, NULL, sqm_reading_thread, &args);

        // Timeout mechanism
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += site.sqmReadTimeout;

    #if defined(_GNU_SOURCE)
        int join_ret = pthread_timedjoin_np(tid, NULL, &ts);
    #else
        // Fallback: poll with sleep (not as precise, but portable)
        int join_ret = 1;
        for (unsigned int waited = 0; waited < site.sqmReadTimeout; ++waited) {
            if (pthread_tryjoin_np(tid, NULL) == 0) {
                join_ret = 0;
                break;
            }
            sleep(1);
        }
    #endif
        if (join_ret != 0) {
            printf("Thread timed out, cancelling...\n");
            pthread_cancel(tid);
            pthread_join(tid, NULL);
            site.sqmHealthy = false;
        }
    } else {
        // Do not create the reading thread if the SQM is not healthy
        printf("SQM is not healthy after unit information retrieval. Reading thread will not be started.\n");
    }

    
    printf("site.sqmHealthy: %s\n", site.sqmHealthy ? "true" : "false");
    return 0;
}
