#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <NeoPixelBus.h>
#define pixelCount 15
#define colorSaturation 255
NeoPixelBus strip = NeoPixelBus(pixelCount, 2, NEO_GRB);
RgbColor red = RgbColor(colorSaturation, 0, 0);
RgbColor green = RgbColor(0, colorSaturation, 0);
RgbColor blue = RgbColor(0, 0, colorSaturation);
RgbColor white = RgbColor(colorSaturation);
RgbColor yellow = RgbColor(255, 255, 0);
RgbColor orange = RgbColor(255, 165, 0);
RgbColor violet = RgbColor(238, 130, 238);
RgbColor royalblue = RgbColor(65, 105, 225);
RgbColor lightskyblue = RgbColor(135, 206, 250);
RgbColor aqua = RgbColor(0, 255, 255);
RgbColor aquamarine = RgbColor(127, 255, 212);
RgbColor seagreen = RgbColor(46, 139, 87);
RgbColor limegreen = RgbColor(50, 205, 50);
RgbColor goldenrod = RgbColor(218, 165, 32);
RgbColor pink = RgbColor(255, 192, 203);
RgbColor fuchsia = RgbColor(255, 0, 255);
RgbColor orchid = RgbColor(218, 112, 214);
RgbColor lavender = RgbColor(230, 230, 250);
RgbColor black = RgbColor(0);

NeoPixelAnimator animator(&strip); // NeoPixel animation management object

RgbColor StripRgbColors[pixelCount]; // Holds all colors of the pixels on the strip even if they are off
bool StripLightIsOn[pixelCount]; // Holds on/off information for all the pixels

const char *ssid = ""; //put your ssid
const char *password = ""; // put your key

MDNSResponder mdns;
ESP8266WebServer server ( 80 );
const int led = 13;
void handleRoot() {
    digitalWrite ( led, 1 );
  String out = "<html><head><title>Wifi light</title></head>";
  out += "<body style='background-color:gray'>";
  out += "<span style='display:block; width:100%; font-size:2em; font-family:Verdana; text-align:center'>Choose color</span><br/>";
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

void infoLight(RgbColor color) {
  // Flash the strip in the selected color. White = booted, green = WLAN connected, red = WLAN could not connect
  for (int i = 0; i < pixelCount; i++)
  {
    strip.SetPixelColor(i, color);
    strip.Show();
    delay(10);
    strip.SetPixelColor(i, black);
    strip.Show();
  }
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
void setup ( void ) {
    pinMode ( led, OUTPUT );
    digitalWrite ( led, 0 );
  strip.Begin();
  strip.Show();

  // Show that the NeoPixels are alive
  delay(120); // Apparently needed to make the first few pixels animate correctly
  infoLight(white);

    Serial.begin ( 115200 );

     WiFi.mode(WIFI_STA);
     WiFi.begin(ssid, password);
     if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        // Show that we are connected
        infoLight(red);
    while (1) delay(100);
  }

    WiFi.begin ( ssid, password );
    Serial.println ( "" );
    // Wait for connection
    while ( WiFi.status() != WL_CONNECTED ) {
        delay ( 500 );
        Serial.print ( "." );
    }
    Serial.println ( "" );
    Serial.print ( "Connected to " );
    Serial.println ( ssid );
    Serial.print ( "IP address: " );
    Serial.println ( WiFi.localIP() );
    if ( mdns.begin ( "esp8266", WiFi.localIP() ) ) {
        Serial.println ( "MDNS responder started" );
        infoLight(green);
       }
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
void setColor(RgbColor color) {
  int i;
  for (i=0; i<pixelCount; i++) {
    strip.SetPixelColor(i, color);
  }
  strip.Show();
}
