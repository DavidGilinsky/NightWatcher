/*
 * Project: NightWatcher
 * File: command_handler.c
 * Author: David Gilinsky - gilinsky@gilinskyresearch.com
 * Date: 5 June 2025
 */
#include "command_handler.h"
#include "../parser/parser.h"
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
void command_status(const char *cmd, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    snprintf(response, response_size, "Status: SQM healthy = %s", site->sqmHealthy ? "true" : "false");
}

// Command: show
void command_show(const char *cmd, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    snprintf(response, response_size, "Show: Not implemented");
}

// Command: set
void command_set(const char *cmd, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    snprintf(response, response_size, "Set: Not implemented");
}

// Command: start
void command_start(const char *cmd, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    snprintf(response, response_size, "Start: Not implemented");
}

// Command: stop
void command_stop(const char *cmd, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    snprintf(response, response_size, "Stop: Not implemented");
}

// Command: quit
void command_quit(const char *cmd, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    snprintf(response, response_size, "Quit: Not implemented");
}

/*
 * Handles a command string received over TCP and writes a response to the response buffer.
 * Uses parse_fields from parser.c to split the command into words, trims each word, and dispatches.
 */
void handle_command(const char *cmd, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
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
        command_status(cmd, response, response_size, site, dev);
    } else if (strcmp(words[0], "show") == 0) {
        command_show(cmd, response, response_size, site, dev);
    } else if (strcmp(words[0], "set") == 0) {
        command_set(cmd, response, response_size, site, dev);
    } else if (strcmp(words[0], "start") == 0) {
        command_start(cmd, response, response_size, site, dev);
    } else if (strcmp(words[0], "stop") == 0) {
        command_stop(cmd, response, response_size, site, dev);
    } else if (strcmp(words[0], "quit") == 0) {
        command_quit(cmd, response, response_size, site, dev);
    } else {
        snprintf(response, response_size, "Unknown command: %s", words[0]);
    }
}
