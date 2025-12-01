#ifndef WIFI_CONN_H
#define WIFI_CONN_H

#include <stddef.h>

typedef struct wifi_connection_status
{
    bool is_connected;
    char ip_address[16]; // "xxx.xxx.xxx.xxx" format
    int attempts_made;
    bool is_valid;
} wifi_connection_status;

/**
 * Configuration structure for WiFi credentials.
 * Pass via context parameter to wifi_conn_start().
 */
typedef struct {
    const char *ssid;
    const char *password;
} board_wifi_config_t;

/**
 * Start WiFi connection with exponential backoff.
 * Attempts to connect up to 50 times with increasing delays.
 * Updates internal connection status.
 *
 * @param context Pointer to wifi_config_t with SSID and password.
 *                If NULL, falls back to previously set credentials (legacy mode).
 * @return true if connected successfully, false if failed
 */
bool wifi_conn_start(void *context);

/**
 * Get current WiFi connection status.
 * Returns detailed connection information including IP address and attempt count.
 *
 * @return wifi_connection_status struct with current status
 */
wifi_connection_status wifi_conn_get_status();

/**
 * Stop WiFi connection and shut down WiFi radio completely.
 * Disconnects from network and powers off WiFi subsystem.
 *
 * @param context Optional context pointer (currently unused, reserved for future use)
 */
void wifi_conn_stop(void *context);

/**
 * Set WiFi credentials for connection (LEGACY - prefer passing via context to wifi_conn_start).
 * Must be called before wifi_conn_start() if not using context parameter.
 *
 * @param ssid WiFi network SSID
 * @param password WiFi network password
 */
void wifi_conn_set_credentials(const char *ssid, const char *password);

#endif // WIFI_CONN_H
