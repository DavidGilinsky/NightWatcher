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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "command_handler/command_handler.h"

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
    SQM_LE_Device *dev;
    GlobalConfig *site;
} ThreadArgs;

// TCP listener thread function
void* tcp_listener_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    SQM_LE_Device* dev = args->dev;
    GlobalConfig* site = args->site;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        free(args);
        pthread_exit(NULL);
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(site->controlPort);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        free(args);
        pthread_exit(NULL);
    }
    listen(server_fd, 5);
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd >= 0) {
            char buf[256] = {0};
            ssize_t n = read(client_fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                char response[256] = {0};
                handle_command(buf, response, sizeof(response), site, dev);
                write(client_fd, response, strlen(response));
            }
            close(client_fd);
        }
        usleep(10000); // Sleep 10ms to avoid busy loop
    }
    close(server_fd);
    free(args);
    pthread_exit(NULL);
}

/*
 * Thread function to perform a reading from the SQM-LE device and add the result to the database.
 * Parameters: arg - pointer to ThreadArgs containing device and site configuration.
 * Returns: NULL.
 */
void* sqm_reading_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    SQM_LE_Device* dev = args->dev;
    GlobalConfig* site = args->site;

    int ret = getReading(dev, site);
    free(args);
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
 * Launches the SQM reading thread if the device is healthy and reading is enabled.
 * Handles timeout and updates health status.
 */
void launch_sqm_read_thread(SQM_LE_Device *dev, GlobalConfig *site) {
    if (site->sqmHealthy == true && site->enableSQMread == true) {
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->dev = dev;
        args->site = site;
        pthread_t tid;
        pthread_create(&tid, NULL, sqm_reading_thread, args);

        // Timeout mechanism
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += site->sqmReadTimeout;

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
            site->sqmHealthy = false;
        }
    } else {
        printf("SQM is not healthy or reading is not enabled. Reading thread will not be started.\n");
    }
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

    // Initialize health variables to false until we get positive indication
    site.sqmHealthy = false;
    dev.reading_ready = false;
    
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

    // Launch TCP listener in a separate thread
    pthread_t tcp_thread;
    ThreadArgs *tcp_args = malloc(sizeof(ThreadArgs));
    tcp_args->dev = &dev;
    tcp_args->site = &site;
    pthread_create(&tcp_thread, NULL, tcp_listener_thread, tcp_args);


    // Main loop: periodically check unit information and launch reading thread
    time_t last_heartbeat = 0;
    time_t last_read = 0;
    while (1) {
        time_t now = time(NULL);
        // Heartbeat: check unit information
        if (now - last_heartbeat >= site.sqmHeartbeatInterval) {
            getUnitInformation(&dev, &site);
            last_heartbeat = now;
            printf("site.sqmHealthy: %s\n", site.sqmHealthy ? "true" : "false");
        }
        // Reading: launch reading thread if interval elapsed
        if (now - last_read >= site.readingInterval) {
            if (site.sqmHealthy == true && site.enableSQMread == true) {
            launch_sqm_read_thread(&dev, &site);
            last_read = now;
            }
        }
        sleep(1);
    }

    return 0;
}
