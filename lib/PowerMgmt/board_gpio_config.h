#ifndef BOARD_GPIO_CONFIG_H
#define BOARD_GPIO_CONFIG_H

#include "../../include/board_config.h"

// Board-specific GPIO pin definitions for power management during deep sleep
// These configurations isolate GPIOs to prevent current leakage

#if defined(BOARD_TYPE_ESP32C6)
    // ESP32-C6 SparkFun Thing Plus GPIO Configuration

    // UART pins (default UART0)
    #define UART_TX_PIN 16
    #define UART_RX_PIN 17

    // USB Serial/JTAG pins
    #define USB_DM_PIN 12
    #define USB_DP_PIN 13

    // I2C pins (default Wire pins for ESP32-C6)
    #define I2C_SDA_PIN 6
    #define I2C_SCL_PIN 7

    // GPIOs that should NOT be isolated (actively used + reserved system pins)
    static const uint8_t ACTIVE_PINS_C6[] = {
        // User pins (actively used by application)
        PERIPHERAL_POWER_PIN,  // GPIO15 - controls peripheral power (inverted logic)
        STATUS_LED_PIN,        // GPIO23 - RGB status LED (handled separately)
        BOARD_LED_PIN,         // GPIO8 - board LED (also strapping pin)
        4,                     // GPIO4 - soil sensor analog input (handled by SoilSensor lib)
        I2C_SDA_PIN,           // GPIO6 - I2C data (handled by BatteryMonitor lib)
        I2C_SCL_PIN,           // GPIO7 - I2C clock (handled by BatteryMonitor lib)

        // Reserved system pins (MUST NOT isolate - prevents potential issues)
        2,   // Strapping: Boot mode selection
        9,   // Strapping: Boot mode selection (also used for I2C SCL in some configs)
        24,  // Flash: SPICS0 (chip select)
        25,  // Flash: SPIQ (data)
        26,  // Flash: SPID (data)
        27,  // Flash: SPIWP (write protect)
        28,  // Flash: SPIHD (hold signal)
        29,  // Flash: SPICLK (clock)
        30   // Flash: SPICS1 (chip select)
    };

    #define ACTIVE_PINS_COUNT (sizeof(ACTIVE_PINS_C6) / sizeof(ACTIVE_PINS_C6[0]))
    #define ACTIVE_PINS ACTIVE_PINS_C6

#elif defined(BOARD_TYPE_ESP32S3)
    // ESP32-S3 SparkFun Thing Plus GPIO Configuration

    // UART pins (default UART0)
    #define UART_TX_PIN 43
    #define UART_RX_PIN 44

    // USB Serial pins
    #define USB_DM_PIN 19
    #define USB_DP_PIN 20

    // I2C pins (default Wire pins for ESP32-S3)
    #define I2C_SDA_PIN 8
    #define I2C_SCL_PIN 9

    // GPIOs that should NOT be isolated (actively used + reserved system pins)
    static const uint8_t ACTIVE_PINS_S3[] = {
        // User pins (actively used by application)
        PERIPHERAL_POWER_PIN,  // GPIO45 - controls peripheral power (also strapping pin)
        STATUS_LED_PIN,        // GPIO46 - RGB status LED (also strapping pin)
        4,                     // GPIO4 - soil sensor analog input (handled by SoilSensor lib)
        I2C_SDA_PIN,           // GPIO8 - I2C data (also BOARD_LED, handled by BatteryMonitor lib)
        I2C_SCL_PIN,           // GPIO9 - I2C clock (handled by BatteryMonitor lib)

        // Reserved system pins (MUST NOT isolate - causes watchdog crashes)
        0,   // Strapping: Boot mode selection
        3,   // Strapping: JTAG enable
        26,  // Flash: SPICS1
        27,  // Flash: SPIHD (hold signal)
        28,  // Flash: SPIWP (write protect)
        29,  // Flash: SPICS0 (chip select)
        30,  // Flash: SPICLK (clock)
        31,  // Flash: SPIQ (data I/O)
        32,  // Flash: SPID (data I/O)
        33,  // Octal PSRAM: SPIIO4 (if used)
        34,  // Octal PSRAM: SPIIO5 (if used)
        35,  // Octal PSRAM: SPIIO6 (if used)
        36,  // Octal PSRAM: SPIIO7 (if used)
        37   // Octal PSRAM: SPIDQS (if used)
    };

    #define ACTIVE_PINS_COUNT (sizeof(ACTIVE_PINS_S3) / sizeof(ACTIVE_PINS_S3[0]))
    #define ACTIVE_PINS ACTIVE_PINS_S3

#else
    #warning "Unknown board type for GPIO power management"

    // Safe fallback configuration
    #define UART_TX_PIN 1
    #define UART_RX_PIN 3
    #define USB_DM_PIN 19
    #define USB_DP_PIN 20
    #define I2C_SDA_PIN 21
    #define I2C_SCL_PIN 22

    static const uint8_t ACTIVE_PINS_DEFAULT[] = {4, 8, 21, 22};
    #define ACTIVE_PINS_COUNT (sizeof(ACTIVE_PINS_DEFAULT) / sizeof(ACTIVE_PINS_DEFAULT[0]))
    #define ACTIVE_PINS ACTIVE_PINS_DEFAULT
#endif

#endif // BOARD_GPIO_CONFIG_H
