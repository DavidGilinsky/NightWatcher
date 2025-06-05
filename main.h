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
} GlobalConfig;

int main(void);

#endif // MAIN_H
