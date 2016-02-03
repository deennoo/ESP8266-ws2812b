# ESP8266-ws2812b
Driving ws2812b led strip true ESP8266


This sketch goal is : 

Drive a WS2812B led strip true an ESP8266 chip and make it compatible with Milight protocole

Actually work on a wemos mini d1, and 15 pixels led strip

This creat a html server who can change led strip color 

to acces  : http://ip shown on ide serial monitor

Actually : 

- 18 colors display
- 18 colors to choose on the http server who can be selected tru direct http call : http://esp8266ip/color


TODO : 

- get a smoothy butter color change/fade
- put some effect and add them to http
- get AP mode if can't connect to selected wifi
- get a config web page for : wifi / pixel number / data pin / turnoff esp led
- add Milight compatibility (act as a bridge and a bulb)


Base on works of : 

https://github.com/sticilface/Esp8266-Hue
and
http://pance.mk/index.php/wifi-light-with-esp8266-and-arduino-code/

Compile with : 
Aduino ide 1.6.5
esp8266 ide environnement 2.1.0 rc2
Neopixelbus animator lib
