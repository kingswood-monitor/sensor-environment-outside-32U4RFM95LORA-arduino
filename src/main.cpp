/* Kingswood Monitor Environment Sensor (LoRa)
 * 
 * Firmware for a LoRa based sensor
 * Reads sensor data and battery voltage
 * Package as JSON strind
 * Transmits
 *  
 * NOTE: 
 * Implements 'sleep' function between data transmission, which disables the USB serial line.
 * Reset the device before flashing.
 * 
 *  JSON Format V1.0
 
    packetID = 1234
    protocol n = 1.1

    device
      ID = ESP8266-001
      type = ESP8266
      location = livingroom
      firmware n = 1.1 
      os = mongoose
      battery
        active b = true
        voltage n = 3.8
      lora
        RSSI n = -98
        SNR n = 23
        frequencyError n = 12234

    measurement
      temperature n = 21.1
      humidity n = 87
      co2 = 567
      lux n = 600
      mbars n = 1023
        
    status 
      message = OK
      description = All's well
 * 
 */

#include <SPI.h>
#include <LoRa.h>
#include <SHT1x.h>
#include <Ticker.h> // https://github.com/sstaub/Ticker.git
#include <Adafruit_SleepyDog.h>
#include <ArduinoJson.h>

#include "sensor-utils.h"

#define DEBUG true // set false to suppress debug info on Serial

// Firmware info
#define FIRMWARE_TITLE "Environment Sensor (Outside)"
#define FIRMWARE_NAME "sensor-environment-outside-32U4RFM95LORA-arduino"
#define FIRMWARE_VERSION "1.4"

// JSON protocol version
#define JSON_PROTOCOL_VERSION "1.0"

// device info
#define DEVICE_TYPE "32U4RFM95LORA"
#define DEVICE_ID "001"
#define DEVICE_OS "arduino"
#define DEVICE_LOCATION "Outside"

// battery info
#define BATTERY_ACTIVE true

// pin assignments
#define LED_BUILTIN 13 // red onboard LED
#define VBATPIN A9     // for measuring battery voltage
#define SHT15dataPin A4
#define SHT15clockPin A5
#define DH22_DATA_PIN 18 // A0

// feather32u4 LoRa pin assignment
#define NSS 8    // New Slave Select pin
#define NRESET 4 // Reset pin
#define DIO0 7   // DIO0 pin

// set true to sleep between transmissions to conserve battery
#define SLEEP_MODE false
// number of seconds between transmissions
#define SLEEP_SECONDS 3

const size_t capacity = 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 2 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(7);
StaticJsonDocument<capacity> doc;

JsonObject device = doc.createNestedObject("device");
JsonObject device_battery = device.createNestedObject("battery");
JsonObject device_lora = device.createNestedObject("lora");
JsonObject measurement = doc.createNestedObject("measurement");
JsonObject status = doc.createNestedObject("status");

// Initialise sensors
SHT1x sht1x(SHT15dataPin, SHT15clockPin);

unsigned int packetID;

void setup()
{
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.setPins(NSS, NRESET, DIO0);
  digitalWrite(LED_BUILTIN, LOW);

  int lamp_colour = (SLEEP_MODE) ? OFF : RED;
  utils::setLedColour(lamp_colour);

  Serial.begin(115200);
  delay(2000);

  utils::printBanner(FIRMWARE_TITLE, FIRMWARE_VERSION, DEVICE_OS, FIRMWARE_NAME, JSON_PROTOCOL_VERSION, DEVICE_ID);

  if (!LoRa.begin(433E6))
  {
    Serial.println("Starting LoRa failed.");
    while (1)
      ;
  }

  packetID = 0;
}

void loop()
{
  // code resumes here on wake.

  // serialise the JSON for transmission
  char serialData[255];

  ++packetID;
  doc["packetID"] = packetID;
  doc["protocol"] = JSON_PROTOCOL_VERSION;

  // device
  device["id"] = DEVICE_ID;
  device["type"] = DEVICE_TYPE;
  device["location"] = DEVICE_LOCATION;
  device["firmware"] = FIRMWARE_VERSION;
  device["os"] = DEVICE_OS;

  // battery
  device_battery["active"] = BATTERY_ACTIVE;
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  device_battery["voltage"] = 3.8;

  // lora - NULL for sending device
  device_lora["RSSI"] = nullptr;
  device_lora["SNR"] = nullptr;
  device_lora["frequencyError"] = nullptr;

  // sensors
  measurement["temperature"] = sht1x.readTemperatureC();
  measurement["humidity"] = sht1x.readHumidity();
  measurement["co2"] = nullptr;
  measurement["lux"] = nullptr;
  measurement["mbars"] = nullptr;

  // status
  status["message"] = "OK";
  status["description"] = nullptr;

  serializeJson(doc, serialData);

  // send in async / non-blocking mode
  while (LoRa.beginPacket() == 0)
  {
    delay(100);
  }
  LoRa.beginPacket();
  LoRa.print(serialData);
  LoRa.endPacket(true); // true = async / non-blocking mode

  // signal transmission
  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);

  // log to serial port
  Serial.println("TX Packet: ");
  if (DEBUG)
  {
    serializeJsonPretty(doc, Serial);
    Serial.println();
    Serial.println("---");
  }

  // sleep
  if (SLEEP_MODE)
  {
    Watchdog.sleep(SLEEP_SECONDS * 1000);
  }
  else
  {
    delay(SLEEP_SECONDS * 1000);
  }
}