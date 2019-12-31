/*
 * feather32u4_rfm9x-arduino-lora433TX
 * 
 * Firmware for the LoRa transmitter. 
 * Reads SH22 temperature and pressure, packages as JSON string, and transmits.
 * 
 * NOTE: Implements 'sleep' function between data transmission, which disables the USB serial line.
 * Reset the device before flashing.
 */

#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <Adafruit_SleepyDog.h>

#define FIRMWARE_FILENAME "feather32u4_rfm9x-arduino-lora433TX"
#define FIMWARE_VERSION 1.0

// sensor type for firmware message
#define SENSOR_TYPE "DH22 temperature/humidity"

// device ID for data
#define DEVICE_ID "greenhouse"

// number of seconds between transmissions
#define SLEEP_SECONDS 10

// pin assignments
#define LED_BUILTIN 13
#define VBATPIN A9
#define DH22_DATA_PIN 18 // A0

// for feather32u4
#define NSS 8    // New Slave Select pin
#define NRESET 4 // Reset pin
#define DIO0 7   // DIO0 pin

// ArduinoJSON document
const int capacity = JSON_OBJECT_SIZE(4);
StaticJsonDocument<capacity> doc;

// DH22 temperature/humidity sensor
DHT dht;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  dht.setup(DH22_DATA_PIN);

  LoRa.setPins(NSS, NRESET, DIO0);

  Serial.begin(115200);
  while (!Serial)
  {
    delay(1);
  }

  Serial.println("===========================================================");
  Serial.println();
  Serial.print("Kingswood LoRa Receiver v.");
  Serial.println(FIMWARE_VERSION);
  Serial.println();
  Serial.print("Firmware filename   : ");
  Serial.println(FIRMWARE_FILENAME);
  Serial.print("Sensor type         : ");
  Serial.println(SENSOR_TYPE);
  Serial.print("Sensor ID           : ");
  Serial.println(DEVICE_ID);
  Serial.print("Sample interval (s) : ");
  Serial.println(SLEEP_SECONDS);
  Serial.println("===========================================================");
  Serial.println();

  if (!LoRa.begin(433E6))
  {
    Serial.println("Starting LoRa failed.");
    while (1)
      ;
  }
}

void loop()
{

  digitalWrite(LED_BUILTIN, LOW); // show we're asleep
  int sleepMS = Watchdog.sleep(SLEEP_SECONDS * 1000);

  // Code resumes here on wake.
  digitalWrite(LED_BUILTIN, HIGH);

  // Read the sensor
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();

  // Get battery voltage
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage

  // Build the sensor data JSON
  doc["deviceID"] = DEVICE_ID;
  doc["temperature"] = temperature;
  doc["humdity"] = humidity;
  doc["voltage"] = measuredvbat;
  char data[128];
  serializeJson(doc, data);

  // wait until the radio is ready to send a packet
  while (LoRa.beginPacket() == 0)
  {
    delay(100);
  }

  // send in async / non-blocking mode
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket(true); // true = async / non-blocking mode

  delay(100); // Lets the light flash to show transmission

// try and reconnect the USB after sleepo: doesn't seem to work
#if defined(USBCON) && !defined(USE_TINYUSB)
  USBDevice.attach();
#endif
}