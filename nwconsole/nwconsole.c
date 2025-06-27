/*
 * Project: NightWatcher
 * File: nwconsole.c
 * Author: David Gilinsky - gilinsky@gilinskyresearch.com
 * Date: 5 June 2025
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "nwconsole.h"


int connect_to_nightwatcher(const char *ip, int port) {
    int sock;
    struct sockaddr_in server;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server.sin_addr) <= 0) {
        close(sock);
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

// Read IP and port from nwconsole.conf
int read_nwconsole_conf(char *ip, size_t ip_len, int *port) {
    FILE *f = fopen("conf/nwconsole.conf", "r");
    if (!f) return -1;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *key = strtok(line, ":\n");
        char *val = strtok(NULL, "\n");
        if (!key || !val) continue;
        while (*val == ' ' || *val == '\t') val++;
        if (strcmp(key, "ip") == 0) {
            strncpy(ip, val, ip_len-1);
            ip[ip_len-1] = '\0';
        } else if (strcmp(key, "port") == 0) {
            *port = atoi(val);
        }
    }
    fclose(f);
    return 0;
}

// Fetch data from NightWatcher
int fetch_nw_data(int sock, NWData *data) {
    char sendbuf[128], recvbuf[1024];
    ssize_t n;
    memset(data, 0, sizeof(NWData));

    // Use the new dt command to get all data
    snprintf(sendbuf, sizeof(sendbuf), "dt\n");
    if (write(sock, sendbuf, strlen(sendbuf)) < 0) return -1;
    n = read(sock, recvbuf, sizeof(recvbuf) - 1);
    if (n <= 0) return -1;
    recvbuf[n] = '\0';
    // Parse the comma-separated response from dt
    // Example: siteName,latitude,longitude,elevation,sqmModel,sqmSerial,sqmIP,sqmPort,dbName,readingInterval,controlPort,sqmHealthy,...,mpsqa,...,temperature_f,pressure_in,humidity,...
    // For demo, extract mpsqa, temperature, pressure, humidity, site_name, intervals
    char *fields[64];
    int i = 0;
    char *token = strtok(recvbuf, ",");
    while (token && i < 64) {
        fields[i++] = token;
        token = strtok(NULL, ",");
    }
    // Defensive: check field count
    if (i > 0) {
        // mpsqa is field 28 (0-based index)
        data->mpsqa = (i > 28) ? atof(fields[27]) : 0;
        // temperature is field 34 (AW_WeatherData.temperature_f)
        data->temperature = (i > 44) ? atof(fields[45]) : 0;
        // pressure is field 38 (AW_WeatherData.pressure_in)
        data->pressure = (i > 48) ? atof(fields[49]) : 0;
        // humidity is field 35 (AW_WeatherData.humidity)
        data->humidity = (i > 45) ? atof(fields[46]) : 0;
        // site_name is field 0
        strncpy(data->site_name, fields[0], sizeof(data->site_name)-1);
        data->site_name[sizeof(data->site_name)-1] = '\0';
        // site_location: combine latitude,longitude (fields 1,2)
        snprintf(data->site_location, sizeof(data->site_location), "%s,%s", (i>1)?fields[1]:"", (i>2)?fields[2]:"");
        // sqm_interval is field 9
        data->sqm_interval = (i > 9) ? atoi(fields[9]) : -1;
        // weather_interval is field 16
        data->weather_interval = (i > 16) ? atoi(fields[16]) : -1;
        // enable_data_send is after enableWeather (field 20, so index 21)
        data->enable_data_send = (i > 21) ? atoi(fields[20]) : 0;
    } else {
        data->mpsqa = 0;
        data->temperature = 999;
        data->pressure = 0;
        data->humidity = 0;
        strcpy(data->site_name, "N/A");
        strcpy(data->site_location, "N/A");
        data->sqm_interval = -1;
        data->weather_interval = -1;
        data->enable_data_send = 0;
    }
    return 0;
}

void draw_ui(WINDOW *win1, WINDOW *win2, WINDOW *win3, NWData *data) {
    werase(win1);
    werase(win2);
    werase(win3);
    box(win1, 0, 0);
    box(win2, 0, 0);
    box(win3, 0, 0);
    // Section 1: SQM and Weather
    mvwprintw(win1, 1, 2, "mpsqa: %.2f", data->mpsqa);
    mvwprintw(win1, 2, 2, "Temp: %.1f F", data->temperature);
    mvwprintw(win1, 3, 2, "Pressure: %.1f in", data->pressure);
    mvwprintw(win1, 4, 2, "Humidity: %.1f%%", data->humidity);
    // Section 2: Site info
    mvwprintw(win2, 1, 2, "Site: %s", data->site_name);
    mvwprintw(win2, 2, 2, "Location: %s", data->site_location);
    // Section 3: Update intervals
    mvwprintw(win3, 1, 2, "SQM update interval: %d s", data->sqm_interval);
    mvwprintw(win3, 2, 2, "Weather update interval: %d s", data->weather_interval);
    mvwprintw(win3, 3, 2, "Data Send Enabled: %s", data->enable_data_send ? "Yes" : "No");
    wrefresh(win1);
    wrefresh(win2);
    wrefresh(win3);
}

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    int height = LINES / 3;
    int width = COLS;
    WINDOW *win1 = newwin(height, width, 0, 0);
    WINDOW *win2 = newwin(height, width, height, 0);
    WINDOW *win3 = newwin(LINES - 2 * height, width, 2 * height, 0);

    char ip[64] = "127.0.0.1";
    int port = 9000;
    read_nwconsole_conf(ip, sizeof(ip), &port);

    NWData data;
    while (1) {
        int sock = connect_to_nightwatcher(ip, port);
        if (sock < 0) {
            endwin();
            fprintf(stderr, "Failed to connect to NightWatcher TCP interface.\n");
            return 1;
        }
        fetch_nw_data(sock, &data);
        close(sock);
//        printf("Temperature: %f\n", data.temperature);
        draw_ui(win1, win2, win3, &data);
        timeout(1000); // 1 second
        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;
    }
    delwin(win1);
    delwin(win2);
    delwin(win3);
    endwin();
    return 0;
}
