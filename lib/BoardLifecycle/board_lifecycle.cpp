#include "board_lifecycle.h"
#include <Arduino.h>
#include <esp_sleep.h>

// Maximum number of callbacks that can be registered for each lifecycle phase
#define MAX_LIFECYCLE_CALLBACKS 8

// Internal structure to store callback and its context
typedef struct
{
    board_lifecycle_callback callback;
    void *context;
} board_lifecycle_callback_entry;

// Static storage for wakeup callbacks (FIFO execution: index 0 → N-1)
static board_lifecycle_callback_entry wakeup_callbacks[MAX_LIFECYCLE_CALLBACKS];
static uint8_t wakeup_callback_count = 0;

// Static storage for sleep callbacks (LIFO execution: index N-1 → 0)
static board_lifecycle_callback_entry sleep_callbacks[MAX_LIFECYCLE_CALLBACKS];
static uint8_t sleep_callback_count = 0;

// Metrics tracking
static board_lifecycle_metrics metrics = {0};

bool board_lifecycle_register_wakeup(board_lifecycle_callback callback, void *context)
{
    if (callback == NULL)
    {
        Serial.println(F("BoardLifecycle: WARNING - Attempted to register NULL wakeup callback"));
        return false;
    }

    if (wakeup_callback_count >= MAX_LIFECYCLE_CALLBACKS)
    {
        Serial.print(F("BoardLifecycle: ERROR - Wakeup callback array full (max "));
        Serial.print(MAX_LIFECYCLE_CALLBACKS);
        Serial.println(F("), callback not registered"));
        return false;
    }

    wakeup_callbacks[wakeup_callback_count].callback = callback;
    wakeup_callbacks[wakeup_callback_count].context = context;
    wakeup_callback_count++;

    metrics.wakeup_callbacks_registered = wakeup_callback_count;

    Serial.print(F("BoardLifecycle: Registered wakeup callback #"));
    Serial.println(wakeup_callback_count);

    return true;
}

bool board_lifecycle_register_sleep(board_lifecycle_callback callback, void *context)
{
    if (callback == NULL)
    {
        Serial.println(F("BoardLifecycle: WARNING - Attempted to register NULL sleep callback"));
        return false;
    }

    if (sleep_callback_count >= MAX_LIFECYCLE_CALLBACKS)
    {
        Serial.print(F("BoardLifecycle: ERROR - Sleep callback array full (max "));
        Serial.print(MAX_LIFECYCLE_CALLBACKS);
        Serial.println(F("), callback not registered"));
        return false;
    }

    sleep_callbacks[sleep_callback_count].callback = callback;
    sleep_callbacks[sleep_callback_count].context = context;
    sleep_callback_count++;

    metrics.sleep_callbacks_registered = sleep_callback_count;

    Serial.print(F("BoardLifecycle: Registered sleep callback #"));
    Serial.println(sleep_callback_count);

    return true;
}

board_lifecycle_status board_lifecycle_wakeup(void)
{
    metrics.wakeup_failures = 0;
    uint32_t start_time = millis();

    Serial.println(F("BoardLifecycle: ===== Starting wakeup sequence (FIFO order) ====="));
    Serial.print(F("BoardLifecycle: Executing "));
    Serial.print(wakeup_callback_count);
    Serial.println(F(" wakeup callbacks"));

    // Execute callbacks in FIFO order (0 → N-1)
    for (uint8_t i = 0; i < wakeup_callback_count; i++)
    {
        board_lifecycle_callback_entry *entry = &wakeup_callbacks[i];

        Serial.print(F("BoardLifecycle: ["));
        Serial.print(i + 1);
        Serial.print(F("/"));
        Serial.print(wakeup_callback_count);
        Serial.print(F("] Executing wakeup callback..."));

        if (entry->callback != NULL)
        {
            uint32_t callback_start = millis();

            // Execute the callback
            entry->callback(entry->context);

            uint32_t callback_time = millis() - callback_start;

            Serial.print(F(" completed in "));
            Serial.print(callback_time);
            Serial.println(F("ms"));
        }
        else
        {
            Serial.println(F(" SKIPPED (NULL callback)"));
            metrics.wakeup_failures++;
        }
    }

    metrics.last_wakeup_time_ms = millis() - start_time;

    Serial.print(F("BoardLifecycle: Wakeup sequence complete in "));
    Serial.print(metrics.last_wakeup_time_ms);
    Serial.println(F("ms"));
    Serial.println(F("BoardLifecycle: ====================================="));

    if (metrics.wakeup_failures > 0)
    {
        if (metrics.wakeup_failures >= wakeup_callback_count)
        {
            return BOARD_LIFECYCLE_TOTAL_FAILURE;
        }
        return BOARD_LIFECYCLE_PARTIAL_FAILURE;
    }

    return BOARD_LIFECYCLE_SUCCESS;
}

board_lifecycle_status board_lifecycle_prep_sleep(void)
{
    metrics.sleep_failures = 0;
    uint32_t start_time = millis();

    Serial.println(F("BoardLifecycle: ===== Starting sleep preparation (LIFO order) ====="));
    Serial.print(F("BoardLifecycle: Executing "));
    Serial.print(sleep_callback_count);
    Serial.println(F(" sleep callbacks"));

    // Execute callbacks in LIFO order (N-1 → 0)
    for (int8_t i = sleep_callback_count - 1; i >= 0; i--)
    {
        board_lifecycle_callback_entry *entry = &sleep_callbacks[i];

        Serial.print(F("BoardLifecycle: ["));
        Serial.print(sleep_callback_count - i);
        Serial.print(F("/"));
        Serial.print(sleep_callback_count);
        Serial.print(F("] Executing sleep callback..."));

        if (entry->callback != NULL)
        {
            uint32_t callback_start = millis();

            // Execute the callback
            entry->callback(entry->context);

            uint32_t callback_time = millis() - callback_start;

            Serial.print(F(" completed in "));
            Serial.print(callback_time);
            Serial.println(F("ms"));
        }
        else
        {
            Serial.println(F(" SKIPPED (NULL callback)"));
            metrics.sleep_failures++;
        }
    }

    metrics.last_sleep_prep_time_ms = millis() - start_time;

    Serial.print(F("BoardLifecycle: Sleep preparation complete in "));
    Serial.print(metrics.last_sleep_prep_time_ms);
    Serial.println(F("ms"));
    Serial.println(F("BoardLifecycle: ====================================="));

    if (metrics.sleep_failures > 0)
    {
        if (metrics.sleep_failures >= sleep_callback_count)
        {
            return BOARD_LIFECYCLE_TOTAL_FAILURE;
        }
        return BOARD_LIFECYCLE_PARTIAL_FAILURE;
    }

    return BOARD_LIFECYCLE_SUCCESS;
}

void board_lifecycle_enter_sleep(uint64_t seconds)
{
    // Execute all sleep preparation callbacks first
    board_lifecycle_prep_sleep();

    Serial.print(F("BoardLifecycle: Entering deep sleep for "));
    Serial.print(seconds);
    Serial.println(F(" seconds..."));

    // Disable all wakeup sources first
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    // Enable timer wakeup
    esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);

    // Enter deep sleep (this function does not return)
    esp_deep_sleep_start();
}

board_lifecycle_metrics board_lifecycle_get_metrics(void)
{
    return metrics;
}

void board_lifecycle_print_metrics(void)
{
    Serial.println(F(""));
    Serial.println(F("BoardLifecycle: ===== Lifecycle Metrics ====="));
    Serial.print(F("  Wakeup callbacks registered: "));
    Serial.println(metrics.wakeup_callbacks_registered);
    Serial.print(F("  Sleep callbacks registered:  "));
    Serial.println(metrics.sleep_callbacks_registered);
    Serial.print(F("  Wakeup failures (last run):  "));
    Serial.println(metrics.wakeup_failures);
    Serial.print(F("  Sleep failures (last run):   "));
    Serial.println(metrics.sleep_failures);
    Serial.print(F("  Last wakeup time:            "));
    Serial.print(metrics.last_wakeup_time_ms);
    Serial.println(F("ms"));
    Serial.print(F("  Last sleep prep time:        "));
    Serial.print(metrics.last_sleep_prep_time_ms);
    Serial.println(F("ms"));
    Serial.println(F("BoardLifecycle: ============================="));
    Serial.println(F(""));
}
