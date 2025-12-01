#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// Board type enumeration
// One of these should be defined via build flags in platformio.ini
#if defined(BOARD_TYPE_ESP32C6)
#define BOARD_TYPE ESP32_C6
#define PERIPHERAL_POWER_INVERTED 1 // C6: LOW=ON, HIGH=OFF
#elif defined(BOARD_TYPE_ESP32S3)
#define BOARD_TYPE ESP32_S3
#define PERIPHERAL_POWER_INVERTED 0 // S3: HIGH=ON, LOW=OFF
#else
#warning "No board type defined! Defaulting to ESP32-S3 behavior."
#define BOARD_TYPE ESP32_S3
#define PERIPHERAL_POWER_INVERTED 0
#endif

// Helper macros for peripheral power control
#if PERIPHERAL_POWER_INVERTED
#define PERIPHERAL_POWER_ON_STATE LOW
#define PERIPHERAL_POWER_OFF_STATE HIGH
#else
#define PERIPHERAL_POWER_ON_STATE HIGH
#define PERIPHERAL_POWER_OFF_STATE LOW
#endif

// Power control pin for soil sensor (not optional)
//
// Assumes soil sensor VCC is controlled via a GPIO pin
// Define SOIL_SENSOR_VCC_PIN in platformio.ini to enable
// If not defined, soil sensor power control is disabled
//
// Pin must be set to `HIGH` to power the soil sensor, and needs to
// be set to `LOW` or `INPUT_PULLDOWN` to disable power during sleep.
//
#ifndef SOIL_SENSOR_VCC_PIN
#define SOIL_SENSOR_VCC_PIN -1 // Not used unless defined
#endif

#endif // BOARD_CONFIG_H
