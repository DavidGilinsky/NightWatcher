/*
 * Project: NightWatcher
 * File: sqm_le.c
 * Author: David Gilinsky - gilinsky@gilinskyresearch.com
 * Date: 5 June 2025
 */
#include "nightwatcher.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

/*
 * Establishes a TCP connection to the SQM-LE device using the IP and port in the dev struct.
 * Returns: 0 on success, negative value on error.
 */
static int sqm_le_connect(SQM_LE_Device *dev) {
    struct sockaddr_in addr;
    dev->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (dev->socket_fd < 0) return -1;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(dev->port);
    if (inet_pton(AF_INET, dev->ip, &addr.sin_addr) <= 0) return -2;
    if (connect(dev->socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return -3;
    return 0;
}

/*
 * Closes the TCP connection to the SQM-LE device.
 */
static void sqm_le_disconnect(SQM_LE_Device *dev) {
    if (dev->socket_fd >= 0) close(dev->socket_fd);
    dev->socket_fd = -1;
}

/*
 * Sends bytes to the SQM-LE device and receives a response.
 * Returns: 0 on success, negative value on error.
 */
static int sqm_le_sendrecv(SQM_LE_Device *dev, const void *sendbuf, size_t sendlen, void *recvbuf, size_t recvlen) {
    if (sqm_le_connect(dev) != 0) return -1;
    if (write(dev->socket_fd, sendbuf, sendlen) != (ssize_t)sendlen) {
        sqm_le_disconnect(dev);
        return -2;
    }
    if (read(dev->socket_fd, recvbuf, recvlen) != (ssize_t)recvlen) {
        sqm_le_disconnect(dev);
        return -3;
    }
    sqm_le_disconnect(dev);
    return 0;
}

/*
 * Gets a reading from the SQM-LE device, parses the response, and updates the dev struct.
 * Sets site->sqmHealthy to true if successful.
 * Returns: 0 on success, negative value on error.
 */
int getReading(SQM_LE_Device *dev, GlobalConfig *site) {
    if (site) site->sqmHealthy = false;
    if (dev) dev->reading_ready = false;
    uint8_t cmd[] = {'r', 'x'}; // Send ASCII "rx"
    uint8_t resp[56];
    int ret = sqm_le_sendrecv(dev, cmd, sizeof(cmd), resp, sizeof(resp));
    if (ret == 0) {
        // Copy up to 63 bytes and null-terminate
        size_t copy_len = (sizeof(dev->last_reading) - 1 < sizeof(resp)) ? sizeof(dev->last_reading) - 1 : sizeof(resp);
        memcpy(dev->last_reading, resp, copy_len);
        dev->last_reading[copy_len] = '\0';

        // Replace 0xd (carriage return) with null terminator
        char *cr = strchr(dev->last_reading, 0x0d);
        if (cr) *cr = '\0';

        // Parse fields
        char field_bufs[6][32];
        char *fields[6] = { field_bufs[0], field_bufs[1], field_bufs[2], field_bufs[3], field_bufs[4], field_bufs[5] };
        int nfields = parse_fields(dev->last_reading, ',', fields, 6, 32);
        if (nfields == 6) {
            // Disregard fields[0], start at fields[1]
            dev->mpsqa = strtof(fields[1], NULL);
            dev->sensorFreq = atoi(fields[2]);
            dev->sensorPeriodCount = atoi(fields[3]);
            dev->sensorPeriodSecs = strtof(fields[4], NULL);
            dev->sensorTemp = strtof(fields[5], NULL);
            if (site) site->sqmHealthy = true;
            if (dev) dev->reading_ready = true;
        }
    }
    return ret;
}

/*
 * Gets the serial number from the SQM-LE device and updates the dev struct.
 * Returns: 0 on success, negative value on error.
 */
int getReadingSerialNumber(SQM_LE_Device *dev) {
    uint8_t cmd[] = {0x02};
    char resp[32];
    int ret = sqm_le_sendrecv(dev, cmd, sizeof(cmd), resp, sizeof(resp));
    if (ret == 0) {
    //    strncpy(dev->serial_number, resp, sizeof(dev->serial_number));
    }
    return ret;
}

/*
 * Gets the calibration value from the SQM-LE device and updates the dev struct.
 * Returns: 0 on success, negative value on error.
 */
int getCalibration(SQM_LE_Device *dev) {
    uint8_t cmd[] = {0x03};
    float resp;
    int ret = sqm_le_sendrecv(dev, cmd, sizeof(cmd), &resp, sizeof(resp));
    if (ret == 0) {
        dev->calibration = resp;
    }
    return ret;
}

/*
 * Gets unit information from the SQM-LE device and updates the dev struct.
 * Returns: 0 on success, negative value on error.
 */
/*
 * Gets unit information from the SQM-LE device, parses the response, and updates the dev struct and site struct.
 * Sends 'ix', receives 38 bytes, parses into 5 fields, and populates site->sqmModel and site->sqmSerial.
 * Returns: 0 on success, negative value on error.
 */
int getUnitInformation(SQM_LE_Device *dev, GlobalConfig *site) {
    if (site) site->sqmHealthy = false;
    uint8_t cmd[] = {'i', 'x'};
    char resp[38];
    int ret = sqm_le_sendrecv(dev, cmd, sizeof(cmd), resp, sizeof(resp));
    if (ret == 0) {
        // Null-terminate the response
        size_t copy_len = sizeof(resp) < sizeof(dev->unit_info) ? sizeof(resp) : sizeof(dev->unit_info) - 1;
        memcpy(dev->unit_info, resp, copy_len);
        dev->unit_info[copy_len] = '\0';
        // Parse fields
        char field_bufs[5][16];
        char *fields[5] = { field_bufs[0], field_bufs[1], field_bufs[2], field_bufs[3], field_bufs[4] };
        int nfields = parse_fields(dev->unit_info, ',', fields, 5, 16);
        if (nfields == 5 && site) {
            site->sqmModel = atoi(fields[1]);
            dev->sqmModel = site->sqmModel;
            site->sqmSerial = atoi(fields[3]);
            dev->sqmSerial = site->sqmSerial;
            site->sqmHealthy = true;
            printf("sqmModel: %d\n", site->sqmModel);
            printf("sqmSerial: %d\n", site->sqmSerial);
            printf("Reading Ready: %s\n", dev->reading_ready ? "true" : "false");
        }
    }
    return ret;
}

/*
 * Arms the device for light calibration.
 * Returns: 0 on success, negative value on error.
 */
int armLightCalibration(SQM_LE_Device *dev) {
    uint8_t cmd[] = {0x05};
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Arms the device for dark calibration.
 * Returns: 0 on success, negative value on error.
 */
int armDarkCalibration(SQM_LE_Device *dev) {
    uint8_t cmd[] = {0x06};
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Sets the light calibration offset.
 * Returns: 0 on success, negative value on error.
 */
int setLightCalilbrationOffset(SQM_LE_Device *dev, float offset) {
    uint8_t cmd[5] = {0x07};
    memcpy(&cmd[1], &offset, sizeof(float));
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Sets the light calibration temperature.
 * Returns: 0 on success, negative value on error.
 */
int setLightCalibrationTemperature(SQM_LE_Device *dev, float temperature) {
    uint8_t cmd[5] = {0x08};
    memcpy(&cmd[1], &temperature, sizeof(float));
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Sets the dark calibration time period.
 * Returns: 0 on success, negative value on error.
 */
int setDarkCalibrationTimePeriod(SQM_LE_Device *dev, int period) {
    uint8_t cmd[5] = {0x09};
    memcpy(&cmd[1], &period, sizeof(int));
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Sets the dark calibration temperature.
 * Returns: 0 on success, negative value on error.
 */
int setDarkCalibrationTemperature(SQM_LE_Device *dev, float temperature) {
    uint8_t cmd[5] = {0x0A};
    memcpy(&cmd[1], &temperature, sizeof(float));
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Resets the SQM-LE device.
 * Returns: 0 on success, negative value on error.
 */
int reset(SQM_LE_Device *dev) {
    uint8_t cmd[] = {0x0B};
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Upgrades the SQM-LE device firmware.
 * Returns: 0 on success, negative value on error.
 */
int upgrade(SQM_LE_Device *dev) {
    uint8_t cmd[] = {0x0C};
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Sets the persistent period value.
 * Returns: 0 on success, negative value on error.
 */
int setPeriodPersistant(SQM_LE_Device *dev, int period) {
    uint8_t cmd[5] = {0x0D};
    memcpy(&cmd[1], &period, sizeof(int));
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Sets the immediate period value.
 * Returns: 0 on success, negative value on error.
 */
int setPeriodImmediate(SQM_LE_Device *dev, int period) {
    uint8_t cmd[5] = {0x0E};
    memcpy(&cmd[1], &period, sizeof(int));
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Sets the persistent threshold value.
 * Returns: 0 on success, negative value on error.
 */
int setThresholdPersistant(SQM_LE_Device *dev, float threshold) {
    uint8_t cmd[5] = {0x0F};
    memcpy(&cmd[1], &threshold, sizeof(float));
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Sets the immediate threshold value.
 * Returns: 0 on success, negative value on error.
 */
int setThresholdImmediate(SQM_LE_Device *dev, float threshold) {
    uint8_t cmd[5] = {0x10};
    memcpy(&cmd[1], &threshold, sizeof(float));
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Gets interval settings from the device (not implemented).
 * Returns: 0 on success, negative value on error.
 */
int getIntervalSettings(SQM_LE_Device *dev) {
    uint8_t cmd[] = {0x11};
    // Example: receive settings into dev fields (not implemented)
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Gets internal variables from the device (not implemented).
 * Returns: 0 on success, negative value on error.
 */
int getInternalVariables(SQM_LE_Device *dev) {
    uint8_t cmd[] = {0x12};
    // Example: receive variables into dev fields (not implemented)
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}

/*
 * Simulates internal calculations on the device (not implemented).
 * Returns: 0 on success, negative value on error.
 */
int simulateInternalCalculations(SQM_LE_Device *dev) {
    uint8_t cmd[] = {0x13};
    // Example: simulate calculations (not implemented)
    return sqm_le_sendrecv(dev, cmd, sizeof(cmd), NULL, 0);
}
