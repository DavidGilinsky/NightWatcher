/*
 * Project: NightWatcher
 * File: AmbientWeather.h
 * Author: David Gilinsky - gilinsky@gilinskyresearch.com
 * Date: 5 June 2025
 */
#ifndef AMBIENTWEATHER_H
#define AMBIENTWEATHER_H

#include <stdbool.h>

// Structure to hold weather data
typedef struct {
    double temperature_f;
    double humidity;
    double wind_speed_mph;
    double wind_gust_mph;
    double pressure_in;
    double rainfall_in;
    char   timestamp[32];
    bool   weatherReady;
} AW_WeatherData;

// Initialize the AmbientWeather API client
// Returns true on success, false on failure
bool aw_init(const char* api_key, const char* application_key, const char* device_mac);

// Retrieve the latest weather data
// Returns true on success, false on failure
bool aw_get_current_weather(AW_WeatherData* data);

// Cleanup any resources used by the API client
void aw_cleanup();

#endif // AMBIENTWEATHER_H
