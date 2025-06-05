/*
 * Project: NightWatcher
 * File: command_handler.c
 * Author: David Gilinsky - gilinsky@gilinskyresearch.com
 * Date: 5 June 2025
 */
#include "command_handler.h"
#include <stdio.h>
#include <string.h>

/*
 * Handles a command string received over TCP and writes a response to the response buffer.
 * Parameters:
 *   cmd           - the command string
 *   response      - buffer to write the response
 *   response_size - size of the response buffer
 *   site          - pointer to GlobalConfig struct
 *   dev           - pointer to SQM_LE_Device struct
 */
void handle_command(const char *cmd, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev) {
    (void)site;
    (void)dev;
    // Example stub: echo the command back as the response
    snprintf(response, response_size, "Echo: %s", cmd);
    // Add command parsing and handling logic here
}
