#include "power_mgmt.h"
#include "board_gpio_config.h"
#include <driver/gpio.h>
#include <esp_sleep.h>

// Forward declaration for btStop (from esp32-hal-bt.c)
extern "C" bool btStop();

// Check if a GPIO pin should be actively managed (not isolated)
static bool is_active_pin(uint8_t pin)
{
    for (size_t i = 0; i < ACTIVE_PINS_COUNT; i++)
    {
        if (ACTIVE_PINS[i] == pin)
        {
            return true;
        }
    }
    return false;
}

void power_mgmt_post_wakeup(void *context)
{
    // Context parameter reserved for future use
    (void)context; // Suppress unused parameter warning

    Serial.println(F("Power management: post-wakeup initialization"));

    // Release GPIO hold on peripheral power pin
    // This allows normal GPIO control after waking from deep sleep
    gpio_num_t periph_power_gpio = (gpio_num_t)PERIPHERAL_POWER_PIN;
    esp_err_t err = gpio_hold_dis(periph_power_gpio);

    if (err == ESP_OK)
    {
        Serial.print(F("Power management: Released GPIO hold on pin "));
        Serial.println(PERIPHERAL_POWER_PIN);
    }
    else
    {
        Serial.print(F("Power management: Warning - failed to release GPIO hold on pin "));
        Serial.print(PERIPHERAL_POWER_PIN);
        Serial.print(F(" (error: "));
        Serial.print(err);
        Serial.println(F(")"));
    }

    Serial.println(F("Power management: post-wakeup complete"));
}

static void isolate_uart_usb_pins()
{
    // Flush serial output before isolating UART pins to prevent corruption
    Serial.println(F("Power management: Flushing serial output..."));
    Serial.flush();
    delay(10);  // Small delay to ensure serial transmission completes

    // Isolate UART pins to prevent floating during sleep
    pinMode(UART_TX_PIN, INPUT_PULLDOWN);
    pinMode(UART_RX_PIN, INPUT_PULLDOWN);

    // Isolate USB pins (USB already disabled via btStop(), but prevent floating)
    pinMode(USB_DM_PIN, INPUT_PULLDOWN);
    pinMode(USB_DP_PIN, INPUT_PULLDOWN);

    // Note: Serial output unavailable after this point due to pin isolation
}

static void isolate_i2c_pins()
{
    // After Wire.end() is called, I2C pins should be isolated to prevent floating
    // I2C pins are in ACTIVE_PINS during normal operation but need isolation for sleep
    pinMode(I2C_SDA_PIN, INPUT_PULLDOWN);
    pinMode(I2C_SCL_PIN, INPUT_PULLDOWN);
}

static void isolate_unused_gpios()
{
    // Isolate all other GPIOs not actively used
    // This prevents floating inputs from causing current leakage

    Serial.println(F("Power management: Isolating unused GPIO pins..."));

    int isolated_count = 0;

    // ESP32-C6 has GPIOs 0-30, ESP32-S3 has GPIOs 0-48
    // We'll iterate through a safe range and skip active pins
    #if defined(BOARD_TYPE_ESP32C6)
        const uint8_t MAX_GPIO = 30;
    #elif defined(BOARD_TYPE_ESP32S3)
        const uint8_t MAX_GPIO = 48;
    #else
        const uint8_t MAX_GPIO = 39;  // Safe fallback
    #endif

    for (uint8_t pin = 0; pin <= MAX_GPIO; pin++)
    {
        // Skip pins that are actively used or already handled
        if (is_active_pin(pin))
        {
            continue;
        }

        // Skip UART and USB pins (handled separately above)
        if (pin == UART_TX_PIN || pin == UART_RX_PIN ||
            pin == USB_DM_PIN || pin == USB_DP_PIN)
        {
            continue;
        }

        // Set unused GPIO to INPUT_PULLDOWN to prevent floating
        pinMode(pin, INPUT_PULLDOWN);
        isolated_count++;
    }

    Serial.print(F("Power management: Isolated "));
    Serial.print(isolated_count);
    Serial.println(F(" unused GPIO pins"));
}

static void enable_peripheral_power_hold()
{
    // Enable GPIO hold on peripheral power pin to maintain OFF state during sleep
    // This is critical - without hold, the pin may float and peripherals could power on

    gpio_num_t periph_power_gpio = (gpio_num_t)PERIPHERAL_POWER_PIN;
    esp_err_t err = gpio_hold_en(periph_power_gpio);

    if (err == ESP_OK)
    {
        Serial.print(F("Power management: Enabled GPIO hold on peripheral power pin "));
        Serial.println(PERIPHERAL_POWER_PIN);

        #if PERIPHERAL_POWER_INVERTED
            Serial.println(F("Power management: Peripheral power will remain OFF (inverted logic, HIGH held)"));
        #else
            Serial.println(F("Power management: Peripheral power will remain OFF (normal logic, LOW held)"));
        #endif
    }
    else
    {
        Serial.print(F("Power management: ERROR - failed to enable GPIO hold on pin "));
        Serial.print(PERIPHERAL_POWER_PIN);
        Serial.print(F(" (error: "));
        Serial.print(err);
        Serial.println(F(")"));
        Serial.println(F("Power management: WARNING - Peripherals may draw current during sleep!"));
    }
}

void power_mgmt_prep_sleep(void *context)
{
    // Context parameter reserved for future use
    (void)context; // Suppress unused parameter warning

    Serial.println(F("Power management: preparing for deep sleep..."));

    // Step 1: Shut down Bluetooth radio
    Serial.println(F("Power management: Shutting down Bluetooth..."));
    btStop();
    delay(100); // Allow time for Bluetooth to shutdown

    // Step 2: Configure ESP32 power domains for deep sleep
    Serial.println(F("Power management: Configuring ESP32 power domains..."));
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);

    // Step 3: Isolate board LED pin
    #ifdef BOARD_LED_PIN
    pinMode(BOARD_LED_PIN, INPUT_PULLDOWN);
    Serial.print(F("Power management: Board LED pin "));
    Serial.print(BOARD_LED_PIN);
    Serial.println(F(" set to INPUT_PULLDOWN"));
    #endif

    // Step 4: Isolate I2C pins (after Wire.end() has been called)
    Serial.println(F("Power management: Isolating I2C pins..."));
    isolate_i2c_pins();

    // Step 5: Isolate all other unused GPIOs
    isolate_unused_gpios();

    // Step 6: Enable GPIO hold on peripheral power pin
    // This must be done AFTER peripheral_power_off() has been called
    // Note: GPIO hold may not work on non-RTC GPIOs during deep sleep
    enable_peripheral_power_hold();

    // Step 7: Isolate UART and USB pins (must be last - disables Serial output)
    isolate_uart_usb_pins();

    // Serial output unavailable after this point
}

void power_mgmt_emergency_sleep()
{
    // Emergency sleep is the same as normal sleep preparation
    // Just provides a separate entry point for clarity in error paths
    Serial.println(F("Power management: EMERGENCY sleep preparation"));
    power_mgmt_prep_sleep(NULL);
}
