#include "battery_monitor.h"
#include <status_led.h>
#include <HardwareSerial.h>

void battery_status_to_led(const battery_status *status)
{
    if (!status || !status->is_valid)
    {
        Serial.println("Invalid battery status, report as error with LED!");
        set_status_led(STATUS_ERROR);
        return;
    }

    if (status->is_charging)
    {
        Serial.print(F("Battery is CHARGING at a rate of "));
        Serial.print(status->change_rate, 1);
        Serial.println(F(" %/hr"));
        set_status_led(STATUS_BATTERY_IS_CHARGING);
    }
    else
    {
        Serial.print(F("Battery is DISCHARGING at a rate of "));
        Serial.print(status->change_rate, 1);
        Serial.println(F(" %/hr"));
        // set_status_led(STATUS_BATTERY_IS_DISCHARGING);
    }

    if (status->state_of_charge < 25.0)
    {
        set_status_led(STATUS_BATTERY_CHARGE_LOW);
    }
    else if (status->state_of_charge < 50.0)
    {
        set_status_led(STATUS_BATTERY_CHARGE_BELOW_50);
    }
    else if (status->state_of_charge < 75.0)
    {
        set_status_led(STATUS_BATTERY_CHARGE_BELOW_75);
    }
    else
    {
        set_status_led(STATUS_BATTERY_OK);
    }
}