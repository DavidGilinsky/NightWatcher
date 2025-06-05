# NightWatcher

NightWatcher is a modular C-based system for automated sky quality monitoring, site configuration management, and data logging/analysis. It is designed for observatories and research sites using the Unihedron SQM-LE sky quality meter, with extensible support for site environmental data, remote control, robust health monitoring, signal handling, and a TCP command interface.

## Features
- Communicate with Unihedron SQM-LE devices over TCP/IP
- Parse and process device readings, including calibration and environmental data
- Flexible configuration file management (key:value format)
- Modular codebase: device communication, configuration, parsing, database, and command handling
- RRDTool-based time-series database for efficient storage and retrieval
- Example configuration and parser utilities
- Support for remote control via a configurable TCP control port
- Threaded reading with timeout and health monitoring
- Health status (`site.sqmHealthy`) is checked after unit information retrieval; readings are only taken if the device is healthy
- Signal handling for SIGHUP (reload/reinitialize) and SIGTERM (graceful shutdown)
- Configurable options for enabling/disabling SQM reading and reading on startup
- Main loop periodically checks device health (site.sqmHeartbeatInterval), launches reading threads (site.readingInterval), and listens for TCP commands
- TCP command parser robustly handles whitespace and case, and dispatches to command functions (status, show, set, start, stop, quit)
- Extensible for additional sensors and site data

## Directory Structure
- `sqm-le/` — C library for SQM-LE device communication
- `parser/` — Generic string parsing utilities
- `config_file_handler/` — Library for reading/writing/deleting config files
- `db_handler/` — Library for RRDTool-based database management
- `command_handler/` — Library for TCP command parsing and dispatch
- `conf/` — Example configuration files
- `main.c` — Main program with threaded reading, health monitoring, signal handling, and main loop

## Configuration
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
readingInterval:300
controlPort:9000
sqmHeartbeatInterval:60
sqmReadTimeout:10
sqmWriteTimeout:10
enableReadOnStartup:true
```

- `readingInterval`: Number of seconds between SQM readings
- `controlPort`: TCP port on which to listen for remote commands
- `sqmHeartbeatInterval`: Heartbeat interval in seconds (for health checks)
- `sqmReadTimeout`: Timeout in seconds for SQM reading thread
- `sqmWriteTimeout`: Timeout in seconds for SQM write operations
- `enableReadOnStartup`: If true, perform a reading on startup
- `enableSQMread`: (code only) Enable periodic SQM reading (not in config file)

## Health Monitoring and Main Loop
- The program checks the health of the SQM-LE device using unit information retrieval at intervals of `site.sqmHeartbeatInterval`.
- The reading thread is launched at intervals of `site.readingInterval` if `site.sqmHealthy` and `site.enableSQMread` are true.
- If a reading or parsing operation fails, `site.sqmHealthy` is set to false.
- Health status is printed at the end of each run.
- The main loop also listens for TCP commands and dispatches them to the command handler.

## Command Handler
- Listens for TCP connections on `site.controlPort`
- Parses the first word of each command, ignoring whitespace and case
- Supported commands: `status`, `show`, `set`, `start`, `stop`, `quit`
- Each command function receives the command and a response buffer, and can interact with the site and device state

## Signal Handling
- SIGHUP: Triggers a reload or reinitialization (logic can be extended as needed)
- SIGTERM: Triggers a graceful shutdown

## Build Instructions

You can build the project using gcc. For example:

```
gcc -Wall -Wextra -g -o nightwatcher main.c sqm-le/sqm_le.c parser/parser.c config_file_handler/config_file_handler.c db_handler/db_handler.c command_handler/command_handler.c -lrrd -lpthread
```

## Dependencies
- Standard C library
- rrdtool (with development headers and library)
- pthreads (for threading)

## Example Usage
- Loads configuration from `conf/nwconf.conf`
- Connects to the SQM-LE device and retrieves readings in a separate thread (if healthy)
- Stores readings and site/environmental data in an RRDTool database
- Monitors health status and enforces timeouts on device communication
- Handles SIGHUP and SIGTERM for reloading and graceful shutdown
- Main loop ensures continuous health monitoring, periodic readings, and command processing
- Ready for extension to support remote control and additional sensors

## License
MIT License (or specify your license here)
