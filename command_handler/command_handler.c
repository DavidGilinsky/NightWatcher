/*
 * Project: NightWatcher
 * File: command_handler.c
 * Author: David Gilinsky - gilinsky@gilinskyresearch.com
 * Date: 5 June 2025
 */
#include "command_handler.h"
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

// Helper: parse the first word of the command, skipping leading whitespace, and normalize to lowercase
static void get_first_word(const char *cmd, char *word, size_t word_size) {
    // Skip leading whitespace
    while (*cmd && isspace((unsigned char)*cmd)) cmd++;
    size_t i = 0;
    while (cmd[i] && !isspace((unsigned char)cmd[i]) && i < word_size - 1) {
        word[i] = tolower((unsigned char)cmd[i]);
        i++;
    }
    word[i] = '\0';
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
 * Dispatches to the appropriate command function based on the first word.
 */
void handle_command(const char *cmd, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    (void)site; (void)dev;
    char word[32];
    strncpy(word, cmd, sizeof(word)-1); word[sizeof(word)-1] = '\0';
    trim_whitespace(word);
    get_first_word(word, word, sizeof(word));
    printf("[DEBUG] Command word: '%s'\n", word);
    if (strcmp(word, "status") == 0) {
        command_status(cmd, response, response_size, site, dev);
    } else if (strcmp(word, "show") == 0) {
        command_show(cmd, response, response_size, site, dev);
    } else if (strcmp(word, "set") == 0) {
        command_set(cmd, response, response_size, site, dev);
    } else if (strcmp(word, "start") == 0) {
        command_start(cmd, response, response_size, site, dev);
    } else if (strcmp(word, "stop") == 0) {
        command_stop(cmd, response, response_size, site, dev);
    } else if (strcmp(word, "quit") == 0) {
        command_quit(cmd, response, response_size, site, dev);
    } else {
        snprintf(response, response_size, "Unknown command: %s", word);
    }
}
