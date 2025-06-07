/*
 * Project: NightWatcher
 * File: AmbientWeather.c
 * Author: David Gilinsky - gilinsky@gilinskyresearch.com
 * Date: 5 June 2025
 */
#include "AmbientWeather.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "../../main.h"

static char g_api_key[128] = {0};
static char g_app_key[128] = {0};
static char g_device_mac[64] = {0};

// Buffer for HTTP response
typedef struct {
    char *data;
    size_t size;
} aw_http_buffer;

static size_t aw_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    aw_http_buffer *mem = (aw_http_buffer *)userp;
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if(ptr == NULL) return 0;
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    return realsize;
}

bool aw_init(const char* api_key, const char* application_key, const char* device_mac) {
    if (!api_key || !application_key || !device_mac) return false;
    strncpy(g_api_key, api_key, sizeof(g_api_key)-1);
    strncpy(g_app_key, application_key, sizeof(g_app_key)-1);
    strncpy(g_device_mac, device_mac, sizeof(g_device_mac)-1);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    return true;
}

bool aw_get_current_weather(AW_WeatherData* data) {
    if (!data) return false;
    CURL *curl = curl_easy_init();
    if (!curl) return false;
    char url[512];
    snprintf(url, sizeof(url),
        "https://api.ambientweather.net/v1/devices/%s?apiKey=%s&applicationKey=%s&limit=1",
        g_device_mac, g_api_key, g_app_key);
    printf("URL: %s\n", url);
    aw_http_buffer buffer = {0};
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, aw_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        free(buffer.data);
        return false;
    }
    // Parse JSON
    cJSON *root = cJSON_Parse(buffer.data);
    if (!root) {
        curl_easy_cleanup(curl);
        free(buffer.data);
        return false;
    }
    // The API returns an array of readings
    cJSON *reading = cJSON_GetArrayItem(root, 0);
    if (!reading) {
        cJSON_Delete(root);
        curl_easy_cleanup(curl);
        free(buffer.data);
        return false;
    }
    // Extract fields with NULL checks, use 999.99 if missing
    cJSON *item = NULL;
    item = cJSON_GetObjectItem(reading, "tempf");
    data->temperature_f = (item && cJSON_IsNumber(item)) ? item->valuedouble : 999.99;
    item = cJSON_GetObjectItem(reading, "humidity");
    data->humidity = (item && cJSON_IsNumber(item)) ? item->valuedouble : 999.99;
    item = cJSON_GetObjectItem(reading, "windspeedmph");
    data->wind_speed_mph = (item && cJSON_IsNumber(item)) ? item->valuedouble : 999.99;
    item = cJSON_GetObjectItem(reading, "windgustmph");
    data->wind_gust_mph = (item && cJSON_IsNumber(item)) ? item->valuedouble : 999.99;
    item = cJSON_GetObjectItem(reading, "baromabsin");
    data->pressure_in = (item && cJSON_IsNumber(item)) ? item->valuedouble : 999.99;
    item = cJSON_GetObjectItem(reading, "hourlyrainin");
    data->rainfall_in = (item && cJSON_IsNumber(item)) ? item->valuedouble : 999.99;
    item = cJSON_GetObjectItem(reading, "date");
    const char* ts = (item && cJSON_IsString(item)) ? item->valuestring : "";
    strncpy(data->timestamp, ts, sizeof(data->timestamp)-1);
    cJSON_Delete(root);
    curl_easy_cleanup(curl);
    free(buffer.data);
    return true;
}

void aw_cleanup() {
    curl_global_cleanup();
}
