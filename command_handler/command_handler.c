/*
 * Project: NightWatcher
 * File: command_handler.c
 * Author: David Gilinsky - gilinsky@gilinskyresearch.com
 * Date: 5 June 2025
 */
#include "nightwatcher.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Helper: trim leading/trailing whitespace in place
static void trim_whitespace(char *str) {
    // Trim leading
    char *start = str;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != str) memmove(str, start, strlen(start) + 1);
    // Trim trailing
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) *end-- = '\0';
}

// Command: status
// Return the overall status of the site.
// Return formation should be Status:[parameter]:[value]\n 
void command_status(char *words[], int nwords, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev, AW_WeatherData *weatherData) {
    (void)words; (void)nwords; (void)dev;
    char temp[256];
    if (site) {
        snprintf(response, response_size, "Status:SQM Enabled:%s\n", site->enableSQMread ? "true" : "false");
        snprintf(temp, sizeof(temp),"Status:SQM Healthy:%s\n", site->sqmHealthy ? "true" : "false");
        strcat(response, temp);
    } else {
        snprintf(response, response_size, "Status:Site Initialized:false\n");
    }
    if (dev) {
        snprintf(temp, sizeof(temp),"Status:SQM Reading Ready:%s\n", dev->reading_ready ? "true" : "false");
        strcat(response, temp);
    } else {
        snprintf(temp, sizeof(temp), "Status:SQM Device Initialized:false\n");
    }
    if (weatherData) {
        snprintf(temp, sizeof(temp), "Status:Weather Enabled:%s\n", site->enableWeather ? "true" : "false");
        strcat(response, temp);
        snprintf(temp, sizeof(temp),"Status:Weather Ready:%s\n", weatherData->weatherReady ? "true" : "false");
        strcat(response, temp);
    } else {
        snprintf(temp, sizeof(temp), "Status:Weather Initialized:false\n");
        strcat(response, temp);
    }
}

// Command: show
void command_show(char *words[], int nwords, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev, AW_WeatherData *weatherData) {
    (void)words; (void)nwords; (void)site; (void)dev; (void)weatherData;
//  snprintf(response, response_size, "Show: Not implemented");
    if(site && dev) {
        if (strcmp(words[1], "reading") == 0) {
          if (dev->reading_ready) {
            snprintf(response, response_size, "Reading: %s\n", dev->last_reading); 
          } else {
            snprintf(response, response_size, "Reading: Not ready\n");
          }
        }
        if (strcmp(words[1], "serial") == 0) {
          snprintf(response, response_size, "Serial: %d\n", dev->sqmSerial); 
        }
        if (strcmp(words[1], "model") == 0) {
          snprintf(response, response_size, "Model: %d\n", dev->sqmModel); 
        }
        if (strcmp(words[1], "weather") == 0) {
          if (weatherData->weatherReady) {            
            snprintf(response, response_size, "Weather:%f,%f,%f\n",
                 weatherData->temperature_f, 
                 weatherData->pressure_in, 
                 weatherData->humidity);
          } else {
            snprintf(response, response_size, "Weather:Not ready\n");
          }
        }

    }
}

// Command: set
void command_set(char *words[], int nwords, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    (void)words; (void)nwords; (void)site; (void)dev;
    snprintf(response, response_size, "Set: Not implemented");
}

// Command: start
void command_start(char *words[], int nwords, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    (void)words; (void)nwords; (void)site; (void)dev;
    if (site->enableSQMread == false) {
        site->enableSQMread = true;
    }
    snprintf(response, response_size, "Start: SQM read enabled\n");
}

// Command: stop
void command_stop(char *words[], int nwords, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    (void)words; (void)nwords; (void)site; (void)dev;
    if (site->enableSQMread == true) {
        site->enableSQMread = false;
    }
    snprintf(response, response_size, "Stop: SQM read disabled\n");
}

// Command: db
void command_db(char *words[], int nwords, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    (void)words; (void)nwords; (void)site; (void)dev;
    snprintf(response, response_size, "DB: Not implemented");
}

// Command: quit
void command_quit(char *words[], int nwords, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    (void)words; (void)nwords; (void)site; (void)dev;
    snprintf(response, response_size, "Quit: Exiting NightWatcher gracefully\n");
    exit(0);
}

// command: dt - Data Transmit - transmit all data to the client
void command_dt(char *words[], int nwords, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev, AW_WeatherData *weatherData) {
    (void)words; (void)nwords; (void)site; (void)dev; (void)weatherData;
    int i;
    memcpy(response, &site, sizeof(GlobalConfig));
    memcpy(response + sizeof(GlobalConfig), &dev, sizeof(SQM_LE_Device));
    memcpy(response + sizeof(GlobalConfig) + sizeof(SQM_LE_Device), &weatherData, sizeof(AW_WeatherData));
}
/*
 * Handles a command string received over TCP and writes a response to the response buffer.
 * Uses parse_fields from parser.c to split the command into words, trims each word, and dispatches.
 */
void handle_command(const char *cmd, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev, AW_WeatherData *weatherData) {
    char word_bufs[8][64];
    char *words[8] = { word_bufs[0], word_bufs[1], word_bufs[2], word_bufs[3], word_bufs[4], word_bufs[5], word_bufs[6], word_bufs[7] };
    int nwords = parse_fields(cmd, ' ', words, 8, 64);
    for (int i = 0; i < nwords; ++i) {
        trim_whitespace(words[i]);
    }
    if (nwords == 0 || strlen(words[0]) == 0) {
        snprintf(response, response_size, "No command received");
        return;
    }
    // Normalize first word to lowercase
    for (char *p = words[0]; *p; ++p) *p = tolower((unsigned char)*p);
    printf("[DEBUG] Command word: '%s'\n", words[0]);
    if (strcmp(words[0], "status") == 0) {
        command_status(words, nwords, response, response_size, site, dev, weatherData);
    } else if (strcmp(words[0], "show") == 0) {
        command_show(words, nwords, response, response_size, site, dev, weatherData);
    } else if (strcmp(words[0], "set") == 0) {
        command_set(words, nwords, response, response_size, site, dev);
    } else if (strcmp(words[0], "start") == 0) {
        command_start(words, nwords, response, response_size, site, dev);
    } else if (strcmp(words[0], "stop") == 0) {
        command_stop(words, nwords, response, response_size, site, dev);
    } else if (strcmp(words[0], "db") == 0) {
        command_db(words, nwords, response, response_size, site, dev);
    } else if (strcmp(words[0], "quit") == 0) {
        command_quit(words, nwords, response, response_size, site, dev);
    } else if (strcmp(words[0], "dt") == 0) {
        command_dt(words, nwords, response, response_size, site, dev, weatherData);
    } else {
        snprintf(response, response_size, "Unknown command: %s", words[0]);
    }
}
