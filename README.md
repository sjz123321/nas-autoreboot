# NAS AutoReboot

[中文文档](./README_zh.md)

An ESP8266-based automatic/remote reboot tool for home NAS systems.

## Features

- Serial port timeout monitoring and auto-reboot
- Remote reboot capability via HTTP requests
- Easy configuration and deployment

## Hardware Requirements

- ESP8266 core board
- USB to Serial converter (WCH CH340, driver-free on Debian)
- Reset pin connected to GPIO5

## System Architecture

[![flow chart](./pic/chart.png "flow chart")](./pic/chart.png "flow chart")

## Components

### 1. ESP8266 Arduino Program (`wificlient.ino`)

Required configuration:
- Replace all `<??>` tags with your settings:
  - `host`
  - `host_port`
  - `wifi_ssid`
  - `wifi_password`

#### Serial Monitoring Mode
- Baud rate: 9600
- Timeout threshold: 300 seconds
- Sends "monitor mode" text every second
- Waits for heartbeat response
- Auto-reboot on timeout

#### Remote Reboot Mode
- Polls `http://{host}:{host_port}/read` every 400 seconds
- Triggers reboot when response is `reset=1`
- Automatically resets flag via `/clear_reset` after reboot

### 2. Serial Port Monitor (`serial_test.c`)

- Monitors `/dev/ttyUSB0` by default
- Responds with "OK!" when receiving "monitor mode"

### 3. Remote Reboot Server (`remote_reboot.c`)

Default port: 8765

Endpoints:
- `/read` - Get reset flag status
- `/clear_reset` - Clear reset flag (set to 0)
- `/set_reset` - Set reset flag (set to 1)

## Hardware Schematic

[![sch_pic](./pic/sch.png "sch_pic")](./pic/sch.png "sch_pic")

## File List

1. `wificlient.ino` - ESP8266 Arduino program
2. `serial_test.c` - Serial port monitoring service
3. `remote_reboot.c` - Remote reboot HTTP server
