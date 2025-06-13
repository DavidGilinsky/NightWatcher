#ifndef NIGHTWATCHER_CLIENT_H
#define NIGHTWATCHER_CLIENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nightwatcher_data {
    const char *datetime;      // ISO 8601 string, e.g. "2024-06-01 12:00:00"
    const char *site_name;
    double latitude;
    double longitude;
    double mpsqa;
    double temperature;
    double pressure;
    double humidity;
};

/**
 * Send data to the NightWatcher WordPress API endpoint.
 * @param url The full API endpoint URL (e.g., https://example.com/wp-json/nightwatcher/v1/submit)
 * @param username WordPress username
 * @param app_password WordPress Application Password
 * @param data Pointer to nightwatcher_data struct
 * @param response_buf Optional buffer to receive response (may be NULL)
 * @param response_buf_size Size of response buffer
 * @return true on success, false on failure
 */
bool nightwatcher_send_data(const char *url, const char *username, const char *app_password,
                            const struct nightwatcher_data *data,
                            char *response_buf, size_t response_buf_size);

#ifdef __cplusplus
}
#endif

#endif // NIGHTWATCHER_CLIENT_H
