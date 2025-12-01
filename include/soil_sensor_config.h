#ifndef SOIL_SENSOR_CONFIG_H
#define SOIL_SENSOR_CONFIG_H

// Analog output pin connected to the soil moisture sensor
#define SOIL_SENSOR_AOUT_PIN 4

// Default ADC value when soil is completely dry (higher reading)
// Typical range: 3000-4095
#define SOIL_CONFIG_DEFAULT_DRY_VALUE 2733

// Default ADC value when soil is completely wet (lower reading)
// Typical range: 0-1500
#define SOIL_CONFIG_DEFAULT_WET_VALUE 1380

// Pin controlling peripheral power (e.g., sensors VCC enable)
// Can be overridden via PlatformIO build flags per environment.
#ifndef PERIPHERAL_POWER_PIN
#define PERIPHERAL_POWER_PIN 45
#endif

#endif // SOIL_SENSOR_CONFIG_H