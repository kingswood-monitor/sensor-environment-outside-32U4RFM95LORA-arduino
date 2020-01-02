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
#include <DHT.h>

#include "main.h"
#include "utils.h"
#include <Adafruit_SleepyDog.h>
#include <ArduinoJson.h>

// Create the empty JSON document
const size_t capacity = JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3);
DynamicJsonDocument doc(capacity);
JsonObject sensors = doc.createNestedObject("sensors");
JsonObject sensors_DH22 = sensors.createNestedObject("DH22");
JsonObject sensors_battery = sensors.createNestedObject("battery");

// serialise the JSON for transmission
void toJsonString(const char *deviceID, const int packetID, const float temperature, const float humidity, const float voltage, char *serialData, int length);

// DH22 temperature/humidity sensor
DHT dht;

// packet id
unsigned int packetID;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(1);
  }

  // print the firmware banner information
  printBanner(FIRMWARE_FILENAME, FIRMWARE_VERSION, DEVICE_ID);

  // initialise hardware
  pinMode(LED_BUILTIN, OUTPUT);
  dht.setup(DH22_DATA_PIN);
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

  // read the sensor
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();

  // get battery voltage
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage

  ++packetID;

  // serialise the JSON for transmission
  char serialData[255];
  toJsonString(DEVICE_ID, packetID, temperature, humidity, measuredvbat, serialData, sizeof(serialData) - 1);

  // send in async / non-blocking mode
  while (LoRa.beginPacket() == 0)
  {
    delay(100);
  }

  LoRa.beginPacket();
  LoRa.print(serialData);
  LoRa.endPacket(true); // true = async / non-blocking mode

  // log to serial port
  Serial.print("TX Packet: ");
  Serial.println(serialData);

  // delay(50);                      // Lets the light flash to show transmission
  digitalWrite(LED_BUILTIN, LOW); // show we're asleep

  // sleep
  // Watchdog.sleep(SLEEP_SECONDS * 1000);
  delay(SLEEP_SECONDS * 1000);
}

// serialise the JSON document from the data
// TODO: FIX RETURNING ADDRESS OF LOCAL VARIABLE

void toJsonString(const char *deviceID, const int packetID, const float temperature, const float humidity, const float voltage, char *serialData, int length)
{
  doc["deviceID"] = deviceID;
  doc["packetID"] = packetID;
  sensors_DH22["temperature"] = temperature;
  sensors_DH22["humidity"] = humidity;
  sensors_battery["voltage"] = voltage;

  serializeJson(doc, serialData, length);
}