# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Battery-powered ESP32 soil moisture sensor that publishes readings to MQTT (Home Assistant). The device uses deep sleep to conserve power, waking every 5 hours to take readings and publish to an MQTT broker.

## Build System

This is a PlatformIO project for ESP32 Arduino framework.

**IMPORTANT**: This project MUST be built using the Makefile build system. Do NOT use `pio run` commands directly.

### Build Commands

```bash
# Build the project (compiles for all configured boards)
make build

# Clean build artifacts
make clean

# View all available make targets
make help
```

The Makefile wraps PlatformIO and handles environment setup, dependency management, and build configuration. Direct use of `pio` commands may fail due to Python version requirements or other environment issues.

### Build Environments

Two board configurations in `platformio.ini`:
- `sparkfun_esp32c6_thing_plus`: C6 board with LED on GPIO23, peripheral power on GPIO15 (inverted logic)
- `sparkfun_esp32s3_thing_plus`: S3 board with LED on GPIO46, peripheral power on GPIO45 (normal logic)

Board-specific pins are configured via build flags in `platformio.ini`.

### Other Development Commands

For tasks beyond building (uploading, monitoring), refer to the Makefile or use PlatformIO commands at your own risk:

```bash
# Upload firmware (after building with make)
pio run -t upload -e sparkfun_esp32c6_thing_plus

# Monitor serial output
pio device monitor
```

### Build Version Injection

The build system automatically injects version information at compile time via `build_version.py`:

**Software Version (`BUILD_SW_VERSION`)**:
- Date-based format: `YYYY.MM.DD` (e.g., `2026.01.04`)
- Generated from UTC timestamp at build time
- Reflects when the firmware was compiled, not when code was written
- Updates automatically on each build day

**Hardware Version (`BUILD_HW_VERSION`)**:
- Board identifier: `esp32c6` or `esp32s3`
- Detected from PlatformIO environment name
- Helps Home Assistant distinguish between different board types

**How it Works**:
1. `build_version.py` runs before compilation (PlatformIO `pre:` script)
2. Injects `BUILD_SW_VERSION` and `BUILD_HW_VERSION` as preprocessor defines
3. Used in `lib/PubSubConn/pub_sub_conn.cpp` to populate MQTT device discovery JSON
4. Visible in Home Assistant device information

**Fallback Behavior**: If the build script fails to run, fallback values of `"unknown"` are used with compiler warnings.

**Version Display**: These versions appear in the MQTT discovery device JSON sent to Home Assistant, allowing you to track firmware versions and hardware types for each sensor.

## Architecture

### Core Flow (Deep Sleep Cycle)

1. **setup()** - Runs once on wake from deep sleep
   - Enable peripheral power (board-specific logic in `peripheral_power_on()`)
   - Start battery monitor (I2C MAX17048 fuel gauge)
   - Connect to WiFi with exponential backoff
   - Setup MQTT connection (`setup_pubsub()`)

2. **loop()** - Runs once (device enters deep sleep at end)
   - Read battery status, abort to deep sleep if invalid/low voltage
   - Connect to MQTT broker
   - Publish battery metrics (voltage, state of charge, change rate)
   - Read soil moisture sensor (25-sample average)
   - Publish soil readings (raw ADC + moisture percentage)
   - Enter deep sleep for 5 hours

3. **Deep sleep** - Power conservation
   - Gracefully disconnect MQTT and WiFi
   - Power down peripherals via `peripheral_power_off()`
   - Disable unused wake sources and power domains
   - Wake via timer after configured duration

### Library Modules

**StatusLed** (`lib/StatusLed/`)
- Visual feedback using board LED (colors via build flags)
- Status codes defined in `status.h` (wifi connecting, connected, errors, battery states)

**BatteryMonitor** (`lib/BatteryMonitor/`)
- I2C communication with MAX17048 fuel gauge chip
- Returns `battery_status` struct with voltage, SOC, charge rate, validity flags
- Implements retry logic for I2C initialization
- LED indication for battery states

**SoilSensor** (`lib/SoilSensor/`)
- Reads analog moisture sensor on GPIO4
- Calibration values in `soil_sensor_config.h` (dry/wet ADC thresholds)
- Returns raw ADC value + calculated moisture percentage (0-100%)
- Averages multiple samples to reduce noise

**PubSubConn** (`lib/PubSubConn/`)
- MQTT connection management using PubSubClient library
- Generates unique client ID from WiFi MAC address
- Home Assistant MQTT Discovery integration
- Publishes availability (LWT), sensor configs, and state messages
- Topic format: `node/sensor/{client_id}/{metric}` and `homeassistant/sensor/{client_id}/{metric}/config`

### Configuration Files

**Secrets** (git-ignored, see example files):
- `include/wifi_secrets.h`: WiFi SSID and password
- `include/mqtt_secrets.h`: MQTT broker IP, port, credentials

**Board Configuration**:
- `include/soil_sensor_config.h`: Sensor calibration values, peripheral power pin defaults
- Build flags in `platformio.ini` override pins per board

### Board-Specific Quirks

**Peripheral Power Control**: C6 uses inverted logic (LOW=on, HIGH=off) while S3 uses normal logic (HIGH=on, LOW=off). This is handled by checking `PERIPHERAL_POWER_PIN != 45` to detect board type.

**Serial Connection**: `ENABLE_SERIAL_CONNECTION` build flag controls whether to wait for serial monitor on startup (20s timeout). Set to 0 for production to avoid delays.

## MQTT Integration

Device publishes to Home Assistant-compatible MQTT topics with autodiscovery:

**Sensors published**:
- Soil moisture percentage (%)
- Soil raw ADC reading (diagnostic)
- Battery percentage (%)
- Battery voltage (V, diagnostic)
- Battery change rate (%/h, diagnostic)

**Availability**: LWT (Last Will and Testament) on `node/sensor/{client_id}/availability` with "online"/"offline" payloads.

## Power Management

- Deep sleep between readings (typically 5 hours, configurable in `main.cpp:280`)
- Peripheral power controlled via GPIO to cut sensor power during sleep
- WiFi/Bluetooth disabled before sleep
- GPIO pins set to INPUT_PULLDOWN to prevent floating during sleep
- Error conditions trigger extended sleep (10 hours for low battery, 20 minutes for MQTT failures)

## Development Notes

- Device waits for serial connection on startup if `ENABLE_SERIAL_CONNECTION=1` (see `main.cpp:56-71`)
- To enter flash mode: hold BOOT button, press RESET button
- Status LED provides visual feedback for all major states (see `lib/StatusLed/status.h` for codes)
- MQTT buffer size set to 1024 bytes to handle Home Assistant discovery JSON payloads
