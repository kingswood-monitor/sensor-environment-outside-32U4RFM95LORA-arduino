#ifndef UTILS_H
#define UTILS_H

// Pin assignments for a 4-wire LED for status indication
#define RED_LED A1 //
#define GREEN_LED A2
#define BLUE_LED A3

#define OFF (0)
#define RED (1)
#define BLUE (2)
#define GREEN (3)

// print firmware details to Serial
void printBanner(const char *firmware_filename, float firmware_version, const char *deviceID);

//
void setLedColour(int colour);

// construct the device ID from the chip ID e.g. "HUZZAHESP8266-2DC832"
const char *deviceID(const char *type);

#endif