#ifndef NWCONSOLE_H
#define NWCONSOLE_H

#include <ncurses.h>

#define SERVER_IP   "127.0.0.1"  // Change as needed
#define SERVER_PORT 9000         // Change as needed
#define BUF_SIZE    1024

typedef struct {
    float mpsqa;
    float temperature;
    float pressure;
    float humidity;
    char site_name[128];
    char site_location[128];
    int sqm_interval;
    int weather_interval;
    int enable_data_send; // 1 if enabled, 0 if not
} NWData;

int connect_to_nightwatcher(const char *ip, int port);
int fetch_nw_data(int sock, NWData *data);
void draw_ui(WINDOW *win1, WINDOW *win2, WINDOW *win3, NWData *data);

#endif // NWCONSOLE_H
