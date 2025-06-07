/*
 * Project: NightWatcher
 * File: command_handler.h
 * Author: David Gilinsky - gilinsky@gilinskyresearch.com
 * Date: 5 June 2025
 */
#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <stddef.h>

// Handles a command string received over TCP and writes a response to the response buffer.
void handle_command(const char *cmd, char *response, size_t response_size, GlobalConfig *site, SQM_LE_Device *dev, AW_WeatherData *weatherData);

#endif // COMMAND_HANDLER_H
