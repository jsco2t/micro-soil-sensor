#include "battery_monitor.h"
#include "SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h"
#include <Wire.h>

// Configuration constants
#define BATTERY_MONITOR_MAX_RETRIES 10
#define BATTERY_MONITOR_RETRY_DELAY_MS 10

SFE_MAX1704X lipo(MAX1704X_MAX17048); // Create a MAX17048
bool is_started = false;

bool battery_monitor_start(void *context)
{
    // Context parameter reserved for future use (e.g., I2C config)
    (void)context; // Suppress unused parameter warning

    if (is_started)
    {
        return true; // already started
    }

    if (!Wire.begin()) // Initialize I2C
    {
        Serial.println(F("Failed to initialize I2C!"));
        return false;
    }

    // Start the MAX17048 sensor with retry logic
    for (int attempt = 0; attempt < BATTERY_MONITOR_MAX_RETRIES; attempt++)
    {
        if (lipo.begin(Wire))
        {
            is_started = true;
            battery_monitor_reset();
            return true;
        }
        delay(BATTERY_MONITOR_RETRY_DELAY_MS * (attempt + 1)); // Exponential back-off
    }

    is_started = false;
    Serial.print(F("Could not find a valid MAX17048 sensor after "));
    Serial.print(BATTERY_MONITOR_MAX_RETRIES);
    Serial.println(F(" attempts, check wiring!"));
    return false;
}

void battery_monitor_reset()
{
    if (!is_started)
    {
        Serial.println(F("Battery monitor not started, cannot reset!"));
        return;
    }

    Serial.println(F("Resetting the MAX17048..."));

    // Read and print the reset indicator
    Serial.print(F("Reset Indicator was: "));
    bool resetIndicator = lipo.isReset(true); // Read the RI flag and clear it automatically if it is set
    Serial.println(resetIndicator);           // Print the RI
    // If RI was set, check it is now clear
    if (resetIndicator)
    {
        Serial.print(F("Reset Indicator is now: "));
        resetIndicator = lipo.isReset(); // Read the RI flag
        Serial.println(resetIndicator);  // Print the RI
    }
    delay(1000); // let the IC settle after reset
}

void battery_monitor_stop(void *context)
{
    // Context parameter reserved for future use
    (void)context; // Suppress unused parameter warning

    if (!is_started)
    {
        Serial.println(F("Battery monitor not started, cannot stop!"));
        return;
    }

    Serial.println(F("Stopping the MAX17048..."));

    if (!Wire.end()) // Deinitialize I2C
    {
        Serial.println(F("Failed to deinitialize I2C!"));
    }

    is_started = false;
}

battery_status read_battery_status()
{
    battery_status status;
    status.is_valid = false;

    if (!is_started)
    {
        Serial.println(F("Battery monitor not started, cannot read status!"));
        return status;
    }

    float batteryVoltage = lipo.getVoltage();
    if (isnan(batteryVoltage))
    {
        Serial.println(F("Failed to read cell voltage, check battery is connected!"));
        return status;
    }
    status.voltage = batteryVoltage;

    float batteryStateOfCharge = lipo.getSOC(); // SOC = State of Charge (percentage - may take 1s before accurate)
    if (isnan(batteryStateOfCharge))
    {
        Serial.println(F("Failed to read cell percent, check battery is connected!"));
        return status;
    }
    status.state_of_charge = batteryStateOfCharge;

    // positive = charging, negative = discharging
    float changeRate = lipo.getChangeRate();
    if (isnan(changeRate))
    {
        Serial.println(F("Failed to read change rate, check battery is connected!"));
        return status;
    }
    status.change_rate = changeRate;

    status.is_valid = true;
    status.is_charging = (changeRate > 0);
    status.is_low_voltage = lipo.isVoltageLow();
    status.is_high_voltage = lipo.isVoltageHigh();

    // Log the structured status using the C-friendly formatter
    char buf[128];
    battery_status_format(&status, buf, sizeof(buf));
    Serial.println(buf);

    return status;
}