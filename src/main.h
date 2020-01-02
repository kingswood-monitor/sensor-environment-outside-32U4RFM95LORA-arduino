#ifndef MAIN_H
#define MAIN_H

// Firmware info
#define FIRMWARE_FILENAME "feather32u4_rfm9x-arduino-lora433TX"
#define FIRMWARE_VERSION 1.1
#define DEVICE_ID "32U4RFM95LORA-001"

// pin assignments
#define LED_BUILTIN 13   // red onboard LED
#define VBATPIN A9       // for measuring battery voltage
#define DH22_DATA_PIN 18 // A0

// feather32u4 LoRa pin assignment
#define NSS 8    // New Slave Select pin
#define NRESET 4 // Reset pin
#define DIO0 7   // DIO0 pin

// number of seconds between transmissions
#define SLEEP_SECONDS 3

#endif