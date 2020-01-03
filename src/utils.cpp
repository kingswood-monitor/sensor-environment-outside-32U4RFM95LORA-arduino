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
    Serial.print("Sensor ID           : ");
    Serial.println(deviceID);
    Serial.println("======================================================================");
    Serial.println();
}

const char *deviceID(const char *type)
{
    // char buf[7];
    // sprintf(buf, "%lX", ESP.getChipId());

    // char *deviceID;
    // deviceID = (char *)malloc(strlen(type) + 8);

    // strcpy(deviceID, type);
    // strcat(deviceID, "-");
    // strcat(deviceID, buf);

    const char *deviceID = "FEATHER32U4LORA-001";

    return deviceID;
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