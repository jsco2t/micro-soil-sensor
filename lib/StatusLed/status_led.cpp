#include "status_led.h"
#include <Arduino.h>

void set_status_led(uint8_t status)
{
    switch (status)
    {
    case STATUS_ERROR:
        pulse_status_led(2, STATUS_LED_RED);
        break;
    case STATUS_WARNING:
        pulse_status_led(2, STATUS_LED_YELLOW);
        break;
    case STATUS_OK:
        pulse_status_led(2, STATUS_LED_GREEN);
        break;
    case STATUS_BATTERY_CHARGE_BELOW_75:
        pulse_fast_status_led(1, STATUS_LED_CYAN);
        break;
    case STATUS_BATTERY_CHARGE_BELOW_50:
        pulse_fast_status_led(1, STATUS_LED_YELLOW);
        break;
    case STATUS_BATTERY_CHARGE_BELOW_30:
        pulse_fast_status_led(1, STATUS_LED_ORANGE);
        break;
    case STATUS_BATTERY_CHARGE_LOW:
        pulse_fast_status_led(3, STATUS_LED_ORANGE);
        break;
    case STATUS_BATTERY_CELL_VOLT_LOW:
        pulse_fast_status_led(3, STATUS_LED_ORANGE);
        break;
    case STATUS_BATTERY_CELL_VOLT_HIGH:
        pulse_fast_status_led(3, STATUS_LED_RED);
        break;
    case STATUS_BATTERY_IS_CHARGING:
        pulse_fast_status_led(1, STATUS_LED_BLUE);
        break;
    case STATUS_BATTERY_IS_DISCHARGING:
        pulse_fast_status_led(1, STATUS_LED_GREEN);
        break;
    case STATUS_BATTERY_NOT_CHARGING:
        pulse_fast_status_led(1, STATUS_LED_PURPLE);
        break;
    case STATUS_BATTERY_INVALID_STATUS:
        pulse_fast_status_led(1, STATUS_LED_RED);
        pulse_fast_status_led(1, STATUS_LED_ORANGE);
        pulse_fast_status_led(1, STATUS_LED_RED);
        pulse_fast_status_led(1, STATUS_LED_PURPLE);
        break;
    case STATUS_BATTERY_OK:
        pulse_fast_status_led(1, STATUS_LED_WHITE);
        break;
    case STATUS_WIFI_DISCONNECTED:
        pulse_fast_status_led(2, STATUS_LED_RED);
        break;
    case STATUS_WIFI_CONNECTING:
        pulse_fast_status_led(1, STATUS_LED_YELLOW);
        break;
    case STATUS_WIFI_CONNECTED:
        pulse_fast_status_led(1, STATUS_LED_GREEN);
        break;
    case PUBSUB_PUB_OK:
        pulse_fast_status_led(1, STATUS_LED_GREEN);
        break;
    case PUBSUB_PUB_ERROR:
        pulse_fast_status_led(1, STATUS_LED_ORANGE);
        break;
    default:
        set_custom_status_led(STATUS_LED_OFF);
        break;
    }
}

void set_custom_status_led(uint8_t statusLedColor)
{
    Serial.print(F("Status LED pin is: "));
    Serial.print(STATUS_LED_PIN);
    Serial.print(F(", setting color code: "));
    Serial.println(statusLedColor);
    switch (statusLedColor)
    {
    case STATUS_LED_RED:
        // Set LED to RED
        rgbLedWrite(STATUS_LED_PIN, RGB_BRIGHTNESS, 0, 0); // Red
        break;
    case STATUS_LED_YELLOW:
        // Set LED to YELLOW
        rgbLedWrite(STATUS_LED_PIN, RGB_BRIGHTNESS, RGB_BRIGHTNESS / 2, 0); // Yellow
        break;
    case STATUS_LED_ORANGE:
        // Set LED to ORANGE
        rgbLedWrite(STATUS_LED_PIN, RGB_BRIGHTNESS, RGB_BRIGHTNESS / 4, 0); // Orange
        break;
    case STATUS_LED_GREEN:
        // Set LED to GREEN
        rgbLedWrite(STATUS_LED_PIN, 0, RGB_BRIGHTNESS, 0); // Green
        break;
    case STATUS_LED_BLUE:
        // Set LED to BLUE
        rgbLedWrite(STATUS_LED_PIN, 0, 0, RGB_BRIGHTNESS); // Blue
        break;
    case STATUS_LED_PURPLE:
        // Set LED to PURPLE
        rgbLedWrite(STATUS_LED_PIN, RGB_BRIGHTNESS - 12, 0, RGB_BRIGHTNESS); // Purple
        break;
    case STATUS_LED_CYAN:
        // Set LED to CYAN
        rgbLedWrite(STATUS_LED_PIN, 0, RGB_BRIGHTNESS, RGB_BRIGHTNESS); // Cyan
        break;
    case STATUS_LED_WHITE:
        // Set LED to WHITE
        rgbLedWrite(STATUS_LED_PIN, RGB_BRIGHTNESS, RGB_BRIGHTNESS, RGB_BRIGHTNESS); // White
        break;
    case STATUS_LED_OFF:
        [[fallthrough]];
    default:
        // Turn LED OFF
        rgbLedWrite(STATUS_LED_PIN, 0, 0, 0); // Off / black
        break;
    }
}

void pulse_status_led(unsigned int pulseCount, uint8_t statusLedColor)
{
    pulse_custom_status_led(pulseCount, 250, 250, statusLedColor);
}

void pulse_slow_status_led(unsigned int pulseCount, uint8_t statusLedColor)
{
    pulse_custom_status_led(pulseCount, 500, 1000, statusLedColor);
}

void pulse_fast_status_led(unsigned int pulseCount, uint8_t statusLedColor)
{
    pulse_custom_status_led(pulseCount, 125, 125, statusLedColor);
}

void pulse_custom_status_led(unsigned int pulseCount, unsigned int pulseDurationMs, unsigned int pauseDurationMs, uint8_t statusLedColor)
{
    Serial.print(F("Pulsing status LED with: "));
    Serial.print(pulseCount);
    Serial.print(F(" pulses, "));
    Serial.print(pulseDurationMs);
    Serial.print(F("ms pulse duration, "));
    Serial.print(pauseDurationMs);
    Serial.print(F("ms pause duration, color code: "));
    Serial.println(statusLedColor);

    for (unsigned int i = 0; i < pulseCount; i++)
    {
        set_custom_status_led(statusLedColor);
        delay(pulseDurationMs);
        // Turn LED OFF
        rgbLedWrite(STATUS_LED_PIN, 0, 0, 0); // Off / black
        delay(pauseDurationMs);
    }
}

void shutdown_status_led(void *context)
{
    // Context parameter reserved for future use
    (void)context; // Suppress unused parameter warning

    // Turn off the RGB LED completely
    rgbLedWrite(STATUS_LED_PIN, 0, 0, 0);

    // Set the status LED pin to INPUT_PULLDOWN to prevent current leakage during deep sleep
    pinMode(STATUS_LED_PIN, INPUT_PULLDOWN);

    Serial.print(F("Status LED shutdown complete (pin "));
    Serial.print(STATUS_LED_PIN);
    Serial.println(F(" set to INPUT_PULLDOWN)"));
}