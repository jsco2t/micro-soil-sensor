#include "wifi_conn.h"
#include <WiFi.h>
#include <HardwareSerial.h>
#include <status_led.h>
#include <status.h>

// Configuration constants
#define WIFI_MAX_ATTEMPTS 50
#define WIFI_BASE_DELAY_MS 100
#define WIFI_RECONNECT_INTERVAL 5

// Module-local state
static char stored_ssid[64] = "";
static char stored_password[64] = "";
static wifi_connection_status current_status;
static bool credentials_set = false;

void wifi_conn_set_credentials(const char *ssid, const char *password)
{
    if (!ssid || !password)
    {
        Serial.println(F("WiFi credentials cannot be NULL!"));
        return;
    }

    strncpy(stored_ssid, ssid, sizeof(stored_ssid) - 1);
    stored_ssid[sizeof(stored_ssid) - 1] = '\0';

    strncpy(stored_password, password, sizeof(stored_password) - 1);
    stored_password[sizeof(stored_password) - 1] = '\0';

    credentials_set = true;

    Serial.print(F("WiFi credentials set for SSID: "));
    Serial.println(stored_ssid);
}

bool wifi_conn_start(void *context)
{
    // Initialize status
    current_status.is_valid = false;
    current_status.is_connected = false;
    current_status.attempts_made = 0;
    current_status.ip_address[0] = '\0';

    // Extract credentials from context if provided
    const char *ssid = stored_ssid;
    const char *password = stored_password;

    if (context != NULL)
    {
        board_wifi_config_t *config = (board_wifi_config_t *)context;
        ssid = config->ssid;
        password = config->password;
        Serial.println(F("WiFi: Using credentials from context"));
    }
    else if (!credentials_set)
    {
        Serial.println(F("WiFi credentials not set and no context provided, cannot connect!"));
        return false;
    }
    else
    {
        Serial.println(F("WiFi: Using previously stored credentials (legacy mode)"));
    }

    // Validate credentials
    if (!ssid || !password)
    {
        Serial.println(F("WiFi credentials are NULL, cannot connect!"));
        return false;
    }

    // connect to WiFi
    Serial.print(F("Connecting to WiFi SSID: "));
    Serial.println(ssid);
    set_status_led(STATUS_WIFI_CONNECTING);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_MAX_ATTEMPTS)
    {
        Serial.print(F("."));
        attempts++;
        delay(WIFI_BASE_DELAY_MS * attempts); // incremental back-off

        // try reconnecting every 5 attempts
        if (attempts % WIFI_RECONNECT_INTERVAL == 0)
        {
            WiFi.reconnect();
        }
    }

    current_status.attempts_made = attempts;

    // if we failed to connect to wifi
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(F(""));
        Serial.println(F("Failed to reconnect to WiFi, entering emergency sleep..."));
        set_status_led(STATUS_ERROR);

        current_status.is_valid = true;
        current_status.is_connected = false;
        return false;
    }

    Serial.println(F(""));
    Serial.println(F("WiFi connected."));
    set_status_led(STATUS_WIFI_CONNECTED);
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());

    // Update status with connection details
    current_status.is_valid = true;
    current_status.is_connected = true;
    IPAddress ip = WiFi.localIP();
    snprintf(current_status.ip_address, sizeof(current_status.ip_address),
             "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    return true;
}

wifi_connection_status wifi_conn_get_status()
{
    if (!current_status.is_valid)
    {
        Serial.println(F("WiFi connection status not valid, call wifi_conn_start() first!"));
    }
    return current_status;
}

void wifi_conn_stop(void *context)
{
    // Context parameter reserved for future use
    (void)context; // Suppress unused parameter warning

    Serial.println(F("Shutting down WiFi..."));

    // shut down wifi
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100); // allow time for wifi to shutdown

    // Update status
    current_status.is_valid = false;
    current_status.is_connected = false;
    current_status.ip_address[0] = '\0';

    Serial.println(F("WiFi shutdown complete."));
}
