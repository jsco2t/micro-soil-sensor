#ifndef PUB_SUB_CONN_H
#define PUB_SUB_CONN_H

#include <Arduino.h>

/**
 * Configuration structure for MQTT connection.
 * Pass via context parameter to pubsub_connect().
 */
typedef struct {
    const char *broker_ip;
    uint16_t broker_port;
    const char *username;
    const char *password;
} board_mqtt_config_t;

String get_mqtt_topic(const char *topicFormat);

/**
 * Setup and connect to MQTT broker in one step.
 * Combines setup_pubsub() + connect_pubsub() for simplified lifecycle management.
 *
 * @param context Pointer to mqtt_config_t with broker details.
 *                If NULL, falls back to mqtt_secrets.h values (legacy mode).
 * @return true if connected successfully, false if failed
 */
bool pubsub_connect(void *context);

void publish_autodisco_messages();
bool publish_pub_sub_message(const char *topic, const char *payload);

/**
 * Disconnect from MQTT broker and shut down WiFi.
 * Processes pending messages before disconnecting gracefully.
 *
 * @param context Optional context pointer (currently unused, reserved for future use)
 */
void pubsub_disconnect(void *context);

bool is_autodisco_published();

// Legacy functions (prefer pubsub_connect instead)
void setup_pubsub();
bool connect_pubsub();
void disconnect_pubsub();

#endif // PUB_SUB_CONN_H