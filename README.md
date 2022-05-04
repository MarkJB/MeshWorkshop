# MeshWorkshop

Some examples of using painlessMesh to send and receive sensor data and react to that sensor data.

Need Arduino software: https://www.arduino.cc/en/software 

Need to add Espressif boards to the Arduino preferences. Copy the following github url into the 'Additional Boards Managers URLs' text field: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json (Note: If using ESP8266 you will need this boards file https://arduino.esp8266.com/stable/package_esp8266com_index.json) 

Go to 'Tools->Board->Boards Manager` search for Esp and install the latest Espressif package (~150MB) 

Go to 'Sketch->Include Library->Manage Library` and search for 'Painless Mesh' and install the latest. 

Add the library to your sketch with either #include <painlessMesh.h> or `Sketch->Include Library->Painless Mesh` (which will add the same include text previously mentioned). 

 

To open the examples, make sure you have selected an ESP32/ESP8266 compatible board ('Tools->Board->ESP32 Arduino'). Then you can open the examples from 'File->Examples->Painless Mesh` 

 

Note: If compiling fails, complaining about AsyncTCP.h: No such file or directory, then you can get those files from here: https://github.com/me-no-dev/AsyncTCP, then in the Arduino IDE go to "Sketch -> Include Library -> Add .ZIP Library..." and select the downloaded zip file or directory.

 

Similarly if it complains about ESPAsyncTCP.h then you can get the files from here: https://github.com/me-no-dev/ESPAsyncTCP and add this as a library as well.

If using Servos on the ESP32, you will need to install the ESP32Servo library. Go to "Sketch -> Include Library -> ESP32Servo" which will add a bunch of librarys to include at the top of the file (you might have to remove tone.h or the compile may fail)

For a good reference guide for Arduino ESP32 pinout and pin uses see this: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
