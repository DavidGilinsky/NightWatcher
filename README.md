# NightWatcher

NightWatcher is a modular C-based system for automated sky quality monitoring, site configuration management, and data logging/analysis. It is designed for observatories and research sites using the Unihedron SQM-LE sky quality meter, with extensible support for site environmental data, remote control, robust health monitoring, signal handling, and a TCP command interface.

## Console Interface (`nwconsole`)

The `nwconsole` subproject provides a curses-based console interface to NightWatcher. It connects to the NightWatcher process via the TCP command interface and displays:

- The latest SQM mpsqa reading and weather data (temperature, pressure, humidity)
- Site name and location
- Update intervals for SQM and weather readings

### nwconsole Configuration

`nwconsole` now uses its own configuration file: `nwconsole/conf/nwconsole.conf`. This file uses a simple key:value format:

```
ip:127.0.0.1
port:9000
```

The console client reads the IP address and port from this file at startup, allowing flexible deployment and connection to remote NightWatcher instances.

### Building and Running nwconsole

```
cd nwconsole
mkdir build
cd build
cmake ..
make
./nwconsole
```

## Features

- Communicate with Unihedron SQM-LE devices over TCP/IP
- Parse and process device readings, including calibration and environmental data
- Retrieve current personal weather station data from AmbientWeather API (robust to missing fields, uses 999.99 for missing values)
- Flexible configuration file management (key:value format)
- Modular codebase: device communication, configuration, parsing, database, command handling, and weather integration
- RRDTool-based time-series database for efficient storage and retrieval
- Example configuration and parser utilities
- Support for remote control via a configurable TCP control port
- Threaded reading with timeout and health monitoring
- Weather and SQM readings are each handled in their own threads
- Health status (`site.sqmHealthy`) is checked after unit information retrieval; readings are only taken if the device is healthy
- Signal handling for SIGHUP (reload/reinitialize) and SIGTERM (graceful shutdown)
- Configurable options for enabling/disabling SQM reading and reading on startup
- Main loop periodically checks device health (`site.sqmHeartbeatInterval`) and launches reading threads (`site.readingInterval`); TCP command listener runs in a separate thread and does not block the main loop
- TCP command parser robustly handles whitespace and case, and dispatches to command functions (`status`, `show`, `set`, `start`, `stop`, `quit`, `dt`)
- Extensible for additional sensors and site data

## TCP Command Interface

- Listens for TCP connections on `site.controlPort`
- Supported commands:
  - `status`: Returns overall system status (enabled, healthy, ready flags)
  - `show reading`: Returns the latest SQM reading (mpsqa, temperature, pressure, humidity)
  - `show weather`: Returns the latest weather data (temperature, pressure, humidity)
  - `dt`: Returns all site, device, and weather data as a comma-separated string (for efficient bulk data retrieval and use by clients like nwconsole)
  - `set`, `start`, `stop`, `quit`: Control commands

## Directory Structure

- `nwconsole/` — Curses-based console client for NightWatcher (configurable via `nwconsole/conf/nwconsole.conf`)
- `sqm-le/` — C library for SQM-LE device communication
- `parser/` — Generic string parsing utilities
- `config_file_handler/` — Library for reading/writing/deleting config files
- `db_handler/` — Library for RRDTool-based database management
- `command_handler/` — Library for TCP command parsing and dispatch
- `weather/AmbientWeather/` — C library for retrieving AmbientWeather personal weather station data (uses libcurl and libcjson)
- `send_data/GilinskyResearch/` — C client for sending data to a WordPress REST API endpoint
- `WordPress_Plugin/` — WordPress plugin providing a REST API endpoint and block for NightWatcher data
- `conf/` — Example configuration files for the main NightWatcher daemon
- `main.c` — Main program with threaded reading, health monitoring, signal handling, main loop, and TCP listener thread
- `CMakeLists.txt` — CMake build configuration file
- `main.h` — Header for main program
- `nightwatcher` — Compiled binary (created after build)
- `nightwatcher_db` — Default RRDTool database file (created at runtime)

## Configuration

### Main Daemon

Configuration is managed via a key:value file (see `conf/nwconf.conf`). Example fields:

```
siteName:Cresta Loma
latitude:32.30316
longitude:110.98590
elevation:719.33
sqmModel:200
sqmSerial:123456
sqmIP:192.168.7.4
sqmPort:10001
dbName:nightwatcher_db
readingInterval:30
controlPort:9000
sqmHeartbeatInterval:10
sqmReadTimeout:10
sqmWriteTimeout:10
enableReadOnStartup:true
AmbientWeatherAPIKey:...
AmbientWeatherAppKey:...
AmbientWeatherUpdateInterval:60
AmbientWeatherDeviceMAC:...
enableWeather:true
enableDataSend:false
```

- `enableDataSend`: Set to `true` to enable sending data to a remote WordPress REST API endpoint (see below).

### nwconsole

Configuration is managed via `nwconsole/conf/nwconsole.conf`:

```
ip:127.0.0.1
port:9000
```

## Data Sending and WordPress Integration

NightWatcher can send data to a remote WordPress REST API endpoint for integration with web dashboards or other systems. This is controlled by the `enableDataSend` option in the main configuration file. The client implementation is in `send_data/GilinskyResearch/nightwatcher_client.c`, and credentials/endpoint are configured in `send_data/GilinskyResearch/gilinskyresearch.conf`:

```
url:https://example.com/wp-json/nightwatcher/v1/submit
username:apiuser
password:apipass
```

### WordPress Plugin

The `WordPress_Plugin/` directory contains a plugin that provides:
- A REST API endpoint (`/wp-json/nightwatcher/v1/submit`) for receiving data from NightWatcher
- A setup page for configuring the database and user
- A simple block for displaying NightWatcher data in the WordPress editor

The plugin verifies database/user privileges, creates a table, and authenticates incoming data via HTTP Basic Auth.

## Build Instructions

The recommended way to build the project is with CMake:

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

To build the console client:

```
cd nwconsole
mkdir build
cd build
cmake ..
make
./nwconsole
```

## License

MIT License (or specify your license here)
