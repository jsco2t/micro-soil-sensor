#ifndef BOARD_LIFECYCLE_H
#define BOARD_LIFECYCLE_H

#include <stdint.h>

/**
 * Board Lifecycle Library for ESP32 Soil Sensor
 *
 * Manages board wakeup and sleep lifecycle with ordered callback execution.
 * Wakeup callbacks execute in FIFO order (registration order: 1,2,3).
 * Sleep callbacks execute in LIFO order (reverse order: 3,2,1).
 *
 * This allows peripherals to be powered on and initialized in dependency order,
 * then shut down in reverse order to properly release resources.
 *
 * Example:
 *   Register: peripheral_power_on, power_mgmt_post_wakeup, battery_monitor_start
 *   Wakeup:   1→2→3 (peripheral power first, then cleanup holds, then battery)
 *   Sleep:    3→2→1 (battery first, then power mgmt, then peripheral power off)
 */

/**
 * Lifecycle operation status codes.
 */
typedef enum
{
    BOARD_LIFECYCLE_SUCCESS = 0,        // All callbacks executed successfully
    BOARD_LIFECYCLE_PARTIAL_FAILURE = 1, // Some callbacks failed but execution continued
    BOARD_LIFECYCLE_TOTAL_FAILURE = 2    // Critical failure during lifecycle execution
} board_lifecycle_status;

/**
 * Lifecycle callback function signature.
 * Callbacks receive an optional context pointer.
 *
 * @param context User-defined context data (can be NULL)
 */
typedef void (*board_lifecycle_callback)(void *context);

/**
 * Metrics tracked during lifecycle operations.
 * Useful for debugging performance issues and identifying slow callbacks.
 */
typedef struct
{
    uint8_t wakeup_callbacks_registered; // Number of registered wakeup callbacks
    uint8_t sleep_callbacks_registered;  // Number of registered sleep callbacks
    uint8_t wakeup_failures;             // Count of failed wakeup callbacks in last execution
    uint8_t sleep_failures;              // Count of failed sleep callbacks in last execution
    uint32_t last_wakeup_time_ms;        // Total time for last wakeup execution (milliseconds)
    uint32_t last_sleep_prep_time_ms;    // Total time for last sleep prep execution (milliseconds)
} board_lifecycle_metrics;

/**
 * Register a callback to execute during wakeup.
 * Callbacks execute in FIFO order (registration order).
 *
 * @param callback Function pointer to callback (must not be NULL)
 * @param context Optional context pointer (can be NULL)
 * @return true if registered successfully, false if array is full or callback is NULL
 */
bool board_lifecycle_register_wakeup(board_lifecycle_callback callback, void *context);

/**
 * Register a callback to execute during sleep preparation.
 * Callbacks execute in LIFO order (reverse registration order).
 *
 * @param callback Function pointer to callback (must not be NULL)
 * @param context Optional context pointer (can be NULL)
 * @return true if registered successfully, false if array is full or callback is NULL
 */
bool board_lifecycle_register_sleep(board_lifecycle_callback callback, void *context);

/**
 * Execute all registered wakeup callbacks in FIFO order.
 * Should be called early in setup() after serial initialization.
 *
 * Measures execution time for each callback and tracks failures.
 * Continues execution even if individual callbacks fail.
 *
 * @return Status code indicating overall success or failure
 */
board_lifecycle_status board_lifecycle_wakeup(void);

/**
 * Execute all registered sleep callbacks in LIFO order.
 * Should be called in prep_for_sleep() before entering deep sleep.
 *
 * Measures execution time for each callback and tracks failures.
 * Continues execution even if individual callbacks fail.
 *
 * @return Status code indicating overall success or failure
 */
board_lifecycle_status board_lifecycle_prep_sleep(void);

/**
 * Configure ESP32 timer wakeup and enter deep sleep.
 * This function does not return.
 *
 * @param seconds Number of seconds to sleep before waking up
 */
void board_lifecycle_enter_sleep(uint64_t seconds);

/**
 * Get current lifecycle metrics for debugging.
 * Returns metrics from the last wakeup/sleep execution.
 *
 * @return Metrics structure with current values
 */
board_lifecycle_metrics board_lifecycle_get_metrics(void);

/**
 * Print lifecycle metrics to Serial for debugging.
 * Shows registration counts, failure counts, and execution times.
 */
void board_lifecycle_print_metrics(void);

#endif // BOARD_LIFECYCLE_H
