/* feather32u4_rfm9x-arduino-lora433TX
 * 
 * Firmware for the LoRa transmitter. 
 * Reads sensor data and battery voltage, packages as JSON string, and transmits.
 *  
 * NOTE: Implements 'sleep' function between data transmission, which disables the USB serial line.
 * Reset the device before flashing.
 */

#include <SPI.h>
#include <LoRa.h>
#include <SHT1x.h>
#include <Ticker.h> // https://github.com/sstaub/Ticker.git

#include "main.h"
#include "utils.h"
#include <Adafruit_SleepyDog.h>
#include <ArduinoJson.h>

// set true to sleep between transmissions to conserve battery
#define SLEEP_MODE false

// Create the empty JSON document - https://arduinojson.org/v6/assistant/
const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + 170;
DynamicJsonDocument doc(capacity); // TODO: Make statis
JsonObject sensors = doc.createNestedObject("sensors");
JsonObject sensors_SHT15 = sensors.createNestedObject("SHT15");
JsonObject status = doc.createNestedObject("status");
JsonObject status_lora = status.createNestedObject("lora");

// SHT15 temperature/humidity sensor
SHT1x sht1x(SHT15dataPin, SHT15clockPin);

// packet id
unsigned int packetID;

void setup()
{
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH); // inverted = 'OFF'

  int lamp_colour = (SLEEP_MODE) ? OFF : RED;
  setLedColour(lamp_colour);

  Serial.begin(115200);
  delay(1000);

  // print the firmware banner information
  printBanner(FIRMWARE_FILENAME, FIRMWARE_VERSION, DEVICE_ID);

  // initialise hardware
  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.setPins(NSS, NRESET, DIO0);

  // start LoRa
  if (!LoRa.begin(433E6))
  {
    Serial.println("Starting LoRa failed.");
    while (1)
      ;
  }

  // misc.
  packetID = 0;
}

void loop()
{
  // code resumes here on wake.
  digitalWrite(LED_BUILTIN, HIGH);

  // get battery voltage
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage

  ++packetID;

  // serialise the JSON for transmission
  char serialData[255];

  doc["deviceID"] = DEVICE_ID;
  doc["packetID"] = packetID;
  sensors_SHT15["temperature"] = sht1x.readTemperatureC();
  sensors_SHT15["humidity"] = sht1x.readHumidity();
  status_lora["packetRssi"] = LoRa.packetRssi();
  status_lora["packetSnr"] = LoRa.packetSnr();
  status_lora["packetFrequencyError"] = LoRa.packetFrequencyError();
  status["batteryVoltage"] = measuredvbat;
  status["info"] = "OK";

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
  digitalWrite(LED_BUILTIN, LOW); // inverted = 'ON'
  flash.start();

  // log to serial port
  Serial.print("TX Packet: ");
  Serial.println(serialData);

  // delay(50);                      // Lets the light flash to show transmission
  digitalWrite(LED_BUILTIN, LOW); // show we're asleep

  // update timers
  flash.update();

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

void flashCB()
{
  digitalWrite(LED_BUILTIN, HIGH); // inverted = 'OFF'
}
