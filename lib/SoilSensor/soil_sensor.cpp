#include "soil_sensor.h"
#include "../../include/soil_sensor_config.h"
#include <HardwareSerial.h>

bool soil_sensor_start(void *context)
{
    // Context parameter reserved for future use (e.g., sensor config)
    (void)context; // Suppress unused parameter warning

    // Power on the soil sensor if VCC pin is defined
    if (SOIL_SENSOR_VCC_PIN != -1)
    {
        pinMode(SOIL_SENSOR_VCC_PIN, OUTPUT);
        digitalWrite(SOIL_SENSOR_VCC_PIN, HIGH); // Power ON
        delay(500);                              // Give time for sensor to power up
        Serial.println(F("Soil sensor powered ON"));
    }
    else
    {
        Serial.println(F("Soil sensor VCC pin not defined, assuming always powered"));
        return false;
    }

    return true;
}

void soil_sensor_stop(void *context)
{
    // Context parameter reserved for future use
    (void)context; // Suppress unused parameter warning

    // Power off the soil sensor if VCC pin is defined
    if (SOIL_SENSOR_VCC_PIN != -1)
    {
        digitalWrite(SOIL_SENSOR_VCC_PIN, LOW); // Power OFF
        Serial.println(F("Soil sensor powered OFF"));
    }

    // Also prepare GPIO for sleep (formerly prep_soil_sensor_for_sleep)
    // Pull down the soil sensor pin to prevent floating input during sleep
    pinMode(SOIL_SENSOR_AOUT_PIN, INPUT_PULLDOWN);
    Serial.println(F("Soil sensor AOUT pin set to INPUT_PULLDOWN for sleep"));
}

SoilSensorReading read_soil_moisture()
{
    SoilSensorReading reading;
    reading.rawValue = get_average_reading(25);

    // Validate calibration values to prevent division by zero
    int calibrationRange = SOIL_CONFIG_DEFAULT_DRY_VALUE - SOIL_CONFIG_DEFAULT_WET_VALUE;
    if (calibrationRange == 0)
    {
        Serial.println(F("ERROR: Invalid soil sensor calibration (DRY == WET)!"));
        reading.moisturePercent = 0;
        return reading;
    }

    if (SOIL_CONFIG_DEFAULT_DRY_VALUE <= SOIL_CONFIG_DEFAULT_WET_VALUE)
    {
        Serial.println(F("WARNING: Soil sensor calibration inverted (DRY <= WET)!"));
    }

    if (reading.rawValue == 0)
    {
        Serial.println(F("ERROR: Invalid soil sensor reading (RAW == 0)!"));
        reading.moisturePercent = 0;
        return reading;
    }

    if (reading.rawValue > SOIL_CONFIG_DEFAULT_DRY_VALUE)
    {
        Serial.print(F("WARNING: Soil sensor reading ("));
        Serial.print(reading.rawValue);
        Serial.println(F(") exceeds DRY calibration value"));
        reading.rawValue = SOIL_CONFIG_DEFAULT_DRY_VALUE;
        Serial.print(F("Clamping reading to DRY value: "));
        Serial.println(reading.rawValue);
    }
    else if (reading.rawValue < SOIL_CONFIG_DEFAULT_WET_VALUE)
    {
        Serial.print(F("WARNING: Soil sensor reading ("));
        Serial.print(reading.rawValue);
        Serial.println(F(") below WET calibration value"));
        reading.rawValue = SOIL_CONFIG_DEFAULT_WET_VALUE;
        Serial.print(F("Clamping reading to WET value: "));
        Serial.println(reading.rawValue);
    }

    // Linear interpolation: moisture = 100 * (dry - raw) / (dry - wet)
    // Note: dry_value > wet_value (dry reads higher ADC)
    int moisture = 100 * (SOIL_CONFIG_DEFAULT_DRY_VALUE - reading.rawValue) / calibrationRange;
    Serial.print(F("Soil Sensor Moisture: "));
    Serial.print(moisture);
    Serial.println(F("%"));

    // Clamp to 0-100%
    if (moisture < 0)
    {
        moisture = 0;
    }

    if (moisture > 100)
    {
        moisture = 100;
    }

    reading.moisturePercent = moisture;

    return reading;
}

int get_average_reading(int samples)
{
    // Validate and clamp sample count to reasonable range
    if (samples <= 0)
    {
        samples = 10;
    }
    else if (samples > 100)
    {
        Serial.print(F("WARNING: Sample count too high ("));
        Serial.print(samples);
        Serial.println(F("), clamping to 100"));
        samples = 100;
    }

    int total = 0;
    for (int i = 0; i < samples; i++)
    {
        total += analogRead(SOIL_SENSOR_AOUT_PIN);
        delay(50); // small delay between samples
    }
    return (total) / samples;
}

void prep_soil_sensor_for_sleep()
{
    // pull down the soil sensor pin to prevent floating input during sleep
    pinMode(SOIL_SENSOR_AOUT_PIN, INPUT_PULLDOWN);
}