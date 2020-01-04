#include <Arduino.h>
#include "utils.h"

void printBanner(const char *firmware_filename, float firmware_version, const char *deviceID)
{
    Serial.println();
    Serial.println("======================================================================");
    Serial.print("# Kingswood LoRa / WiFi Bridge v.");

    Serial.println(firmware_version);
    Serial.println("----------------------------------------------------------------------");
    Serial.print("Firmware filename   : ");
    Serial.println(firmware_filename);
    Serial.print("Device ID           : ");
    Serial.println(deviceID);
    Serial.println("======================================================================");
    Serial.println();
}

void setLedColour(int colour)
{
    switch (colour)
    {
    case RED:
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(BLUE_LED, LOW);
        break;

    case BLUE:
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(BLUE_LED, HIGH);
        break;

    case GREEN:
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, LOW);
        break;

    case OFF:
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(BLUE_LED, LOW);
        break;
    }
}