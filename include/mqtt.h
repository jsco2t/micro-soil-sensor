#ifndef MQTT_H
#define MQTT_H

// soil sensor topics
#define SOIL_SENSOR_PERCENT_MQTT_TOPIC "node/sensor/%s/moisture_percent"
#define SOIL_SENSOR_PERCENT_CONFIG_MQTT_TOPIC "homeassistant/sensor/%s/moisture_percent/config"
#define SOIL_SENSOR_RAW_MQTT_TOPIC "node/sensor/%s/moisture_reading_raw"
#define SOIL_SENSOR_RAW_CONFIG_MQTT_TOPIC "homeassistant/sensor/%s/moisture_reading_raw/config"

// availability (LWT) topic
#define AVAILABILITY_MQTT_TOPIC "node/sensor/%s/availability"

// battery status topics
#define BATTERY_VOLTAGE_MQTT_TOPIC "node/sensor/%s/voltage"
#define BATTERY_VOLTAGE_CONFIG_MQTT_TOPIC "homeassistant/sensor/%s/voltage/config"
#define BATTERY_PERCENTAGE_MQTT_TOPIC "node/sensor/%s/charge_percentage"
#define BATTERY_PERCENTAGE_CONFIG_MQTT_TOPIC "homeassistant/sensor/%s/charge_percentage/config"
// #define BATTERY_CHARGING_MQTT_TOPIC "homeassistant/sensor/%s/battery/charging"
#define BATTERY_DISCHARGE_RATE_MQTT_TOPIC "node/sensor/%s/discharge_rate"
#define BATTERY_DISCHARGE_RATE_CONFIG_MQTT_TOPIC "homeassistant/sensor/%s/discharge_rate/config"

#endif // MQTT_H