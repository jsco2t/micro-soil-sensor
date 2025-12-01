#include <HardwareSerial.h>
#include <status.h>
#include <status_led.h>
#include <battery_monitor.h>
#include <esp_sleep.h>
#include <wifi_conn.h>
#include "wifi_secrets.h" // contains WiFi SSID and password
#include "mqtt.h"
#include "mqtt_secrets.h" // contains MQTT server info (NOW safe to include!)
#include "board_config.h"
#include "soil_sensor_config.h"
#include <pub_sub_conn.h>
#include <soil_sensor.h>
#include <power_mgmt.h>
#include <board_lifecycle.h>

// =====  Board Configuration Structure =====
// Unified configuration for all subsystems
typedef struct
{
    // WiFi configuration
    board_wifi_config_t wifi;

    // MQTT configuration
    board_mqtt_config_t mqtt;

    // Note: Other subsystems (battery, soil, power_mgmt, status_led)
    // don't need runtime configuration yet - they use build flags
} board_config;

// ===== Peripheral Power Functions =====
// Replaces all the duplicate wrapper code from old main.cpp

static void peripheral_power_on(void *context)
{
    // Context parameter reserved for future use
    (void)context;

    pinMode(PERIPHERAL_POWER_PIN, OUTPUT);
    digitalWrite(PERIPHERAL_POWER_PIN, PERIPHERAL_POWER_ON_STATE);

#if PERIPHERAL_POWER_INVERTED
    Serial.println(F("Peripheral power ON (inverted logic: LOW=ON)"));
#else
    Serial.println(F("Peripheral power ON (normal logic: HIGH=ON)"));
#endif

    delay(500); // Give time for peripherals to power up
}

static void peripheral_power_off(void *context)
{
    // Context parameter reserved for future use
    (void)context;

    pinMode(PERIPHERAL_POWER_PIN, OUTPUT);
    digitalWrite(PERIPHERAL_POWER_PIN, PERIPHERAL_POWER_OFF_STATE);

#if PERIPHERAL_POWER_INVERTED
    Serial.println(F("Peripheral power OFF (inverted logic: HIGH=OFF)"));
#else
    Serial.println(F("Peripheral power OFF (normal logic: LOW=OFF)"));
#endif

    delay(500); // Give time for peripherals to power down
}

// ===== Lifecycle Callback Wrappers =====
// Wrap bool-returning functions to match void lifecycle callback signature

static void wakeup_battery_monitor(void *context)
{
    if (!battery_monitor_start(context))
    {
        Serial.println(F("WARNING: Battery monitor failed to start"));
    }
}

static void wakeup_wifi(void *context)
{
    if (!wifi_conn_start(context))
    {
        Serial.println(F("WARNING: WiFi connection failed"));
    }
}

static void wakeup_mqtt(void *context)
{
    if (!pubsub_connect(context))
    {
        Serial.println(F("WARNING: MQTT connection failed"));
    }
}

static void wakeup_soil_sensor(void *context)
{
    if (!soil_sensor_start(context))
    {
        Serial.println(F("WARNING: Soil sensor failed to start"));
    }
}

// ===== Lifecycle Registration Helper =====
// Centralizes all callback registration with clear dependency ordering

void register_all_lifecycle_callbacks(board_config *config)
{
    Serial.println(F("Registering lifecycle callbacks..."));

    // ===== WAKEUP CALLBACKS (FIFO: execute in registration order) =====

    // 1. Peripheral power - MUST BE FIRST (powers sensors/battery)
    board_lifecycle_register_wakeup(peripheral_power_on, config);

    // 2. Power management - releases GPIO holds from previous sleep
    board_lifecycle_register_wakeup(power_mgmt_post_wakeup, config);

    // 3. Battery monitor - depends on peripheral power + I2C
    board_lifecycle_register_wakeup(wakeup_battery_monitor, config);

    // 4. WiFi connection - independent, but MQTT needs it
    board_lifecycle_register_wakeup(wakeup_wifi, &config->wifi);

    // 5. MQTT connection - depends on WiFi
    board_lifecycle_register_wakeup(wakeup_mqtt, &config->mqtt);

    // 6. Soil sensor - depends on peripheral power
    board_lifecycle_register_wakeup(wakeup_soil_sensor, config);

    // ===== SLEEP CALLBACKS (LIFO: execute in reverse order) =====
    // Register in reverse so LIFO execution matches dependency order

    board_lifecycle_register_sleep(power_mgmt_prep_sleep, config);    // Executes 6th (LAST)
    board_lifecycle_register_sleep(peripheral_power_off, config);     // Executes 5th
    board_lifecycle_register_sleep(battery_monitor_stop, config);     // Executes 4th
    board_lifecycle_register_sleep(shutdown_status_led, config);      // Executes 3rd
    board_lifecycle_register_sleep(soil_sensor_stop, config);         // Executes 2nd
    board_lifecycle_register_sleep(pubsub_disconnect, &config->mqtt); // Executes 1st (FIRST - also shuts down WiFi)

    Serial.println(F("Lifecycle callbacks registered successfully"));
}

// ===== Setup Function =====
// Configure and start lifecycle

void setup()
{
    // 1. Check if we are outputting to serial
    if (ENABLE_SERIAL_CONNECTION != 0)
    {
        Serial.begin(115200);
        delay(500); // Give time for Serial to initialize

        unsigned long startTime = millis();
        while (millis() - startTime < 20000)
        {
            if (Serial)
            {
                Serial.println("Serial connection detected, proceeding...");
                break;
            }
            delay(100);
        }
    }
    else
    {
        Serial.end();
    }

    Serial.println("Board setup started...");

    // 2. Configure subsystems
    board_config config = {
        .wifi = {
            .ssid = ssid,
            .password = password},
        .mqtt = {.broker_ip = mqtt_server, .broker_port = mqtt_server_port, .username = mqtt_user, .password = mqtt_password}};

    // 3. Register wakeup and sleep callback functions
    register_all_lifecycle_callbacks(&config);

    // 4. Call the lifecycle wakeup method - let it start the system up
    board_lifecycle_status wakeup_status = board_lifecycle_wakeup();

    if (wakeup_status == BOARD_LIFECYCLE_TOTAL_FAILURE)
    {
        Serial.println(F("CRITICAL: Wakeup lifecycle failed completely"));
        board_lifecycle_print_metrics();
        board_lifecycle_enter_sleep(1200ULL); // Emergency 20min sleep
    }
    else if (wakeup_status == BOARD_LIFECYCLE_PARTIAL_FAILURE)
    {
        Serial.println(F("WARNING: Some wakeup callbacks failed"));
        board_lifecycle_print_metrics();
        // Continue anyway - some subsystems may still work
    }

    // 5. Final ADC configuration
    analogSetAttenuation(ADC_11db);

    Serial.println(F("Setup complete!"));
}

// ===== Helper Function for Publishing =====

void publish_with_status(const char *topic, const char *payload)
{
    if (publish_pub_sub_message(topic, payload))
    {
        Serial.print("Published to topic: ");
        Serial.print(topic);
        Serial.print(" with payload: ");
        Serial.println(payload);
    }
    else
    {
        set_status_led(PUBSUB_PUB_ERROR);
        delay(100);
    }
}

// ===== Main Loop =====
// Loop runs once per wake cycle, then device enters deep sleep

void loop()
{
    // Read battery status
    battery_status status = read_battery_status();

    // Check battery validity
    if (!status.is_valid)
    {
        Serial.println("Battery status is INVALID, entering sleep to conserve power...");
        set_status_led(STATUS_BATTERY_INVALID_STATUS);
        board_lifecycle_enter_sleep(10ULL * 3600); // sleep for 10 hours
    }
    else if (status.is_valid && status.is_low_voltage && !status.is_charging)
    {
        Serial.println("Battery voltage is LOW, entering sleep to conserve power...");
        set_status_led(STATUS_BATTERY_CHARGE_LOW);
        board_lifecycle_enter_sleep(10ULL * 3600); // sleep for 10 hours
    }

    battery_status_to_led(&status);

    // Publish battery metrics
    if (status.is_valid)
    {
        Serial.print("Battery Voltage: ");
        Serial.print(status.voltage, 3);
        Serial.print(" V, State of Charge: ");
        Serial.print(status.state_of_charge, 1);
        Serial.print(" %, Change Rate: ");
        Serial.print(status.change_rate, 3);
        Serial.println(" %/hr");

        char voltageStr[10];
        snprintf(voltageStr, sizeof(voltageStr), "%.3f", status.voltage);
        String topic = get_mqtt_topic(BATTERY_VOLTAGE_MQTT_TOPIC);
        publish_with_status(topic.c_str(), voltageStr);

        char socStr[10];
        snprintf(socStr, sizeof(socStr), "%.1f", status.state_of_charge);
        topic = get_mqtt_topic(BATTERY_PERCENTAGE_MQTT_TOPIC);
        publish_with_status(topic.c_str(), socStr);

        char rateStr[10];
        snprintf(rateStr, sizeof(rateStr), "%.3f", status.change_rate);
        topic = get_mqtt_topic(BATTERY_DISCHARGE_RATE_MQTT_TOPIC);
        publish_with_status(topic.c_str(), rateStr);
    }
    else
    {
        Serial.println("Battery status is invalid, skipping publish.");
    }

    // Read and publish soil moisture
    SoilSensorReading soilReading = read_soil_moisture();
    Serial.print("Soil moisture reading: ");
    Serial.print(soilReading.rawValue);
    Serial.print(" (");
    Serial.print(soilReading.moisturePercent);
    Serial.println("%)");

    char moistureStr[10];
    snprintf(moistureStr, sizeof(moistureStr), "%d", soilReading.moisturePercent);
    String topic = get_mqtt_topic(SOIL_SENSOR_PERCENT_MQTT_TOPIC);
    publish_with_status(topic.c_str(), moistureStr);

    char rawStr[10];
    snprintf(rawStr, sizeof(rawStr), "%d", soilReading.rawValue);
    topic = get_mqtt_topic(SOIL_SENSOR_RAW_MQTT_TOPIC);
    publish_with_status(topic.c_str(), rawStr);

    Serial.print("Published moisture reading: ");
    Serial.println(moistureStr);

    // Success indication
    pulse_status_led(1, STATUS_LED_WHITE);

    // Enter deep sleep
    Serial.println("Entering sleep...");
    board_lifecycle_enter_sleep(7ULL * 3600); // sleep for 7 hours
    // board_lifecycle_enter_sleep(1ULL * 240);
}
