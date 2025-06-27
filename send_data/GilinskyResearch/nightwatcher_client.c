
#include "../../nightwatcher.h"
#include "nightwatcher_client.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <stdbool.h>



static void trim(char *str) {
    char *end;
    while (*str == ' ' || *str == '\t') str++;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) *end-- = '\0';
}

// Reads url, username, password from gilinskyresearch.conf (key:value format)
bool nightwatcher_load_api_config(const char *conf_path, struct nightwatcher_api_config *cfg) {
    if (!conf_path || !cfg) return false;
    FILE *f = fopen(conf_path, "r");
    if (!f) return false;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *sep = strchr(line, ':');
        if (!sep) continue;
        *sep = '\0';
        char *key = line;
        char *val = sep + 1;
        trim(key); trim(val);
        if (strcmp(key, "url") == 0) strncpy(cfg->url, val, sizeof(cfg->url)-1);
        else if (strcmp(key, "username") == 0) strncpy(cfg->username, val, sizeof(cfg->username)-1);
        else if (strcmp(key, "password") == 0) strncpy(cfg->password, val, sizeof(cfg->password)-1);
    }
    fclose(f);
    return (cfg->url[0] && cfg->username[0] && cfg->password[0]);
}

struct response_accumulator {
    char *buf;
    size_t max;
    size_t len;
};

static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    struct response_accumulator *acc = (struct response_accumulator *)userdata;
    size_t total = size * nmemb;
    if (acc->buf && acc->len + total < acc->max) {
        memcpy(acc->buf + acc->len, ptr, total);
        acc->len += total;
        acc->buf[acc->len] = '\0';
    }
    return total;
}

bool nightwatcher_send_data(const char *url, const char *username, const char *app_password,
                            const GlobalConfig *site, const SQM_LE_Device *dev, const AW_WeatherData *weather_data,
                            char *response_buf, size_t response_buf_size) {
    if (!url || !username || !app_password || !site || !dev || !weather_data) return false;
    bool result = false;
    CURL *curl = curl_easy_init();
    if (!curl) return false;

    // Compose JSON payload
    char json[1024];
    // Use site, dev, and weather_data to fill the fields
    // For datetime, use weather_data->datetime if available, else dev->datetime, else ""
//    const char *datetime = weather_data->timestamp[0] ? weather_data->timestamp : (dev->datetime[0] ? dev->datetime : "");
    const char *datetime = dev->last_reading_timestamp[0]  ? dev->last_reading_timestamp : "";
    snprintf(json, sizeof(json),
        "{\"datetime\":\"%s\",\"site_name\":\"%s\",\"latitude\":%.8f,\"longitude\":%.8f,\"mpsqa\":%.4f,\"temperature\":%.2f,\"pressure\":%.2f,\"humidity\":%.2f}",
        datetime,
        site->siteName,
        site->latitude,
        site->longitude,
        dev->mpsqa,
        weather_data->temperature_f,
        weather_data->pressure_in,
        weather_data->humidity);

    // Prepare HTTP Basic Auth string
    char userpass[256];
    snprintf(userpass, sizeof(userpass), "%s:%s", username, app_password);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    struct response_accumulator acc = {0};
    if (response_buf && response_buf_size > 0) {
        acc.buf = response_buf;
        acc.max = response_buf_size;
        acc.len = 0;
        acc.buf[0] = '\0';
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpass);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &acc);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code >= 200 && http_code < 300) {
            result = true;
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return result;
}

// Example usage:
// struct nightwatcher_api_config cfg = {0};
// if (nightwatcher_load_api_config("gilinskyresearch.conf", &cfg)) {
//     nightwatcher_send_data(cfg.url, cfg.username, cfg.password, &data, response, sizeof(response));
// }
