#ifndef STATUS_H
#define STATUS_H

// general status states

#define STATUS_ERROR 0x01   /// Indicate an error status
#define STATUS_WARNING 0x02 /// Indicate a warning status
#define STATUS_OK 0x03      /// Indicate an OK status

// battery status states

#define STATUS_BATTERY_CHARGE_LOW 0x10      /// Indicate low battery charge status
#define STATUS_BATTERY_CELL_VOLT_LOW 0x11   /// Indicate low battery cell voltage status
#define STATUS_BATTERY_CELL_VOLT_HIGH 0x12  /// Indicate high battery cell voltage status
#define STATUS_BATTERY_IS_CHARGING 0x13     /// Indicate battery is charging
#define STATUS_BATTERY_IS_DISCHARGING 0x14  /// Indicate battery is discharging
#define STATUS_BATTERY_NOT_CHARGING 0x15    /// Indicate battery is not charging
#define STATUS_BATTERY_OK 0x16              /// Indicate battery status is OK
#define STATUS_BATTERY_CHARGE_BELOW_75 0x17 /// Indicate battery charge is below 75%
#define STATUS_BATTERY_CHARGE_BELOW_50 0x18 /// Indicate battery charge is below 50%
#define STATUS_BATTERY_CHARGE_BELOW_30 0x19 /// Indicate battery charge is below 30%
#define STATUS_BATTERY_INVALID_STATUS 0x1A  /// Indicate battery status is invalid

// wifi status states

#define STATUS_WIFI_DISCONNECTED 0x20 /// Indicate WiFi is disconnected
#define STATUS_WIFI_CONNECTING 0x21   /// Indicate WiFi is connecting
#define STATUS_WIFI_CONNECTED 0x22    /// Indicate WiFi is connected

// pubsub status states
#define PUBSUB_PUB_OK 0x30    /// Indicate successful pubsub publish
#define PUBSUB_PUB_ERROR 0x31 /// Indicate failed pubsub publish

#endif // STATUS_H