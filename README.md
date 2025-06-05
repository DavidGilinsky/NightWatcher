# NightWatcher

NightWatcher is a modular C-based system for automated sky quality monitoring, site configuration management, and data logging/analysis. It is designed for observatories and research sites using the Unihedron SQM-LE sky quality meter, with extensible support for site environmental data and remote control.

## Features
- Communicate with Unihedron SQM-LE devices over TCP/IP
- Parse and process device readings, including calibration and environmental data
- Flexible configuration file management (key:value format)
- Modular codebase: device communication, configuration, parsing, and database handling
- RRDTool-based time-series database for efficient storage and retrieval
- Example configuration and parser utilities
- Support for remote control via a configurable TCP control port

## Directory Structure
- `sqm-le/` — C library for SQM-LE device communication
- `parser/` — Generic string parsing utilities
- `config_file_handler/` — Library for reading/writing/deleting config files
- `db_handler/` — Library for RRDTool-based database management
- `conf/` — Example configuration files
- `main.c` ��� Example main program

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
```

- `readingInterval`: Number of seconds between SQM readings
- `controlPort`: TCP port on which to listen for remote commands

## Build Instructions

You can build the project using gcc. For example:

```
gcc -Wall -Wextra -g -o nightwatcher main.c sqm-le/sqm_le.c parser/parser.c config_file_handler/config_file_handler.c db_handler/db_handler.c -lrrd
```

## Dependencies
- Standard C library
- rrdtool (with development headers and library)

## Example Usage
- Loads configuration from `conf/nwconf.conf`
- Connects to the SQM-LE device and retrieves readings
- Stores readings and site/environmental data in an RRDTool database
- Ready for extension to support remote control and additional sensors

## License
MIT License (or specify your license here)
