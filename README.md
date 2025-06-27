# NightWatcher

NightWatcher is a modular C-based system for automated sky quality monitoring, site configuration management, and data logging/analysis. It is designed for observatories and research sites using the Unihedron SQM-LE sky quality meter, with extensible support for site environmental data, remote control, robust health monitoring, signal handling, and a TCP command interface.

Public Repository: https://github.com/DavidGilinsky/NightWatcher

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


## Console Interface (`nwconsole`)

The `nwconsole` subproject provides a curses-based console interface to NightWatcher. It connects to the NightWatcher process via the TCP command interface and displays:

- The latest SQM mpsqa reading and weather data (temperature, pressure, humidity)
- Site name and location
- Update intervals for SQM and weather readings

### nwconsole Configuration

Configuration is managed via `nwconsole/conf/nwconsole.conf.example`:

```
# NightWatcher Console Configuration Example
# This file documents the configuration options for the NightWatcher console client.
# Fill in values appropriate for your deployment.

# IP address of the NightWatcher server
ip:SERVER_IP_ADDRESS

# Port number to connect to on the NightWatcher server
port:PORT_NUMBER
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

Configuration is managed via a key:value file (see `conf/nwconf.conf.example`). Example fields:

```
# NightWatcher Main Configuration Example
# This file documents all available configuration options for NightWatcher.
# Fill in values appropriate for your deployment. Do not include secrets in this example file.

# Name of the site or installation location
siteName:YOUR_SITE_NAME

# Latitude of the site (decimal degrees)
latitude:0.00000

# Longitude of the site (decimal degrees)
longitude:0.00000

# Elevation above sea level (meters)
elevation:0.0

# Model number of the SQM device
sqmModel:MODEL_NUMBER

# Serial number of the SQM device
sqmSerial:SERIAL_NUMBER

# IP address of the SQM device
sqmIP:DEVICE_IP_ADDRESS

# Port number for the SQM device
sqmPort:PORT_NUMBER

# Name of the database to use
dbName:DATABASE_NAME

# Interval (in seconds) between readings
readingInterval:SECONDS

# Port for control commands
controlPort:PORT_NUMBER

# Heartbeat interval for the SQM device (seconds)
sqmHeartbeatInterval:SECONDS

# Timeout for reading from the SQM device (seconds)
sqmReadTimeout:SECONDS

# Timeout for writing to the SQM device (seconds)
sqmWriteTimeout:SECONDS

# Whether to enable reading on startup (true/false)
enableReadOnStartup:true

# AmbientWeather API Key (leave blank in example)
AmbientWeatherAPIKey:

# AmbientWeather App Key (leave blank in example)
AmbientWeatherAppKey:

# Update interval for AmbientWeather (seconds)
AmbientWeatherUpdateInterval:SECONDS

# MAC address of the AmbientWeather device
AmbientWeatherDeviceMAC:XX:XX:XX:XX:XX:XX

# Whether to enable weather integration (true/false)
enableWeather:true

# Whether to enable data sending (true/false)
enableDataSend:false
```

- `enableDataSend`: Set to `true` to enable sending data to a remote WordPress REST API endpoint (see below).



## Data Sending and WordPress Integration

NightWatcher can send data to a remote WordPress REST API endpoint for integration with web dashboards or other systems. This is controlled by the `enableDataSend` option in the main configuration file. The client implementation is in `send_data/GilinskyResearch/nightwatcher_client.c`, and credentials/endpoint are configured in `send_data/GilinskyResearch/gilinskyresearch.conf.example`:

```
# GilinskyResearch Data Submission Configuration Example
# This file documents the configuration options for submitting data to GilinskyResearch.
# Do not include real credentials in this example file.

# URL endpoint for data submission
url:https://YOUR_SERVER_URL/wp-json/nightwatcher/v1/submit

# Username for authentication
username:YOUR_USERNAME

# Password or API key for authentication (leave blank in example)
password:
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
