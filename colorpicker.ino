//sources : http://www.esp8266.com/viewtopic.php?f=29&t=3470&start=36
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <NeoPixelBus.h>

#define pixelCount 4            //number of pixels in RGB strip

NeoPixelBus strip = NeoPixelBus(pixelCount, 0);  //GPIO 0

const char* ssid     = "MY_ROUTER";
const char* password = "MY_PASSWD";

IPAddress apIP(192, 168, 1, 1);        //FOR AP mode
IPAddress netMsk(255,255,255,0);         //FOR AP mode

const char* html = "<html><head><style></style></head>"
                   "<body><form action='/' method='GET'><input type='color' name='color' value='#000000'/>"
                   "<input type='submit' name='submit' value='Update RGB Strip'/></form></body></html>";

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  strip.Begin();   
  strip.Show();

//***************WIFI client************************//
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rWorking to connect");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
//***************WIFI ACCESS POINT******************//
/*  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP,apIP,netMsk);
  WiFi.softAP(ssid);//,password);  //leave password away for open AP
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());   */
//**************************************************//

 
  server.on("/", handle_root);             //root page
  server.onNotFound(handleNotFound);       //page if not found
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleNotFound() {
  Serial.print("\t\t\t\t URI Not Found: ");
  Serial.println(server.uri());
  server.send ( 200,"text/plain","URI Not Found" );
}

void handle_root() {
  String toSend = html;
  if (server.hasArg("color")) {
    String rgbStr = server.arg("color");  //get value from html5 color element
    rgbStr.replace("%23","#"); //%23 = # in URI
    toSend.replace("#000000",rgbStr);     //replace our default black with new color, so when page loads our new color shows
    int rgb[3];                           //define rgb pointer for ws2812
    getRGB(rgbStr,rgb);                   //convert RGB string to rgb ints
    updateEntireStrip(rgb[0], rgb[1], rgb[2]);           //update strip
  }
  server.send(200, "text/html", toSend);
  delay(100);
}

void getRGB(String hexRGB, int *rgb) {
  hexRGB.toUpperCase();
  char c[7]; 
  hexRGB.toCharArray(c,8);
  rgb[0] = convertToInt(c[1],c[2]); //red
  rgb[1] = convertToInt(c[3],c[4]); //green
  rgb[2] = convertToInt(c[5],c[6]); //blue  
}

int convertToInt(char upper,char lower)
{
  int uVal = (int)upper;
  int lVal = (int)lower;
  uVal = uVal >64 ? uVal - 55 : uVal - 48;
  uVal = uVal << 4;
  lVal = lVal >64 ? lVal - 55 : lVal - 48;
  return uVal + lVal;
}

void updateEntireStrip(int red, int blue, int green) {
  RgbColor rgb = RgbColor(red, blue, green);     
  for(int i=0; i < pixelCount; i++) {
    strip.SetPixelColor(i,rgb);
  }
  strip.Show();
}

  
