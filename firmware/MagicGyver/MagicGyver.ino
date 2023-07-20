// НАСТРОЙКИ В ФАЙЛЕ config.h !!!

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "config.h"
#include "stepper.h"
#include "hub.h"

void setup() {
    data_init();
    step_init();

    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(AP_SSID, AP_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");

    hub_init();
}

void loop() {
    hub_tick();
}