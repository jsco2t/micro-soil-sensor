```
   _____       _ __   _____
  / ___/____  (_) /  / ___/___  ____  _________  _____
  \__ \/ __ \/ / /   \__ \/ _ \/ __ \/ ___/ __ \/ ___/
 ___/ / /_/ / / /   ___/ /  __/ / / (__  ) /_/ / /
/____/\____/_/_/   /____/\___/_/ /_/____/\____/_/

```

# ESP32 Soil Moisture Sensor

Battery-powered ESP32 soil moisture sensor that publishes readings to MQTT for Home Assistant integration. The device uses deep sleep to conserve power, waking every 5 hours to take readings and publish to an MQTT broker.

---

## ⚠️ Important Disclaimers

**READ THIS BEFORE USING THIS PROJECT:**

### A) Zero Warranties - Hobby Project

This is a **personal hobby project** provided "as-is" with **absolutely no warranties** of any kind, express or implied. Use this code at your own risk. The author assumes no liability for any damage, injury, data loss, or other issues that may arise from using this project.

### B) Learning Project - Not a Tutorial

This project was created as a **learning exercise**. It likely contains mistakes, suboptimal design choices, and approaches that may not represent best practices. **Do not use this as a tutorial or definitive guide.** If you're learning, use this as a reference point to compare with other resources, not as the authoritative source.

### C) Hardware-Specific Implementation

This firmware is designed **specifically for the hardware the author used** (see Hardware Requirements below). It uses specific GPIO pins, boards, and components that match the authors setup. **It will not work universally on other hardware without modification.** You will need to adapt pin assignments, voltage levels, and component-specific code for your own hardware.

---

## Hardware Requirements

This project was built for the following **specific hardware**:

### Microcontroller Boards

- **SparkFun ESP32-C6 Thing Plus**
  - Status LED: GPIO23 (WS2812 RGB)
  - Peripheral Power: GPIO15 (inverted logic - LOW=on, HIGH=off)
  - Board LED: GPIO8
- **SparkFun ESP32-S3 Thing Plus**
  - Status LED: GPIO46 (WS2812 RGB)
  - Peripheral Power: GPIO45 (normal logic - HIGH=on, LOW=off)
  - Board LED: GPIO8

#### Common components in both boards

- **MAX17048 Fuel Gauge** (I2C battery monitor)
  - Connected via I2C (SDA/SCL)
  - Monitors LiPo battery voltage and state of charge
- MCP73831 battery charger

### Required Components

- **Analog Soil Moisture Sensor**
  - Connected to GPIO4 (ADC input)
  - VCC power via GPIO2 (controlled power rail)
  - Requires calibration (dry/wet ADC values)
- **LiPo Battery** (single cell, 3.7V nominal)
  - Connected to JST battery connector on board
  - Monitored by MAX17048 fuel gauge
- **WiFi Network** (2.4GHz for ESP32 compatibility)
- **MQTT Broker** (e.g., Mosquitto on Home Assistant)

### Pin Configuration Summary

| Function              | ESP32-C6 Pin | ESP32-S3 Pin |
|-----------------------|--------------|--------------|
| RGB Status LED        | GPIO23       | GPIO46       |
| Peripheral Power Ctrl | GPIO15 (inv) | GPIO45       |
| Board LED             | GPIO8        | GPIO8        |
| Soil Sensor ADC       | GPIO4        | GPIO4        |
| Soil Sensor VCC       | GPIO2        | GPIO2        |
| I2C SDA (Fuel Gauge)  | Board default| Board default|
| I2C SCL (Fuel Gauge)  | Board default| Board default|

**Note:** Peripheral power control uses **inverted logic on C6** (LOW=on) and **normal logic on S3** (HIGH=on). This is handled in firmware but important if modifying.

---

## Software Dependencies

### System Requirements

- **Python 3.10 - 3.13** (PlatformIO requirement)
  - Fedora/RHEL: `sudo dnf install python3.13`
  - Ubuntu/Debian: `sudo apt install python3.13`
  - macOS: `brew install python@3.13`
- **Make** (build system wrapper)
  - Usually pre-installed on Linux/macOS
  - Windows: Install via MinGW, Cygwin, or WSL
- **Git** (for cloning repository)

### PlatformIO Platform & Libraries

These are **automatically installed** by PlatformIO when you run `make setup` and `make build`:

**Platform:**

- `platform-espressif32` (Tasmota fork 2025.11.30)
  - Provides ESP32-C6 and ESP32-S3 support
  - Arduino framework for ESP32

**Libraries:**

- `PubSubClient` v2.8.0 (MQTT client)
- `SparkFun MAX1704x Fuel Gauge Arduino Library` (battery monitoring)

**Note:** You do **not** need to manually install these libraries. The build system handles all dependencies.

---

## Quick Start Guide

### 1. Clone the Repository

```bash
git clone <your-repo-url>
cd soil-sensor
```

### 2. Initialize Build Environment

This creates a Python virtual environment and installs PlatformIO:

```bash
make setup
```

**What this does:**

- Creates `.venv/` directory with isolated Python environment
- Installs PlatformIO and dependencies
- Verifies Python version compatibility (3.10-3.13)

### 3. Configure WiFi and MQTT Secrets

Copy the example secret files and edit with your credentials:

```bash
cp include/wifi_secrets_example.h include/wifi_secrets.h
cp include/mqtt_secrets_example.h include/mqtt_secrets.h
```

Edit `include/wifi_secrets.h`:

```c
const char *ssid = "YourWiFiSSID";
const char *password = "YourWiFiPassword";
```

Edit `include/mqtt_secrets.h`:

```c
const char *mqtt_server = "192.168.1.100";      // Your MQTT broker IP
const unsigned int mqtt_server_port = 1883;     // Default MQTT port
const char *mqtt_user = "your_mqtt_username";   // MQTT username
const char *mqtt_password = "your_mqtt_pass";   // MQTT password
```

**Security Note:** These files are `.gitignore`d to prevent accidentally committing credentials.

### 4. (Optional) Calibrate Soil Sensor

Edit `include/soil_sensor_config.h` to set calibration values:

```c
#define DRY_SENSOR_READING 3500   // ADC value when sensor is in dry air
#define WET_SENSOR_READING 1400   // ADC value when sensor is fully submerged
```

To find these values:

1. Upload firmware with default values
2. Monitor serial output (`make monitor`)
3. Note raw ADC value when sensor is dry (in air)
4. Note raw ADC value when sensor is wet (in water)
5. Update config and rebuild

### 5. Build Firmware

Build for both boards (or specific board):

```bash
make build        # Build for both C6 and S3
make build-c6     # Build only for ESP32-C6
make build-s3     # Build only for ESP32-S3
```

### 6. Upload to Board

Connect your ESP32 board via USB, then:

```bash
make upload       # Interactive - prompts for board selection
make upload-c6    # Upload to ESP32-C6 (auto-detects port)
make upload-s3    # Upload to ESP32-S3 (auto-detects port)
```

**Entering Flash Mode:** If upload fails, manually enter flash mode:

1. Hold **BOOT** button
2. Press **RESET** button
3. Release **RESET** button
4. Release **BOOT** button
5. Run `make upload` again

### 7. Monitor Serial Output

Note that the serial/debug output is disabled by default. The code must be compiled and uploaded with the following flag: `-D ENABLE_SERIAL_CONNECTION=1` (flag defaults to a value of `0`). Flag value is set in the `platformio.ini`.

View real-time debug output (115200 baud):

```bash
make monitor
```

Press `Ctrl+C` to exit.

---

## Make Commands Reference

### Build Commands

| Command          | Description                                      |
|------------------|--------------------------------------------------|
| `make help`      | Show all available commands                      |
| `make setup`     | Initialize Python venv and install PlatformIO    |
| `make build`     | Build firmware for both boards (C6 and S3)       |
| `make build-c6`  | Build firmware for ESP32-C6 only                 |
| `make build-s3`  | Build firmware for ESP32-S3 only                 |
| `make clean`     | Remove build artifacts                           |
| `make rebuild`   | Clean and rebuild both boards                    |

### Upload Commands

| Command          | Description                                      |
|------------------|--------------------------------------------------|
| `make upload`    | Interactive upload (prompts for board choice)    |
| `make upload-c6` | Build and upload to ESP32-C6                     |
| `make upload-s3` | Build and upload to ESP32-S3                     |

### Utility Commands

| Command          | Description                                      |
|------------------|--------------------------------------------------|
| `make monitor`   | Open serial monitor (115200 baud)                |
| `make size`      | Show firmware size for both boards               |
| `make info`      | Display project status and configuration         |
| `make check-env` | Verify build environment is ready                |
| `make check-secrets` | Verify secret files exist                    |

---

## Project Architecture

### Deep Sleep Cycle

The device operates in a deep sleep cycle to conserve battery power:

1. **Wake from deep sleep** (every 7 hours by default)
2. **Power on peripherals** (soil sensor, fuel gauge)
3. **Connect to WiFi** (with exponential backoff retry)
4. **Read battery status** (voltage, SOC, charge rate)
5. **Connect to MQTT broker**
6. **Publish battery metrics** to Home Assistant
7. **Read soil moisture** (25-sample average)
8. **Publish soil readings** (raw ADC + percentage)
9. **Disconnect MQTT/WiFi gracefully**
10. **Power down peripherals**
11. **Enter deep sleep** (Wake from sleep starts back at #1)

**Sleep duration:** Configurable in `src/main.cpp:280` (default: 7 hours)
**Error handling:** Extended sleep on errors (10 hours for low battery, 20 minutes for MQTT failures)

Resuming from Deep Sleep in `esp32-c6/s3` devices results in the device performing a full setup
(as if the device were just powered on).

### Code Organization

```
soil-sensor/
├── src/
│   └── main.cpp              # Main program loop and deep sleep logic
├── lib/
│   ├── StatusLed/            # RGB LED status indicator
│   ├── BatteryMonitor/       # MAX17048 fuel gauge I2C driver
│   ├── SoilSensor/           # Analog moisture sensor reader
│   └── PubSubConn/           # MQTT client with HA autodiscovery
├── include/
│   ├── wifi_secrets.h        # WiFi credentials (git-ignored)
│   ├── mqtt_secrets.h        # MQTT broker config (git-ignored)
│   └── soil_sensor_config.h  # Sensor calibration values
├── build_version.py          # Injects build version at compile time
├── platformio.ini            # PlatformIO config (boards, pins, libs)
└── Makefile                  # Build system wrapper
```

### Status LED Codes

The RGB LED provides visual feedback (defined in `lib/StatusLed/status.h`):

- **Blue pulse:** WiFi connecting
- **Green solid:** WiFi connected
- **Cyan pulse:** MQTT connecting
- **Orange pulse:** Reading sensors
- **Green pulse:** Battery healthy
- **Yellow:** Battery low
- **Red:** Error state

---

## MQTT Integration with Home Assistant

### MQTT Topics Structure

The device publishes to the following topic format:

```
node/sensor/{client_id}/{metric}
homeassistant/sensor/{client_id}/{metric}/config
```

Where `{client_id}` is generated from the WiFi MAC address (e.g., `soil_sensor_AABBCCDDEEFF`).

### Published Sensors

The device publishes **5 sensors** to Home Assistant via MQTT autodiscovery:

| Sensor Name           | Topic Suffix       | Unit | Description                      |
|-----------------------|-------------------|------|----------------------------------|
| Soil Moisture         | `soil_moisture`   | %    | Moisture percentage (0-100%)     |
| Soil Raw ADC          | `soil_raw`        | -    | Raw ADC reading (diagnostic)     |
| Battery Percentage    | `battery_pct`     | %    | Battery state of charge          |
| Battery Voltage       | `battery_voltage` | V    | Battery voltage (diagnostic)     |
| Battery Change Rate   | `battery_rate`    | %/h  | Charge/discharge rate (diagnostic)|

### Home Assistant Autodiscovery

The device automatically registers with Home Assistant using MQTT Discovery protocol:

- **Discovery topic:** `homeassistant/sensor/{client_id}/{metric}/config`
- **Device info:** Includes firmware version, hardware version, manufacturer
- **Availability:** LWT (Last Will and Testament) on `node/sensor/{client_id}/availability`

**Example configuration published:**

```json
{
  "name": "Soil Moisture",
  "state_topic": "node/sensor/soil_sensor_AABBCC/soil_moisture",
  "unit_of_measurement": "%",
  "device_class": "moisture",
  "device": {
    "identifiers": ["soil_sensor_AABBCC"],
    "name": "Soil Sensor AABBCC",
    "model": "ESP32 Soil Sensor",
    "sw_version": "2026.01.04",
    "hw_version": "esp32c6",
    "manufacturer": "DIY"
  }
}
```

### MQTT Broker Requirements

- **Broker:** Any MQTT 3.1.1 compatible broker (Mosquitto recommended)
- **Port:** 1883 (default unencrypted) or 8883 (TLS, requires code modification)
- **Authentication:** Username/password supported
- **Retained messages:** Discovery configs are published with retain flag
- **LWT:** Used for online/offline availability tracking

---

## Build Version System

The firmware automatically injects version information at build time using `build_version.py`:

### Software Version (`BUILD_SW_VERSION`)

- **Format:** `YYYY.MM.DD` (e.g., `2026.01.04`)
- **Generated from:** UTC timestamp at build time
- **Updates:** Automatically on each build day
- **Purpose:** Track firmware versions in Home Assistant device info

### Hardware Version (`BUILD_HW_VERSION`)

- **Values:** `esp32c6` or `esp32s3`
- **Detected from:** PlatformIO environment name
- **Purpose:** Distinguish between different board types in HA

**Fallback:** If build script fails, values default to `"unknown"` with compiler warnings.

---

## Troubleshooting

### Build Issues

#### "ERROR: python3.13 not found!"

**Cause:** Required Python version not installed.

**Solution:**

```bash
# Fedora/RHEL
sudo dnf install python3.13

# Ubuntu/Debian (if available)
sudo apt install python3.13

# macOS
brew install python@3.13
```

If Python 3.13 isn't available, edit `Makefile` line 11 to use Python 3.10-3.12:

```makefile
PYTHON := python3.12  # or python3.11, python3.10
```

#### "ERROR: Virtual environment not found!"

**Cause:** Build environment not initialized.

**Solution:**

```bash
make setup
```

#### "ERROR: include/wifi_secrets.h not found!"

**Cause:** Secret configuration files not created.

**Solution:**

```bash
cp include/wifi_secrets_example.h include/wifi_secrets.h
cp include/mqtt_secrets_example.h include/mqtt_secrets.h
# Edit both files with your credentials
```

#### Build fails with "Unknown board type in environment"

**Cause:** PlatformIO environment name doesn't contain 'esp32c6' or 'esp32s3'.

**Solution:** Check `platformio.ini` environment names match expected patterns. Update `build_version.py` if using different board naming.

### Upload Issues

#### "No serial port detected" or upload fails

**Solutions:**

1. **Check USB cable:** Use a data cable, not charge-only
2. **Check drivers:** Install CP210x or CH340 USB-to-serial drivers
3. **Manual flash mode:**
   - Hold BOOT button
   - Press RESET button
   - Release RESET
   - Release BOOT
   - Run `make upload` within 5 seconds
4. **Check port permissions (Linux):**

   ```bash
   sudo usermod -a -G dialout $USER
   # Log out and back in
   ```

5. **Specify port manually:**

   ```bash
   .venv/bin/pio run -t upload -e sparkfun_esp32c6_thing_plus --upload-port /dev/ttyUSB0
   ```

#### Upload succeeds but device doesn't run

**Check:**

1. Press RESET button after upload completes
2. Verify power supply (USB provides enough current for basic operation)
3. Check serial output for boot messages: `make monitor`

### Runtime Issues

#### Device not connecting to WiFi

**Debug steps:**

1. Enable serial connection in `platformio.ini`:

   ```ini
   build_flags = ... -D ENABLE_SERIAL_CONNECTION=1
   ```

2. Rebuild and monitor serial output: `make build && make upload && make monitor`
3. Verify WiFi credentials in `include/wifi_secrets.h`
4. Check 2.4GHz WiFi network is available (ESP32 doesn't support 5GHz)
5. Verify WiFi signal strength (move closer to access point)

#### MQTT not connecting

**Debug steps:**

1. Verify MQTT broker IP/port in `include/mqtt_secrets.h`
2. Test broker connectivity from same network:

   ```bash
   mosquitto_sub -h 192.168.1.100 -p 1883 -u username -P password -t '#' -v
   ```

3. Check MQTT broker logs for connection attempts
4. Verify firewall allows port 1883
5. Ensure MQTT buffer size is sufficient (1024 bytes in `lib/PubSubConn/`)

#### Sensors not appearing in Home Assistant

**Debug steps:**

1. Verify MQTT integration is configured in Home Assistant
2. Check MQTT autodiscovery is enabled (Configuration > Integrations > MQTT)
3. Monitor MQTT traffic:

   ```bash
   mosquitto_sub -h 192.168.1.100 -t 'homeassistant/#' -v
   ```

4. Check device availability topic: `node/sensor/{client_id}/availability`
5. Force HA to rediscover: Delete device in HA, restart device

#### Incorrect soil moisture readings

**Solutions:**

1. **Calibrate sensor:** See "Calibrate Soil Sensor" section above
2. **Check sensor power:** Verify GPIO2 provides 3.3V when active
3. **Check ADC reading:** Monitor raw ADC value in serial output
4. **Sensor placement:** Ensure sensor is fully inserted in soil
5. **Sensor quality:** Cheap capacitive sensors can drift over time

#### Battery readings incorrect

**Check:**

1. Battery is properly connected to JST connector
2. MAX17048 fuel gauge is connected via I2C (SDA/SCL)
3. Check I2C address (default 0x36) in `lib/BatteryMonitor/`
4. Verify fuel gauge is getting power

#### Device not entering deep sleep / high power consumption

**Debug steps:**

1. Check serial output for deep sleep entry messages
2. Verify `ENABLE_SERIAL_CONNECTION=0` in production (waiting for serial prevents sleep)
3. Check peripheral power is disabled before sleep (LED should turn off)
4. Measure current: Should be <1mA in deep sleep
5. If >1mA in sleep, peripheral power may not be turning off correctly

### Serial Monitor Issues

#### No serial output after upload

**Solutions:**

1. Wait 20 seconds (if `ENABLE_SERIAL_CONNECTION=1`, device waits for monitor)
2. Press RESET button on board
3. Check baud rate is 115200: `make monitor`
4. Try different serial monitor:

   ```bash
   .venv/bin/pio device monitor --baud 115200
   ```

#### Garbled serial output

**Cause:** Wrong baud rate or ESP32 boot messages (normal).

**Solution:** ESP32 bootloader runs at 74880 baud, then switches to 115200 for application. Initial garbage is normal. Application output should be readable.

---

## Power Consumption & Battery Life

### Typical Power Profile

- **Deep sleep:** ~100-500 µA (ESP32 + MAX17048)
- **Active (WiFi on):** ~80-200 mA for 30-60 seconds
- **Wake cycle:** ~30-60 seconds every 5 hours

### Battery Life Estimate

With a 2000mAh LiPo battery and 5-hour wake interval:

- **Sleep power:** ~0.3 mA average
- **Active duty cycle:** ~60s / (5 * 3600s) = 0.33%
- **Average current:** (0.3 mA × 99.67%) + (150 mA × 0.33%) ≈ 0.8 mA
- **Estimated life:** 2000mAh / 0.8mA ≈ **100 days (~3 months)**

**Actual life varies based on:**

- WiFi signal strength (weaker = more power)
- MQTT connection time
- Number of sensor readings
- Battery age/condition
- Temperature

---

## Customization

### Change Sleep Duration

Edit `src/main.cpp` line ~280:

```cpp
deep_sleep(5 * 60 * 60 * 1000000ULL);  // 5 hours in microseconds
```

### Change Sensor Reading Frequency

Soil moisture is averaged over 25 samples. Edit `lib/SoilSensor/soil_sensor.cpp`:

```cpp
#define NUM_SAMPLES 25  // Increase for more stable readings (slower)
```

### Add More Sensors

1. Create new library in `lib/YourSensor/`
2. Read sensor in `src/main.cpp` loop before deep sleep
3. Publish to MQTT in `lib/PubSubConn/pub_sub_conn.cpp`
4. Add autodiscovery config for new sensor

### Board-Specific Modifications

To add a new board, edit:

1. `platformio.ini`: Add new `[env:your_board]` section
2. `build_version.py`: Add hardware version detection
3. Set board-specific GPIO pins via `build_flags`

---

## License

This project is released under the MIT License. See LICENSE file for details.

**Remember:** This is a hobby/learning project with no warranties. Use at your own risk.

---

## Acknowledgments

- Built with [PlatformIO](https://platformio.org/)
- Uses [PubSubClient](https://github.com/knolleary/pubsubclient) for MQTT
- Uses [SparkFun MAX1704x Library](https://github.com/sparkfun/SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library)
- Designed for [Home Assistant](https://www.home-assistant.io/) integration
- Hardware: SparkFun ESP32 Thing Plus boards

---

## Support & Contributing

**This is a personal hobby project.** I'm sharing it in case it's useful to someone, but I don't provide active support.
