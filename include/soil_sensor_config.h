#ifndef SOIL_SENSOR_CONFIG_H
#define SOIL_SENSOR_CONFIG_H

// Analog output pin connected to the soil moisture sensor
#define SOIL_SENSOR_AOUT_PIN 4

// Default ADC value when soil is completely dry (higher reading)
// Typical range: 3000-4095
// For `s3`: 3300 works well with the SparkFun Soil Sensor in dry air
// For `c6`: 2750 works well with the SparkFun Soil Sensor in dry air
#define SOIL_CONFIG_DEFAULT_DRY_VALUE 3300 // needs to be calibrated per sensor/per board: value should be slightly higher than the dry reading

// Default ADC value when soil is completely wet (lower reading)
// Typical range: 0-1500
// For `s3`: 1550 works well with the SparkFun Soil Sensor submerged in water
// For `c6`: 1380 works well with the SparkFun Soil Sensor submerged in water
#define SOIL_CONFIG_DEFAULT_WET_VALUE 1550 // needs to be calibrated per sensor/per board: value should be slightly lower than the submerged in water/wet reading

// Pin controlling peripheral power (e.g., sensors VCC enable)
// Can be overridden via PlatformIO build flags per environment.
#ifndef PERIPHERAL_POWER_PIN
#define PERIPHERAL_POWER_PIN 45
#endif

#endif // SOIL_SENSOR_CONFIG_H