/* feather32u4_rfm9x-arduino-lora433TX
 * 
 * Firmware for the LoRa transmitter. 
 * Reads sensor data and battery voltage, packages as JSON string, and transmits.
 * 
 * Typical JSON structure:
 * {
 *  "deviceID": "FEATHE32U4RFM9XLORA001",
 *  "sensors": {
 *     "DH22": {"temperature": 21.2, "humidity": 99.8 },
 *     "battery": {"voltage": 3.7}
 *  }
 * }
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
#define FIMWARE_VERSION 1.1

// sensor type for firmware message
#define SENSOR_TYPE "DH22"

// device ID for data
#define DEVICE_ID "FEATHE32U4RFM9XLORA_001"

// number of seconds between transmissions
#define SLEEP_SECONDS 3

// pin assignments
#define LED_BUILTIN 13
#define VBATPIN A9
#define DH22_DATA_PIN 18 // A0

// for feather32u4
#define NSS 8    // New Slave Select pin
#define NRESET 4 // Reset pin
#define DIO0 7   // DIO0 pin

// ArduinoJSON document
const size_t capacity = JSON_OBJECT_SIZE(1) + 3 * JSON_OBJECT_SIZE(2);
DynamicJsonDocument doc(capacity);

JsonObject sensors = doc.createNestedObject("sensors");
// add sensors as required and adjust 'capacity': see https://arduinojson.org/v6/assistant/
JsonObject sensors_DH22 = sensors.createNestedObject("DH22");
JsonObject sensors_battery = sensors.createNestedObject("battery");

// DH22 temperature/humidity sensor
DHT dht;

void setup()
{
  // set deviceID in JSON
  doc["deviceID"] = DEVICE_ID;

  // initialise hardware
  pinMode(LED_BUILTIN, OUTPUT);
  dht.setup(DH22_DATA_PIN);
  LoRa.setPins(NSS, NRESET, DIO0);
  Serial.begin(115200);
  while (!Serial)
  {
    delay(1);
  }

  // print the firmware banner information
  Serial.println("===========================================================");
  Serial.println();
  Serial.print("Kingswood LoRa Transmitter v.");
  Serial.println(FIMWARE_VERSION);
  Serial.println();
  Serial.print("Firmware filename   : ");
  Serial.println(FIRMWARE_FILENAME);
  Serial.print("Sensor ID           : ");
  Serial.println(DEVICE_ID);

  // Enumerate the sensors
  Serial.print("Sensors             : ");
  for (JsonObject::iterator it = sensors.begin(); it != sensors.end(); ++it)
  {
    Serial.print(it->key().c_str());
    Serial.print(", ");
  }
  Serial.println();

  Serial.print("Sample interval (s) : ");
  Serial.println(SLEEP_SECONDS);
  Serial.println();
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

  // sleep
  digitalWrite(LED_BUILTIN, LOW); // show we're asleep
  int sleepMS = Watchdog.sleep(SLEEP_SECONDS * 1000);

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

  // build the sensor data JSON
  sensors_DH22["temperature"] = temperature;
  sensors_DH22["humidity"] = humidity;
  sensors_battery["voltage"] = measuredvbat;

  // serialise the JSON for transmission
  char data[256];
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

  // try and reconnect the USB after sleep (NOTE: doesn't seem to work)

#if defined(USBCON) && !defined(USE_TINYUSB)
  USBDevice.attach();
#endif
}