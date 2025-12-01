#ifndef STATUS_LED_H
#define STATUS_LED_H

#include "status.h"
#include <cstdint> // For uint8_t

// Allow environment-specific override of the status LED pin.
// If not defined via build flags, fall back to Arduino's LED_BUILTIN.
#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN LED_BUILTIN
#endif

// Allow environment-specific override of RGB LED brightness (0-255).
// If not defined via build flags, use moderate brightness.
#ifndef RGB_BRIGHTNESS
#define RGB_BRIGHTNESS 64
#endif

#define STATUS_LED_RED 0x01    /// Red LED color
#define STATUS_LED_GREEN 0x02  /// Green LED color
#define STATUS_LED_BLUE 0x03   /// Blue LED color
#define STATUS_LED_YELLOW 0x04 /// Yellow LED color
#define STATUS_LED_ORANGE 0x05 /// Orange LED color
#define STATUS_LED_PURPLE 0x06 /// Purple LED color
#define STATUS_LED_CYAN 0x07   /// Cyan LED color
#define STATUS_LED_WHITE 0x08  /// White LED color
#define STATUS_LED_OFF 0x09    /// Turn LED off

void set_status_led(uint8_t status);
void set_custom_status_led(uint8_t statusLedColor);
void pulse_status_led(unsigned int pulseCount, uint8_t statusLedColor);
void pulse_slow_status_led(unsigned int pulseCount, uint8_t statusLedColor);
void pulse_fast_status_led(unsigned int pulseCount, uint8_t statusLedColor);
void pulse_custom_status_led(unsigned int pulseCount, unsigned int pulseDurationMs, unsigned int pauseDurationMs, uint8_t statusLedColor);

/**
 * Shut down status LED for deep sleep.
 * Turns off the LED and sets the pin to INPUT_PULLDOWN to prevent current leakage.
 * Should be called in prep_for_sleep() before entering deep sleep.
 *
 * @param context Optional context pointer (currently unused, reserved for future use)
 */
void shutdown_status_led(void *context);

#endif // STATUS_LED_H