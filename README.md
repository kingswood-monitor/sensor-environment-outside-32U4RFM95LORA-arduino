# Kingswood LoRa transmitter firmware

This is the firmware for the Kingswood LoRa transmitter. 

Runs on a feather32u4 RFM9X under arduino. 

Reads temperature and humidity data from a DH22.
Constructs a JSON object from the data, e.g.

```
{"sensor":"greenhouse","temperature":3.3,"humdity":99.9,"voltage":3.764062}
```

Implements sleep functionality to conserve battery power.