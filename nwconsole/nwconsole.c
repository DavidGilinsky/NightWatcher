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

// Fetch data from NightWatcher
int fetch_nw_data(int sock, NWData *data) {
    char sendbuf[128], recvbuf[1024];
    ssize_t n;
    memset(data, 0, sizeof(NWData));

    // 1. Get SQM reading
    snprintf(sendbuf, sizeof(sendbuf), "show reading\n");
    if (write(sock, sendbuf, strlen(sendbuf)) < 0) return -1;
    n = read(sock, recvbuf, sizeof(recvbuf) - 1);
    if (n <= 0) return -1;
    recvbuf[n] = '\0';
    // Example response: "Reading: 21.5,15.2,1013.2,45.0\n" or similar
    // Parse for mpsqa, temp, pressure, humidity if present
    float mpsqa = 0, temp = 0, press = 0, humid = 0;
    if (sscanf(recvbuf, "Reading: %f,%f,%f,%f", &mpsqa, &temp, &press, &humid) == 4) {
        data->mpsqa = mpsqa;
        data->temperature = temp;
        data->pressure = press;
        data->humidity = humid;
    } else {
        data->mpsqa = 0;
        data->temperature = 0;
        data->pressure = 0;
        data->humidity = 0;
    }

    // 2. Site info and intervals (not available via TCP, set as N/A or dummy)
    strcpy(data->site_name, "N/A");
    strcpy(data->site_location, "N/A");
    data->sqm_interval = -1;
    data->weather_interval = -1;
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
    mvwprintw(win1, 1, 2, "SQM mpsqa: %.2f", data->mpsqa);
    mvwprintw(win1, 2, 2, "Temp: %.1f C", data->temperature);
    mvwprintw(win1, 3, 2, "Pressure: %.1f hPa", data->pressure);
    mvwprintw(win1, 4, 2, "Humidity: %.1f%%", data->humidity);
    // Section 2: Site info
    mvwprintw(win2, 1, 2, "Site: %s", data->site_name);
    mvwprintw(win2, 2, 2, "Location: %s", data->site_location);
    // Section 3: Update intervals
    mvwprintw(win3, 1, 2, "SQM update interval: %d s", data->sqm_interval);
    mvwprintw(win3, 2, 2, "Weather update interval: %d s", data->weather_interval);
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

    int sock = connect_to_nightwatcher(SERVER_IP, SERVER_PORT);
    if (sock < 0) {
        endwin();
        fprintf(stderr, "Failed to connect to NightWatcher TCP interface.\n");
        return 1;
    }

    NWData data;
    while (1) {
        fetch_nw_data(sock, &data);
        draw_ui(win1, win2, win3, &data);
        timeout(1000); // 1 second
        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;
    }
    close(sock);
    delwin(win1);
    delwin(win2);
    delwin(win3);
    endwin();
    return 0;
}
