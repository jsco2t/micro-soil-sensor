#include "pub_sub_conn.h"
#include "../../include/mqtt.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <wifi_conn.h> // For WiFi shutdown in disconnect

// Fallback version definitions if build script doesn't run
#ifndef BUILD_SW_VERSION
#define BUILD_SW_VERSION "unknown"
#warning "BUILD_SW_VERSION not defined by build script, using fallback"
#endif

#ifndef BUILD_HW_VERSION
#define BUILD_HW_VERSION "unknown"
#warning "BUILD_HW_VERSION not defined by build script, using fallback"
#endif

// Default MQTT configuration (used if context is NULL)
static const char *DEFAULT_MQTT_SERVER = "192.168.1.1";
static const uint16_t DEFAULT_MQTT_PORT = 1883;
static const char *DEFAULT_MQTT_USER = "";
static const char *DEFAULT_MQTT_PASS = "";

// Configuration constants
#define MQTT_BUFFER_SIZE 1024
#define MQTT_KEEPALIVE_SEC 30
#define MQTT_CONNECT_MAX_RETRIES 25
#define MQTT_CONNECT_RETRY_DELAY_MS 2000
#define MQTT_DISCONNECT_LOOP_COUNT 10
#define MQTT_DISCONNECT_LOOP_DELAY_MS 200

WiFiClient wifiClient;
PubSubClient pubsubClient(wifiClient);

// Flag to track if autodiscovery messages have been published
static bool autodisco_published = false;

String get_client_id()
{
    uint8_t mac[6];
    WiFi.macAddress(mac);
    String clientID = "soilsensor_";
    for (int i = 0; i < 6; i++)
    {
        clientID += String(mac[i], HEX);
    }
    return clientID;
}

String get_mqtt_topic(const char *topicFormat)
{
    String deviceID = get_client_id();
    char topic[256];
    snprintf(topic, sizeof(topic), topicFormat, deviceID.c_str());
    return String(topic);
}

void setup_pubsub()
{
    // DEPRECATED: Use pubsub_connect() with context parameter instead
    Serial.println(F("WARNING: setup_pubsub() is deprecated, use pubsub_connect() with context"));
    pubsubClient.setServer(DEFAULT_MQTT_SERVER, DEFAULT_MQTT_PORT);
    // Increase MQTT buffer to handle Home Assistant discovery JSON payloads
    pubsubClient.setBufferSize(MQTT_BUFFER_SIZE);
    // Adjust keepalive to give broker time to process messages
    pubsubClient.setKeepAlive(MQTT_KEEPALIVE_SEC);
    return;
}

bool connect_pubsub()
{
    // DEPRECATED: Use pubsub_connect() with context parameter instead
    Serial.println(F("WARNING: connect_pubsub() is deprecated, use pubsub_connect() with context"));

    int attempts = 0;
    while (!pubsubClient.connected() && attempts < MQTT_CONNECT_MAX_RETRIES)
    {
        String clientID = get_client_id();
        Serial.print(F("Connecting to MQTT as: "));
        Serial.println(clientID);

        // Set Last Will and Testament (LWT) on availability topic
        String availabilityTopic = get_mqtt_topic(AVAILABILITY_MQTT_TOPIC);
        const char *willPayload = "offline";
        // QoS 0 (PubSubClient), retain true so HA sees offline if we drop
        if (pubsubClient.connect(clientID.c_str(), DEFAULT_MQTT_USER, DEFAULT_MQTT_PASS,
                                 availabilityTopic.c_str(), 0, true, willPayload))
        {
            Serial.println(F("Connected to MQTT"));
            // Publish 'online' availability
            pubsubClient.publish(availabilityTopic.c_str(), "online", true);
            publish_autodisco_messages();
            pubsubClient.loop();
            return true;
        }
        else
        {
            Serial.print(F("MQTT connection failed, rc="));
            Serial.print(pubsubClient.state());
            Serial.println(F(" retrying..."));
            attempts++;
            delay(MQTT_CONNECT_RETRY_DELAY_MS);
        }
    }

    if (!pubsubClient.connected())
    {
        Serial.println(F("Failed to connect to MQTT after multiple attempts"));
        return false;
    }

    return false;
}

bool pubsub_connect(void *context)
{
    // Extract MQTT configuration from context if provided
    const char *broker = DEFAULT_MQTT_SERVER;
    uint16_t port = DEFAULT_MQTT_PORT;
    const char *user = DEFAULT_MQTT_USER;
    const char *pass = DEFAULT_MQTT_PASS;

    if (context != NULL)
    {
        board_mqtt_config_t *config = (board_mqtt_config_t *)context;
        broker = config->broker_ip;
        port = config->broker_port;
        user = config->username;
        pass = config->password;
        Serial.println(F("MQTT: Using configuration from context"));
    }
    else
    {
        Serial.println(F("MQTT: Using default configuration (WARNING: likely incorrect!)"));
    }

    // Setup MQTT client with configuration
    pubsubClient.setServer(broker, port);
    pubsubClient.setBufferSize(MQTT_BUFFER_SIZE);
    pubsubClient.setKeepAlive(MQTT_KEEPALIVE_SEC);

    // Connect to broker (same logic as connect_pubsub)
    int attempts = 0;
    while (!pubsubClient.connected() && attempts < MQTT_CONNECT_MAX_RETRIES)
    {
        String clientID = get_client_id();
        Serial.print(F("Connecting to MQTT as: "));
        Serial.println(clientID);

        // Set Last Will and Testament (LWT) on availability topic
        String availabilityTopic = get_mqtt_topic(AVAILABILITY_MQTT_TOPIC);
        const char *willPayload = "offline";

        if (pubsubClient.connect(clientID.c_str(), user, pass,
                                 availabilityTopic.c_str(), 0, true, willPayload))
        {
            Serial.println(F("Connected to MQTT"));
            // Publish 'online' availability
            pubsubClient.publish(availabilityTopic.c_str(), "online", true);
            publish_autodisco_messages();
            pubsubClient.loop();
            return true;
        }
        else
        {
            Serial.print(F("MQTT connection failed, rc="));
            Serial.print(pubsubClient.state());
            Serial.println(F(" retrying..."));
            attempts++;
            delay(MQTT_CONNECT_RETRY_DELAY_MS);
        }
    }

    if (!pubsubClient.connected())
    {
        Serial.println(F("Failed to connect to MQTT after multiple attempts"));
        return false;
    }

    return false;
}

void publish_autodisco_messages()
{
    String deviceId = get_client_id();
    String deviceJson = "\"device\":{";
    deviceJson += "\"identifiers\":[\"" + deviceId + "\"],";
    deviceJson += "\"name\":\"Soil Sensor\",";
    deviceJson += "\"manufacturer\":\"SBW\",";
    deviceJson += "\"model\":\"SBW Soil Sensor\",";
    deviceJson += "\"sw_version\":\"" BUILD_SW_VERSION "\",";
    deviceJson += "\"hw_version\":\"" BUILD_HW_VERSION "\"";
    deviceJson += "}";

    // availability details
    String availabilityTopic = get_mqtt_topic(AVAILABILITY_MQTT_TOPIC);
    String availabilityJson = "\"availability_topic\":\"" + availabilityTopic + "\",";
    availabilityJson += "\"payload_available\":\"online\",";
    availabilityJson += "\"payload_not_available\":\"offline\",";

    // Soil percent sensor
    String configTopic = get_mqtt_topic(SOIL_SENSOR_PERCENT_CONFIG_MQTT_TOPIC);
    String payloadTopic = get_mqtt_topic(SOIL_SENSOR_PERCENT_MQTT_TOPIC);
    String payload = "{";
    payload += "\"name\":\"Soil Moisture\",";
    payload += "\"state_topic\":\"" + payloadTopic + "\",";
    payload += "\"unique_id\":\"" + deviceId + "_soil_percent\",";
    payload += "\"unit_of_measurement\":\"%\",";
    payload += "\"device_class\":\"moisture\",";
    payload += "\"state_class\":\"measurement\",";
    payload += availabilityJson;
    payload += deviceJson + "}";
    bool ok = pubsubClient.publish(configTopic.c_str(), payload.c_str(), true);
    delay(100);
    if (!ok)
    {
        Serial.println("WARN: Failed to publish soil moisture discovery config (likely buffer too small)");
    }
    else
    {
        Serial.println("Published soil moisture sensor autodiscovery config");
    }
    Serial.print("Topic: ");
    Serial.println(configTopic);
    Serial.print("Payload: ");
    Serial.println(payload);
    Serial.println("");
    // ----------------------------------------

    // Raw ADC reading
    configTopic = get_mqtt_topic(SOIL_SENSOR_RAW_CONFIG_MQTT_TOPIC);
    payloadTopic = get_mqtt_topic(SOIL_SENSOR_RAW_MQTT_TOPIC);
    payload = "{";
    payload += "\"name\":\"Soil Raw\",";
    payload += "\"state_topic\":\"" + payloadTopic + "\",";
    payload += "\"unique_id\":\"" + deviceId + "_soil_raw\",";
    payload += "\"entity_category\":\"diagnostic\",";
    payload += "\"state_class\":\"measurement\",";
    payload += availabilityJson;
    payload += deviceJson + "}";
    ok &= pubsubClient.publish(configTopic.c_str(), payload.c_str(), true);
    delay(100);
    if (!ok)
    {
        Serial.println("WARN: Failed to publish soil raw discovery config");
    }
    else
    {
        Serial.println("Published soil raw sensor autodiscovery config");
    }
    Serial.print("Topic: ");
    Serial.println(configTopic);
    Serial.print("Payload: ");
    Serial.println(payload);
    Serial.println("");
    // ----------------------------------------

    // Battery level
    configTopic = get_mqtt_topic(BATTERY_PERCENTAGE_CONFIG_MQTT_TOPIC);
    payloadTopic = get_mqtt_topic(BATTERY_PERCENTAGE_MQTT_TOPIC);
    payload = "{";
    payload += "\"name\":\"Battery\",";
    payload += "\"state_topic\":\"" + payloadTopic + "\",";
    payload += "\"unique_id\":\"" + deviceId + "_battery\",";
    payload += "\"unit_of_measurement\":\"%\",";
    payload += "\"device_class\":\"battery\",";
    payload += "\"state_class\":\"measurement\",";
    payload += availabilityJson;
    payload += deviceJson + "}";
    ok &= pubsubClient.publish(configTopic.c_str(), payload.c_str(), true);
    delay(100);
    if (!ok)
    {
        Serial.println("WARN: Failed to publish battery percentage discovery config");
    }
    else
    {
        Serial.println("Published battery percentage autodiscovery config");
    }
    Serial.print("Topic: ");
    Serial.println(configTopic);
    Serial.print("Payload: ");
    Serial.println(payload);

    Serial.println("");
    // ----------------------------------------

    // Battery voltage reading
    configTopic = get_mqtt_topic(BATTERY_VOLTAGE_CONFIG_MQTT_TOPIC);
    payloadTopic = get_mqtt_topic(BATTERY_VOLTAGE_MQTT_TOPIC);
    payload = "{";
    payload += "\"name\":\"Battery Voltage\",";
    payload += "\"state_topic\":\"" + payloadTopic + "\",";
    payload += "\"unique_id\":\"" + deviceId + "_battery_voltage\",";
    payload += "\"unit_of_measurement\":\"V\",";
    payload += "\"device_class\":\"voltage\",";
    payload += "\"state_class\":\"measurement\",";
    payload += "\"entity_category\":\"diagnostic\",";
    payload += availabilityJson;
    payload += deviceJson + "}";
    ok &= pubsubClient.publish(configTopic.c_str(), payload.c_str(), true);
    delay(100);
    if (!ok)
    {
        Serial.println("WARN: Failed to publish battery voltage discovery config");
    }
    else
    {
        Serial.println("Published battery voltage autodiscovery config");
    }
    Serial.print("Topic: ");
    Serial.println(configTopic);
    Serial.print("Payload: ");
    Serial.println(payload);
    Serial.println("");
    // ----------------------------------------

    // Battery change rate reading
    configTopic = get_mqtt_topic(BATTERY_DISCHARGE_RATE_CONFIG_MQTT_TOPIC);
    payloadTopic = get_mqtt_topic(BATTERY_DISCHARGE_RATE_MQTT_TOPIC);
    payload = "{";
    payload += "\"name\":\"Battery Change Rate\",";
    payload += "\"state_topic\":\"" + payloadTopic + "\",";
    payload += "\"unique_id\":\"" + deviceId + "_battery_change_rate\",";
    // The MAX17048 CRATE register reports percent change per hour
    payload += "\"unit_of_measurement\":\"%/h\",";
    payload += "\"state_class\":\"measurement\",";
    payload += "\"entity_category\":\"diagnostic\",";
    payload += availabilityJson;
    payload += deviceJson + "}";
    ok &= pubsubClient.publish(configTopic.c_str(), payload.c_str(), true);
    delay(100);
    if (!ok)
    {
        Serial.println("WARN: Failed to publish battery change rate discovery config");
    }
    else
    {
        Serial.println("Published battery change rate autodiscovery config");
    }
    Serial.print("Topic: ");
    Serial.println(configTopic);
    Serial.print("Payload: ");
    Serial.println(payload);
    Serial.println("");
    // ----------------------------------------

    pubsubClient.loop();
    delay(500);

    // Mark autodiscovery as published
    autodisco_published = ok;
    Serial.println("All autodiscovery messages published");
}

bool publish_pub_sub_message(const char *topic, const char *payload)
{
    // attempt connection rescue if not connected
    if (!pubsubClient.connected())
    {
        connect_pubsub();
    }

    // Ensure autodiscovery has been published before publishing state messages
    if (pubsubClient.connected() && !autodisco_published)
    {
        Serial.println("Autodiscovery not yet published, publishing now...");
        publish_autodisco_messages();
    }

    if (pubsubClient.connected())
    {
        if (pubsubClient.publish(topic, payload))
        {
            Serial.print("Published to topic ");
            Serial.print(topic);
            Serial.print(": ");
            Serial.println(payload);
            return true;
        }
        else
        {
            Serial.print("Failed to publish to topic ");
            Serial.println(topic);
        }
    }
    else
    {
        Serial.println("Cannot publish, MQTT client not connected");
    }

    return false;
}

void disconnect_pubsub()
{
    if (pubsubClient.connected())
    {
        // Process any remaining MQTT messages
        for (int i = 0; i < MQTT_DISCONNECT_LOOP_COUNT; i++)
        {
            pubsubClient.loop();
            delay(MQTT_DISCONNECT_LOOP_DELAY_MS);
        }

        Serial.println(F("Disconnecting from MQTT gracefully..."));
        pubsubClient.disconnect();
        delay(100);
        // Reset autodiscovery published flag on disconnect
        autodisco_published = false;
    }
    else
    {
        Serial.println(F("MQTT client already disconnected"));
    }
}

void pubsub_disconnect(void *context)
{
    // Disconnect from MQTT first
    disconnect_pubsub();

    // Then shutdown WiFi (MQTT requires WiFi, so turn it off too)
    wifi_conn_stop(context);
}

bool is_autodisco_published()
{
    return autodisco_published;
}