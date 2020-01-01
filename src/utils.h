#ifndef UTILS_H
#define UTILS_H

// print firmware details to Serial
void printBanner(const char *firmware_filename, float firmware_version, const char *deviceID);

// construct the device ID from the chip ID e.g. "HUZZAHESP8266-2DC832"
const char *deviceID(const char *type);

#endif