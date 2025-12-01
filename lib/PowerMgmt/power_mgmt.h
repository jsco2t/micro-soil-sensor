#ifndef POWER_MGMT_H
#define POWER_MGMT_H

#include <Arduino.h>

/**
 * Power Management Library for ESP32 Soil Sensor
 *
 * Provides comprehensive GPIO isolation and power optimization for deep sleep.
 * Reduces idle current consumption by preventing GPIO leakage and maintaining
 * peripheral power states.
 *
 * Key Features:
 * - Board-specific GPIO isolation (ESP32-C6 and ESP32-S3)
 * - GPIO hold for peripheral power control
 * - UART and USB pin isolation
 * - Integration with existing power management (BatteryMonitor, StatusLed, SoilSensor)
 */

/**
 * Initialize power management system.
 * Should be called once in setup() after peripheral_power_on().
 *
 * Performs post-wakeup cleanup:
 * - Releases GPIO hold states from previous sleep cycle
 * - Prepares GPIO subsystem for normal operation
 *
 * @param context Optional context pointer (currently unused, reserved for future use)
 */
void power_mgmt_post_wakeup(void *context);

/**
 * Prepare all GPIOs and peripherals for deep sleep.
 * Should be called in prep_for_sleep() after all peripheral shutdown functions.
 *
 * Performs:
 * - Shuts down Bluetooth radio
 * - Configures ESP32 power domains for deep sleep
 * - Isolates board LED pin
 * - Isolates I2C pins
 * - Isolates unused GPIOs
 * - Enables GPIO hold on peripheral power pin
 * - Isolates UART and USB pins (disables Serial output)
 *
 * @param context Optional context pointer (currently unused, reserved for future use)
 */
void power_mgmt_prep_sleep(void *context);

/**
 * Emergency sleep preparation with minimal cleanup.
 * Used when WiFi connection fails and system needs to sleep immediately.
 *
 * Performs same operations as power_mgmt_prep_sleep() but optimized
 * for emergency scenarios where some peripherals may not be initialized.
 */
void power_mgmt_emergency_sleep();

#endif // POWER_MGMT_H
