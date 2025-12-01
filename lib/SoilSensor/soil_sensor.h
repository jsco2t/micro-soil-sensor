#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

struct SoilSensorReading
{
    int rawValue;
    int moisturePercent; // 0-100%
};

int get_average_reading(int samples);
SoilSensorReading read_soil_moisture();

/**
 * Power on the soil sensor.
 *
 * @param context Optional context pointer (currently unused, reserved for future use)
 * @return true if sensor powered on successfully, false otherwise
 */
bool soil_sensor_start(void *context);

/**
 * Power off the soil sensor and prepare GPIO for sleep.
 * Combines power-off and GPIO isolation (formerly prep_soil_sensor_for_sleep).
 *
 * @param context Optional context pointer (currently unused, reserved for future use)
 */
void soil_sensor_stop(void *context);

// Legacy function (prefer soil_sensor_stop instead)
void prep_soil_sensor_for_sleep();

#endif // SOIL_SENSOR_H