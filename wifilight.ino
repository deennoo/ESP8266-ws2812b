#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <NeoPixelBus.h> //ledstrip driver
#include "config.h" //Config web page
#include "colorpicker.h" //Colorpicker webpage
#include "effects.h" //effects Webpage
#include "index.h" //index
#include "wificonfig.h" //wifi st and ap configpage

const char *ssid = ""; //put your ssid between the ""
const char *password = ""; // put your key between the ""

//led strip config
#define pixelCount 15 // your pixel number
#define pixelPin 2 // pixel pin
NeoPixelBus strip = NeoPixelBus(pixelCount, pixelPin, NEO_GRB);
NeoPixelAnimator animator(&strip); // NeoPixel animation management object made run effect and fader

//Color Definition
#define colorSaturation 255
//RgbColor yourcolor = Rgbcolor (redvalue, greenvalue, redvalue);
RgbColor red = RgbColor(colorSaturation, 0, 0);
RgbColor green = RgbColor(0, colorSaturation, 0);
RgbColor blue = RgbColor(0, 0, colorSaturation);
RgbColor white = RgbColor(colorSaturation);
RgbColor yellow = RgbColor(colorSaturation, colorSaturation, 0);
RgbColor orange = RgbColor(colorSaturation, 165, 0);
RgbColor violet = RgbColor(238, 130, 238);
RgbColor royalblue = RgbColor(65, 105, 225);
RgbColor lightskyblue = RgbColor(135, 206, 250);
RgbColor aqua = RgbColor(0, colorSaturation, colorSaturation);
RgbColor aquamarine = RgbColor(127, 255, 212);
RgbColor seagreen = RgbColor(46, 139, 87);
RgbColor limegreen = RgbColor(50, 205, 50);
RgbColor goldenrod = RgbColor(218, 165, 32);
RgbColor pink = RgbColor(255, 192, 203);
RgbColor fuchsia = RgbColor(colorSaturation, 0, colorSaturation);
RgbColor orchid = RgbColor(218, 112, 214);
RgbColor lavender = RgbColor(230, 230, 250);
RgbColor black = RgbColor(0);


// Flash the strip in the selected color. White = booted, green = WLAN connected, red = WLAN could not connect
void infoLight(RgbColor color) {
    for (int i = 0; i < pixelCount; i++){
    strip.SetPixelColor(i, color);
    strip.Show();
    delay(10);
    strip.SetPixelColor(i, black);
    strip.Show();
  }
}


//Webserver
MDNSResponder mdns;
ESP8266WebServer server ( 80 );
const int led = 13;
void handleRoot() {
    digitalWrite ( led, 1 );
  String out = "<html><head><title>Lampe Wifi</title></head>";
  out += "<body style='background-color:gray'>";
  out += "<span style='display:block; width:100%; font-size:2em; font-family:Verdana; text-align:center'>Choose color</span><br/>";
  out += "<a href='Config'><span style='display:block; width:100%; font-size:2em; font-family:Verdana; text-align:center'>Config</span><br/>";
  out += "<a href='white'><span style='display:block; background-color:white; width:100%; height:6em;'></span></a><br/>";
  out += "<a href='red'><span style='display:block; background-color:red; width:100%; height:6em;'></span></a><br/>";
  out += "<a href='green'><span style='display:block; background-color:green; width:100%; height:6em;'></span></a><br/>";
  out += "<a href='blue'><span style='display:block; background-color:blue; width:100%; height:6em;'></span></a><br/>";
  out += "<a href='yellow'><span style='display:block; background-color:yellow; width:100%; height:6em;'></span></br>";
  out += "<a href='orange'><span style='display:block; background-color:orange; width:100%; height:6em;'></span></br>";
  out += "<a href='violet'><span style='display:block; background-color:violet; width:100%; height:6em;'></span></br>";
  out += "<a href='royalblue'><span style='display:block; background-color:royalblue; width:100%; height:6em;'></span></br>";
  out += "<a href='lightskyblue'><span style='display:block; background-color:lightskyblue; width:100%; height:6em;'></span></br>";
  out += "<a href='aqua'><span style='display:block; background-color:aqua; width:100%; height:6em;'></span></br>";
  out += "<a href='aquamarine'><span style='display:block; background-color:aquamarine; width:100%; height:6em;'></span></br>";
  out += "<a href='seagreen'><span style='display:block; background-color:seagreen; width:100%; height:6em;'></span></br>";
  out += "<a href='limegreen'><span style='display:block; background-color:limegreen; width:100%; height:6em;'></span></br>";
  out += "<a href='goldenrod'><span style='display:block; background-color:goldenrod; width:100%; height:6em;'></span></br>";
  out += "<a href='pink'><span style='display:block; background-color:pink; width:100%; height:6em;'></span></br>";
  out += "<a href='fuchsia'><span style='display:block; background-color:fuchsia; width:100%; height:6em;'></span></br>";
  out += "<a href='orchid'><span style='display:block; background-color:orchid; width:100%; height:6em;'></span></br>";
  out += "<a href='lavender'><span style='display:block; background-color:lavender; width:100%; height:6em;'></span></br>";
  out += "<a href='black'><span style='display:block; background-color:black; width:100%; height:6em;'></span></a>";  
  out += "</body>";
  out += "</html>";
    server.send ( 200, "text/html", out );
    digitalWrite ( led, 0 );
}

void handleNotFound() {
    digitalWrite ( led, 1 );
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for ( uint8_t i = 0; i < server.args(); i++ ) {
        message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
    }
    server.send ( 404, "text/plain", message );
    digitalWrite ( led, 0 );
}

//////////////////Start Working/////////
void setup ( void ) {
//    FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  pinMode ( led, OUTPUT );
  digitalWrite ( led, 0 );
  strip.Begin();
  strip.Show();
//  WiFi.mode(WIFI_AP_STA);
//  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
//  WiFi.softAP("Lampe Wifi://192.168.4.1");    // Create WiFi network
//  WiFi.begin(ssid, password);
//  delay(5000);
//  server.begin(); 


  // Show that the NeoPixels are alive
  delay(120); // Apparently needed to make the first few pixels animate correctly
  infoLight(white);

  Serial.begin ( 115200 );

//WIFI Part
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
  Serial.println("WiFi Failed");
  // Show that we are not connected
  infoLight(red);
  while (1) delay(100);
  }

  WiFi.begin ( ssid, password );
  Serial.println ( "" );
  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      Serial.print ( "." );}
  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );
  if ( mdns.begin ( "esp8266", WiFi.localIP() ) ) {
      Serial.println ( "MDNS responder started" );
  // Show that we are connected
  infoLight(green);}

 //Set action on color choose
    server.on ( "/", []() {handleRoot();} );
    server.on ( "/white", []() {setColor(white); handleRoot();} );
    server.on ( "/red", []() {setColor(red); handleRoot();} );
    server.on ( "/green", []() {setColor(green); handleRoot();} );
    server.on ( "/blue", []() {setColor(blue); handleRoot();} );
    server.on ( "/black", []() {setColor(black); handleRoot();} );
    server.on ( "/yellow", []() {setColor(yellow); handleRoot();} );
    server.on ( "/orange", []() {setColor(orange); handleRoot();} );
    server.on ( "/violet", []() {setColor(violet); handleRoot();} );
    server.on ( "/royalblue", []() {setColor(royalblue); handleRoot();} );
    server.on ( "/lightskyblue", []() {setColor(lightskyblue); handleRoot();} );
    server.on ( "/aqua", []() {setColor(aqua); handleRoot();} );
    server.on ( "/aquamarine", []() {setColor(aquamarine); handleRoot();} );
    server.on ( "/seagreen", []() {setColor(seagreen); handleRoot();} );
    server.on ( "/limegreen", []() {setColor(limegreen); handleRoot();} );
    server.on ( "/goldenrod", []() {setColor(goldenrod); handleRoot();} );
    server.on ( "/pink", []() {setColor(pink); handleRoot();} );
    server.on ( "/fuchsia", []() {setColor(fuchsia); handleRoot();} );
    server.on ( "/orchid", []() {setColor(orchid); handleRoot();} );
    server.on ( "/lavender", []() {setColor(lavender); handleRoot();} ); 
    server.onNotFound ( handleNotFound );
    server.begin();
    Serial.println ( "HTTP server started" );
}

void loop ( void ) {
    mdns.update();
    server.handleClient();
}

//Acting on Led Strip
void setColor(RgbColor color){
  int i;
  for (i=0; i<pixelCount; i++) {
    strip.SetPixelColor(i, color);
  }
  strip.Show();
}
