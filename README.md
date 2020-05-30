# Arduino-MiLightHub_switch
 MilightHub https://github.com/sidoh/esp8266_milight_hub

# What you need
ESP8266 with esp8266_milight_hub (and it's IP ofc.)
[ESP32](https://dl.espressif.com/dl/package_esp32_index.json)
[PCB] (https://easyeda.com/jellewietsma/smart-home-switch)

# How it works
Make sure the esp8266_milight_hub is already set-up and working, and you know it's IP.
Make sure you have an ESP with the PCB, you can also make one yourself but mine is linked in this project (make sure to read the schematics, they should explain themself)
make sure to enable SecondSwitch if you have 2 sets of 4 buttons
Make sure to set MilightLight properly before upload (this can't be done later), set the LightID, mode and group (see definition of 'MilightLightA')

You can either setup the IP and passwords and such in the code, but you can also just upload the sketch and power it on and set it up: 
It will go into APMODE (since it can’t connect to WIFI) connect to it and go to it's IP (192.168.4.1)
You will get a window with SSID Wi-Fi name and password, but also the hub IP, fill these in and submit. 
The ESP will save these settings and reboot (note that the 4 commands to send to the hub are also stored in here) (Leaving fields blank will skip updating them)

# LED feedback
There a lot of blinking patterns, but I tried to list them all here
(Button LED) will blink every 50ms on boot if pressed (should be released on boot)
(Button LED) will turn on when the button is pressed, and only off after it's done (or a connection timeout)

(Main LED) will blink every 10ms if we are less then 1000ms from restarting (super long press)
(Main LED) will blink every 100ms to show we are not connected (APMODE)
(Main LED) will blink every 500ms to show we are trying to connect
(Main LED) will blink every 1000ms to show that OTA (Over The Air update) is on
(Main LED) will turn on on boot, will turn off when done
(Main LED) will turn on when trying to connect to WIFI, will turn off when done
(Main LED) will turn on when a long press (5000ms) is happening, will turn off when done

# Button actions
(Set of 4 buttons pressed on boot) Enable OTA and Settings menu 
(Button short press) execute action; send the data (saved in 'Commands#' which can be hardcoded or be set in SettingsPage) to the milight hub. This is done by default with feedback
It stops after 5 failed tries, or 2 failed connections, or 1 if send Json is wrong
(Button 1 (ID0) long press) Start and enable OTA
(Button 2 (ID1) long press) Enable Settings menu (APMODE) though it's IP on its WIFI (will hide password by default), just reboot ESP when your done to disable it
(Button extreme long press) Hold any button for 10s to reboot the ESP

# Images
![Working GIF](https://github.com/jellewie/Arduino-MiLightHub_switch/blob/master/Untitled%20Project.gif)
![buttons to the wall](https://raw.githubusercontent.com/jellewie/Arduino-MiLightHub_switch/master/3D/Single.jpg)
![Double buttons settup](https://raw.githubusercontent.com/jellewie/Arduino-MiLightHub_switch/master/3D/Double.jpg)
