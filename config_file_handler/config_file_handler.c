/*
 * Project: NightWatcher
 * File: config_file_handler.c
 * Author: David Gilinsky - gilinsky@gilinskyresearch.com
 * Date: 5 June 2025
 */
#include "config_file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Helper: trim leading/trailing whitespace
/*
 * Trims leading and trailing whitespace from a string in place.
 * Parameter: str - the string to trim.
 */
static void trim(char *str) {
    char *end;
    while (*str == ' ' || *str == '\t') str++;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) *end-- = '\0';
}

/*
 * Reads a configuration file in key:value format and populates a GlobalConfig struct.
 * Parameters: cfg - pointer to GlobalConfig struct to populate.
 *             filename - path to the config file.
 * Returns: 0 on success, nonzero on error (file not found or read error).
 */
int read_config(GlobalConfig *cfg, const char *filename) {
    struct stat st;
    if (stat(filename, &st) != 0) return -1; // File does not exist
    FILE *f = fopen(filename, "r");
    if (!f) return -2;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *sep = strchr(line, ':');
        if (!sep) continue;
        *sep = '\0';
        char *key = line;
        char *val = sep + 1;
        trim(key); trim(val);
        if (strcmp(key, "siteName") == 0) strncpy(cfg->siteName, val, sizeof(cfg->siteName)-1);
        else if (strcmp(key, "latitude") == 0) cfg->latitude = strtof(val, NULL);
        else if (strcmp(key, "longitude") == 0) cfg->longitude = strtof(val, NULL);
        else if (strcmp(key, "elevation") == 0) cfg->elevation = strtof(val, NULL);
        else if (strcmp(key, "sqmModel") == 0) cfg->sqmModel = atoi(val);
        else if (strcmp(key, "sqmSerial") == 0) cfg->sqmSerial = atoi(val);
        else if (strcmp(key, "sqmIP") == 0) strncpy(cfg->sqmIP, val, sizeof(cfg->sqmIP)-1);
        else if (strcmp(key, "sqmPort") == 0) cfg->sqmPort = (uint16_t)atoi(val);
        else if (strcmp(key, "dbName") == 0) strncpy(cfg->dbName, val, sizeof(cfg->dbName)-1);
        else if (strcmp(key, "readingInterval") == 0) cfg->readingInterval = (unsigned int)atoi(val);
        else if (strcmp(key, "controlPort") == 0) cfg->controlPort = (uint16_t)atoi(val);
        else if (strcmp(key, "sqmHeartbeatInterval") == 0) cfg->sqmHeartbeatInterval = (unsigned int)atoi(val);
        else if (strcmp(key, "sqmReadTimeout") == 0) cfg->sqmReadTimeout = (unsigned int)atoi(val);
        else if (strcmp(key, "sqmWriteTimeout") == 0) cfg->sqmWriteTimeout = (unsigned int)atoi(val);
    }
    fclose(f);
    return 0;
}

/*
 * Writes the values from a GlobalConfig struct to a configuration file in key:value format.
 * Parameters: cfg - pointer to GlobalConfig struct to write.
 *             filename - path to the config file.
 * Returns: 0 on success, nonzero on error (file cannot be created or written).
 */
int write_config(const GlobalConfig *cfg, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;
    fprintf(f, "siteName:%s\n", cfg->siteName);
    fprintf(f, "latitude:%f\n", cfg->latitude);
    fprintf(f, "longitude:%f\n", cfg->longitude);
    fprintf(f, "elevation:%f\n", cfg->elevation);
    fprintf(f, "sqmModel:%d\n", cfg->sqmModel);
    fprintf(f, "sqmSerial:%d\n", cfg->sqmSerial);
    fprintf(f, "sqmIP:%s\n", cfg->sqmIP);
    fprintf(f, "sqmPort:%u\n", cfg->sqmPort);
    fprintf(f, "dbName:%s\n", cfg->dbName);
    fprintf(f, "readingInterval:%u\n", cfg->readingInterval);
    fprintf(f, "controlPort:%u\n", cfg->controlPort);
    fprintf(f, "sqmHeartbeatInterval:%u\n", cfg->sqmHeartbeatInterval);
    fprintf(f, "sqmReadTimeout:%u\n", cfg->sqmReadTimeout);
    fprintf(f, "sqmWriteTimeout:%u\n", cfg->sqmWriteTimeout);
    fclose(f);
    return 0;
}

/*
 * Deletes the configuration file if it exists.
 * Parameters: cfg - pointer to GlobalConfig struct (unused).
 *             filename - path to the config file.
 * Returns: 0 on success (even if file does not exist), -1 on error.
 */
int delete_config(GlobalConfig *cfg, const char *filename) {
    struct stat st;
    if (stat(filename, &st) != 0) return 0; // File does not exist, still success
    if (unlink(filename) == 0) return 0;
    return -1;
}
