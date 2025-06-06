#ifndef SQM_LE_H
#define SQM_LE_H

#include <stdint.h>
#include <stddef.h>
#include "../main.h"

#define SQM_LE_IP_MAXLEN 64

// Structure to hold SQM-LE device state and connection info
typedef struct {
    char ip[SQM_LE_IP_MAXLEN]; // Device IP address
    uint16_t port;             // Device port
    int socket_fd;             // Socket file descriptor
    // Device-specific variables (add as needed)
    // char serial_number[32];
    int sqmModel;
    int sqmSerial;
    char last_reading[64];
    bool reading_ready;
    float calibration;
    char unit_info[64];
    // Add more fields as needed for calibration, settings, etc.

    float mpsqa;              // Magnitude per square arc second
    int sensorFreq;           // Frequency of sensor in Hz
    int sensorPeriodCount;    // Period of sensor in count. Counts occur at a rate of 460.8 kHz
    float sensorPeriodSecs;   // Period of sensor in seconds
    float sensorTemp;         // Temperature of the sensor in C
} SQM_LE_Device;

// Function declarations
int getReading(SQM_LE_Device *dev, GlobalConfig *site);
int getReadingSerialNumber(SQM_LE_Device *dev);
int getCalibration(SQM_LE_Device *dev);
int getUnitInformation(SQM_LE_Device *dev, GlobalConfig *site);
int armLightCalibration(SQM_LE_Device *dev);
int armDarkCalibration(SQM_LE_Device *dev);
int setLightCalilbrationOffset(SQM_LE_Device *dev, float offset);
int setLightCalibrationTemperature(SQM_LE_Device *dev, float temperature);
int setDarkCalibrationTimePeriod(SQM_LE_Device *dev, int period);
int setDarkCalibrationTemperature(SQM_LE_Device *dev, float temperature);
int reset(SQM_LE_Device *dev);
int upgrade(SQM_LE_Device *dev);
int setPeriodPersistant(SQM_LE_Device *dev, int period);
int setPeriodImmediate(SQM_LE_Device *dev, int period);
int setThresholdPersistant(SQM_LE_Device *dev, float threshold);
int setThresholdImmediate(SQM_LE_Device *dev, float threshold);
int getIntervalSettings(SQM_LE_Device *dev);
int getInternalVariables(SQM_LE_Device *dev);
int simulateInternalCalculations(SQM_LE_Device *dev);

#endif // SQM_LE_H
