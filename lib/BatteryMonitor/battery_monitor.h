#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

typedef struct battery_status
{
    float voltage;
    float state_of_charge;
    float change_rate;
    bool is_valid;
    bool is_charging;
    bool is_low_voltage;
    bool is_high_voltage;
} battery_status;

bool battery_monitor_start(void *context);
void battery_monitor_reset();
void battery_monitor_stop(void *context);
battery_status read_battery_status();
void battery_status_to_led(const battery_status *status);

// C-friendly formatter: writes a textual representation of battery_status into out.
// Returns the number of characters written (excluding the null terminator). If truncated,
// returns out_size - 1. Writes an empty string and returns 0 on error.
#include <stdio.h>
static inline size_t battery_status_format(const battery_status *s, char *out, size_t out_size)
{
    if (!s || !out || out_size == 0)
    {
        return 0;
    }

    int n = snprintf(out, out_size,
                     "BatteryStatus{voltage=%.3fV, percent=%.1f%%, rate=%.3f, valid=%s, charging=%s, low=%s, high=%s}",
                     s->voltage,
                     s->state_of_charge,
                     s->change_rate,
                     s->is_valid ? "true" : "false",
                     s->is_charging ? "true" : "false",
                     s->is_low_voltage ? "true" : "false",
                     s->is_high_voltage ? "true" : "false");

    if (n < 0)
    {
        out[0] = '\0';
        return 0;
    }
    if ((size_t)n >= out_size)
    {
        return out_size - 1; // truncated
    }
    return (size_t)n;
}

#endif // BATTERY_MONITOR_H