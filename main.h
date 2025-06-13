#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdbool.h>

// Global site/device configuration structure
typedef struct {
    char siteName[256];
    float latitude;
    float longitude;
    float elevation;
    int sqmModel;
    int sqmSerial;
    char sqmIP[64];
    uint16_t sqmPort;
    char dbName[256];
    unsigned int readingInterval; // Number of seconds between sqm readings
    uint16_t controlPort; // TCP port on which to listen for commands
    unsigned int sqmHeartbeatInterval; // Heartbeat interval in seconds
    bool sqmHealthy; // True if SQM is healthy
    unsigned int sqmReadTimeout;  // SQM read timeout in seconds
    unsigned int sqmWriteTimeout; // SQM write timeout in seconds
    bool enableSQMread;           // Enable periodic SQM reading
    bool enableReadOnStartup;     // Enable reading on startup (configurable)
    char AmbientWeatherAPIKey[66]; // Ambient Weather API key
    char AmbientWeatherAppKey[66]; // Ambient Weather app key
    unsigned int AmbientWeatherUpdateInterval; // Ambient Weather update interval in seconds
    char AmbientWeatherDeviceMAC[17]; // Ambient Weather device MAC
    char AmbientWeatherEncodedMAC[28]; // Ambient Weather encoded MAC for URL construction
    bool enableWeather; // Enable Weather information retrieval
    bool enableDataSend; // Enable sending data by REST API to configured sites
} GlobalConfig;

int main(void);

#endif // MAIN_H
