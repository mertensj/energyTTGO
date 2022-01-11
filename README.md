# energyTTGO
TTGO T-display to visualize energy consumption as broadcasted by the P1 meter

More info on P1 meter can be found on :
 https://www.homewizard.nl/energy

 API HomeWizard WiFi P1 Meter
 https://homewizard-energy-api.readthedocs.io/

 The /api/v1/data endpoint always returns the most recent measurement.
 The update frequency depends on the device and, in case of the HWE-P1,
 the smart meter that it is connected to.
 With a SMR 5.0 meter, this is every second for power and every 5 minutes for gas.

### Arduino Setup:
- add Arduino -> Preferences -> Additional Board Manager URL:<br>
  https://dl.espressif.com/dl/package_esp32_index.json <br>
- Board Manager:<br>
  search for esp32 Espressif Systems<br>
  -> Install
- Select the correct board<br>
  -> Tools -> Boards -> ESP32 Arduino -> TTGO LoRa32-OLED V1
   
### LIBs & Code References:
- TimeLib Library : install via Arduino Library Manager<br>
  Searching for the word "timekeeping" will show the correct library.
  https://github.com/PaulStoffregen/Time/issues/101#issuecomment-791986472
  https://github.com/geekfactory/TimeLib
- ArduinoJson : install via Arduino Library Manager<br>
  https://github.com/bblanchon/ArduinoJson
  

### Shopping list:
Part|Price|Qtd.|Url
---|---|---|---
TTGO T-Display V1.0 ESP32 |€ 13.4|1|https://nl.banggood.com/TTGO-T-Display-ESP32-CP2104-CH340K-CH9102F-WiFi-bluetooth-Module-1_14-Inch-LCD-Development-Board-LILYGO-for-Arduino-products-that-work-with-official-Arduino-boards-p-1522925.html
