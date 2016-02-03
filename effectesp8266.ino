////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//        150KB free Sketch Space 31 KB free Memory and Untouched SPIFFS........
// This is a demonstration on how to use an input device & webpage to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin(use the flash button).  
// When you press the button it will change to a new pixel animation. Note that you need to press the
// button once or trigger from webpage to start the first animation!
// sources :http://www.esp8266.com/viewtopic.php?f=29&t=3470&start=36
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
// WiFi connection
const char* ssid = "ssid";
const char* password = "password";
const char* host = "EM-Pixels";
IPAddress apIP(192, 168, 4, 1);
WiFiServer server(80);
WiFiClient client;
extern "C" 
{
#include "user_interface.h"
}
ADC_MODE(ADC_VCC);     //   !!*!*!*!*! DO NOT USE THIS IF LDR FITTED !*!*!*!*!*!!
int hr,mn,st,st1;                // Uptime Duration Counter......
unsigned long ulReqcount;        // how often has a valid page been requested
unsigned long ulReconncount;     // how often did we connect to WiFi

// The Flash Button fitted to GPIO 0 on most development modules can be used.
#define BUTTON_PIN   0    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.

#define PIXEL_PIN    2    // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 40

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
const int bright = 10;         //select 1 thru 10 ( this is multiplied by 25 to get brightest pixel value (from 0-255)
const int minFlashes = 2;      //inimum number of flashes/cycles of each pattern before generating new number of segments
const int maxFlashes = 8;   
int maxBurst = 4;              // number of pattern bursts before inverting within each flash/cycle
const int maxNumberOfSegments = 8;      // Maximum allowable number of segments to be generated
                                        // this also defines size of node[] array below
const int maxWait = maxNumberOfSegments;//maximum delay() time in milliseconds - related to number of segments

int node[maxNumberOfSegments];      // array to hold segment nodes
int numSegments = 2;         // initial number of segments

const int maxTailSize = 7;      // length of light "tail" including lead light
int tailSize = maxTailSize;      // initial tailsize - this will vary below

const int arraySize = maxTailSize + 1;  // zero-indexed array for holding brightness values of colors below, and extra "0" at end

// Array to hold 'r' (RED) color values - Add more values array to lengthen 'tail' of lights
// Last value should be "0" to ensure pixels are turned off appropriately
// Make sure the length of tail correlates to maxNumberOfSegments (*if segment length can be 5 pixels, don't want 7 pixel tail)
int r[arraySize] = {
25*bright, 20*bright, 10*bright, 4*bright, 2*bright, bright, bright, 0};
// Due to logarithmic nature of the human eye, each additional brightness level should be slightly less than half the previous value for best effect.

// green color values - NOT USED HERE -- WILL be utilized in future multi-color version of sketch
//** Remove in future version!   
// - BE CAREFUL CHANGIONG THESE - Check for loop containing "strip.setPixelColor" first!
int g[arraySize] = {
25*bright, 20*bright, 10*bright, 4*bright, 2*bright, bright, bright, 0};
// blue color values
int b[arraySize] = {
25*bright, 20*bright, 10*bright, 4*bright, 2*bright, bright, bright, 0};
bool oldState = HIGH;  // Switch
int showType = 0;
bool animate = false;  // Web Button
void setup() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("EM-Pixels http://192.168.4.1");    // Create WiFi network
  WiFi.begin(ssid, password);
  delay(5000);
  server.begin(); 
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}
///////////////////////////////1
void wsysreset()
{
  //Perform challenge to user before getting here to reboot !!!
  //Make an attempt to send mail or other backup of log files then reboot
    ESP.restart();
}
//////////////////////////
// create HTTP 1.1 header
//////////////////////////
String MakeHTTPHeader(unsigned long ulLength)
{
  String sHeader;
  
  sHeader  = F("HTTP/1.1 200 OK\r\nContent-Length: ");
  sHeader += ulLength;
  sHeader += F("\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
  
  return(sHeader);
}


////////////////////
// make html footer
////////////////////
String MakeHTTPFooter()
{
  String sResponse;
  
  sResponse = F(" <BR><BR><FONT SIZE=-1>ESP8266 With 40 WS2812B LED's<BR><FONT SIZE=-2>Compiled Using ver. 1.6.5-1160-gef26c5f, built on Sep 30, 2015</body></html>");

  return(sResponse);
}

void loop() {
  // Get current button state.
  bool newState = digitalRead(BUTTON_PIN);

  // Check if state changed from high to low (button press).
  if ((newState == LOW && oldState == HIGH) || (animate == true)) {
    // Short delay to debounce button.
    delay(2);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON_PIN);
    if ((newState == LOW) || (animate == true)) {
      if (newState == LOW){
      showType++;
      if (showType > 10){
        showType=0;
        }
      startShow(showType);     
      }
    else if (animate == true){
      startShow(showType);
     }  
    }
   }
   // Set the last button state to the old state.
      oldState = newState;
  ///////////////////////////////////
  // Check if a client has connected
  ///////////////////////////////////
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }
  
  // Wait until the client sends some data
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout) 
  { 
    return; 
  }
  
  /////////////////////////////////////
  // Read the first line of the request
  /////////////////////////////////////
  String sRequest = client.readStringUntil('\r');
  client.flush();
  
  // stop client, if request is empty
  if(sRequest=="")
  {
    client.stop();
    return;
  }
  
  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?show=1234 HTTP/1.1
  String sPath="",sParam="", sCmd="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);
    
    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }
 
  
  ///////////////////////////
  // format the html response
  ///////////////////////////
  String sResponse,sResponse2,sHeader;

   if (sPath.startsWith("/pixel_inputs&LED"))
   {
    // 
 if (sPath.startsWith("/pixel_inputs&LED3=0")) {
    showType=0;
    animate = true;
    }
    else if (sPath.startsWith("/pixel_inputs&LED3=1")) {
 showType=1;
animate = true;
    }
    else if (sPath.startsWith("/pixel_inputs&LED3=2")) {
 showType=2;
animate = true;
    }
    else if (sPath.startsWith("/pixel_inputs&LED3=3")) {
 showType=3;
animate = true;
    }
    else if (sPath.startsWith("/pixel_inputs&LED3=4")) {
 showType=4;
animate = true;
    }
    else if (sPath.startsWith("/pixel_inputs&LED3=5")) {
 showType=5;
animate = true;
    }
    else if (sPath.startsWith("/pixel_inputs&LED3=6")) {
 showType=6;
animate = true;
    }
    else if (sPath.startsWith("/pixel_inputs&LED3=7")) {
 showType=7;
animate = true;
    }
    else if (sPath.startsWith("/pixel_inputs&LED3=8")) {
 showType=8;
animate = true;
    }
    else if (sPath.startsWith("/pixel_inputs&LED3=9")) {
showType=9;
animate = true;
    }
    else if (sPath.startsWith("/pixel_inputs&LED3=neenaw")) { 
showType=10;
animate = true;
    }
}       
  /////////////////////////////////////////
  // format the html page for Pixel Input /
  /////////////////////////////////////////
 if(sPath=="/")
  {
    ulReqcount++;
    sResponse  = F("<html>\n<head>\n<title>Webpage NeoPixel Controller</title>\n</head>\n");    
    sResponse += F("\n<script>\nfunction GetButton1()\n{\nnocache = \"&nocache=\" + Math.random() * 1000000;\nvar request = new XMLHttpRequest();\nrequest.open(\"GET\", \"pixel_inputs&LED3=0\" + nocache, true);\nrequest.send(null);\n}\nfunction GetButton2()\n{\nnocache = \"&nocache=\" + Math.random() * 1000000;\nvar request = new XMLHttpRequest();\nrequest.open(\"GET\", \"pixel_inputs&LED3=1\" + nocache, true);\nrequest.send(null);\n}\n");
    sResponse += F("\nfunction GetButton3()\n{\n\nnocache = \"&nocache=\" + Math.random() * 1000000;\nvar request = new XMLHttpRequest();\nrequest.open(\"GET\", \"pixel_inputs&LED3=2\" + nocache, true);\nrequest.send(null);\n}\nfunction GetButton4()\n{\nnocache = \"&nocache=\" + Math.random() * 1000000;\nvar request = new XMLHttpRequest();\nrequest.open(\"GET\", \"pixel_inputs&LED3=3\" + nocache, true);\nrequest.send(null);\n}\nfunction GetButton5()\n");
    sResponse += F("{\nnocache = \"&nocache=\" + Math.random() * 1000000;\nvar request = new XMLHttpRequest();\nrequest.open(\"GET\", \"pixel_inputs&LED3=4\" + nocache, true);\nrequest.send(null);\n}\nfunction GetButton6()\n{\nnocache = \"&nocache=\" + Math.random() * 1000000;\nvar request = new XMLHttpRequest();\nrequest.open(\"GET\", \"pixel_inputs&LED3=5\" + nocache, true);\nrequest.send(null);\n}\n");
    sResponse += F("\nfunction GetButton7()\n{\nnocache = \"&nocache=\" + Math.random() * 1000000;\nvar request = new XMLHttpRequest();\nrequest.open(\"GET\", \"pixel_inputs&LED3=6\" + nocache, true);\nrequest.send(null);\n}\nfunction GetButton8()\n{\nnocache = \"&nocache=\" + Math.random() * 1000000;\nvar request = new XMLHttpRequest();\nrequest.open(\"GET\", \"pixel_inputs&LED3=7\" + nocache, true);\nrequest.send(null);\n}\nfunction GetButton9()\n{\nnocache = \"&nocache=\" + Math.random() * 1000000;\nvar request = new XMLHttpRequest();\nrequest.open(\"GET\", \"pixel_inputs&LED3=8\" + nocache, true);\nrequest.send(null);\n}\n");
    sResponse += F("\nfunction GetButton10()\n{\nnocache = \"&nocache=\" + Math.random() * 1000000;\nvar request = new XMLHttpRequest();\nrequest.open(\"GET\", \"pixel_inputs&LED3=9\" + nocache, true);\nrequest.send(null);\n}\n\nfunction GetButton11()\n{\nnocache = \"&nocache=\" + Math.random() * 1000000;\nvar request = new XMLHttpRequest();\nrequest.open(\"GET\", \"pixel_inputs&LED3=neenaw\" + nocache, true);\nrequest.send(null);\n}\n</script>\n<body>\n<h1>NeoPixel Controller<BR>_________________</h1>\n<BR><BR><a href=\"/diag\">Diagnostics Information Page</a><BR>\n<body onload=\"GetESP8266IO()\">\n<h1><FONT SIZE=-1>Choose Animation of LED's</h1>\n<form id=\"txt_form\" name=\"frmText\">\n");


    sResponse2 = F("\n<button type=\"button\" id=\"LED3\" onclick=\"GetButton1()\">Pixels Set to OFF</button><br /><br />\n<button type=\"button\" id=\"LED3\" onclick=\"GetButton2()\">Pixels Red</button><br /><br />\n<button type=\"button\" id=\"LED3\" onclick=\"GetButton3()\">Pixels Green</button><br /><br />\n<button type=\"button\" id=\"LED3\" onclick=\"GetButton4()\">Pixels Blue</button><br /><br />\n<button type=\"button\" id=\"LED3\" onclick=\"GetButton5()\">Pixels White</button><br /><br />\n<button type=\"button\" id=\"LED3\" onclick=\"GetButton6()\">Flashing Red</button><br />");
    sResponse2 += F("<br />\n<button type=\"button\" id=\"LED3\" onclick=\"GetButton7()\">Police Light</button><br /><br />\n<button type=\"button\" id=\"LED3\" onclick=\"GetButton8()\">Rainbow Animation</button><br /><br />\n<button type=\"button\" id=\"LED3\" onclick=\"GetButton9()\">Rainbow Waves</button><br /><br />\n<button type=\"button\" id=\"LED3\" onclick=\"GetButton10()\">LONG ! Flashing Rainbow</button><br /><br />\n<button type=\"button\" id=\"LED3\" onclick=\"GetButton11()\">Police Strobe !Epilepsy WARNING!</button><br />Warning !! DO NOT ATTEMPT WEB ACCESS WHILE POLICE STROBE IS RUNNING !!\n</form>\n");    
 
    sResponse2 += F("\n<font color=\"#000000\"><body bgcolor=\"#a0dFfe\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\"><BR>Animated Neopixels ANY TYPE !<BR>\n</div>\n<div style=\"clear:both;\"></div><p>\n");
    sResponse2 += MakeHTTPFooter().c_str();
        // Send the response to the client 
    client.print(MakeHTTPHeader(sResponse.length()+sResponse2.length()).c_str());
    client.print(sResponse);
    client.print(sResponse2);
}


else if(sPath=="/yfireset")  
  {
    ulReqcount++;
    
                                client.println("HTTP/1.1 200 OK"); 
                                client.println("Content-Type: text/html");
                                client.println("Connection: close");
                                client.println();
                                client.println("<!DOCTYPE HTML>");
                                client.print("<html><head><title>Webpage NeoPixel Controller</title></head><body>");
                                client.print("<font color=\"#000000\"><body bgcolor=\"#a0dFfe\">");
                                client.print("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">");
                                client.print("<h1>Webpage NeoPixel Controller<BR>WiFi Reset Page </h1><BR><a href=\"/graphic2\">Pressure Graph Page</a><BR><a href=\"/pressure\">Pressure Gauge Page</a>");
                                client.print("<BR><a href=\"/monitor\">Sensor Gauges Page</a><BR><a href=\"/graphic\">Sensor Graph Page</a><BR><a href=\"/table\">Sensor Datalog Page</a><BR><BR><BR><BR>Restarted WiFiConnections = ");
                                client.print(ulReconncount);
                                client.print("<BR><FONT SIZE=-2>environmental.monitor.log@gmail.com<BR><FONT SIZE=-2>ESP8266 With DHT11 & BMP180 Sensors<BR>");
                                client.print("<FONT SIZE=-2>Compiled Using ver. 1.6.5-1160-gef26c5f, built on Sep 30, 2015<BR>");
                                client.println("<IMG SRC=\"https://raw.githubusercontent.com/genguskahn/ESP8266-For-DUMMIES/master/SoC/DimmerDocs/organicw.gif\" WIDTH=\"250\" HEIGHT=\"151\" BORDER=\"1\"></body></html>");

  //WiFiStart();
  }
  else if(sPath=="/diag")
  {   
     float   servolt1 = ESP.getVcc();
     long int spdcount = ESP.getCycleCount();
     delay(1);
     long int spdcount1 = ESP.getCycleCount();
     long int speedcnt = spdcount1-spdcount; 
     FlashMode_t ideMode = ESP.getFlashChipMode();
     ulReqcount++;
                                String duration1 = " ";
                                int hr,mn,st;
                                st = millis() / 1000;
                                mn = st / 60;
                                hr = st / 3600;
                                st = st - mn * 60;
                                mn = mn - hr * 60;
                                if (hr<10) {duration1 += ("0");}
                                duration1 += (hr);
                                duration1 += (":");
                                if (mn<10) {duration1 += ("0");}
                                duration1 += (mn);
                                duration1 += (":");
                                if (st<10) {duration1 += ("0");}
                                duration1 += (st);     
                                client.println("HTTP/1.1 200 OK"); 
                                client.println("Content-Type: text/html");
                                client.println("Connection: close");
                                client.println();
                                client.println("<!DOCTYPE HTML>");
                                client.print("<html><head><title>Webpage NeoPixel Controller</title></head><body>");
                                client.print("<font color=\"#000000\"><body bgcolor=\"#a0dFfe\">");
                                client.print("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">");
                                client.print("<h1>Neopixel Control<BR>SDK Diagnostic Information</h1>");
                                client.print("<BR><a href=\"/\">Pixel Control Page</a><BR>");
                               String diagdat="";
                              //  diagdat+=ulReconncount;
                                diagdat+="<BR>  Web Page Requests = ";
                                diagdat+=ulReqcount;
                                diagdat+="<BR>  WiFi AP Hostname = ";
                                diagdat+=host;
                                diagdat+="<BR>  Free RAM = ";
                                client.print(diagdat);
                                client.print((uint32_t)system_get_free_heap_size()/1024);
                               // diagdat=" KBytes<BR>  Logged Sample Count = ";
                               // diagdat+=ulMeasCount;
                                diagdat=" KBytes<BR>  SDK Version = ";                                 
                                diagdat+=ESP.getSdkVersion();
                                diagdat+="<BR>  Boot Version = ";
                                diagdat+=ESP.getBootVersion();
                                diagdat+="<BR>  Free Sketch Space  = ";
                                diagdat+=ESP.getFreeSketchSpace()/1024;
                                diagdat+=" KBytes<BR>  Sketch Size  = ";
                                diagdat+=ESP.getSketchSize()/1024;
                                diagdat+=" KBytes<BR>";
                                client.print(diagdat);
                                client.printf("  Flash Chip id = %08X\n", ESP.getFlashChipId());
                                client.print("<BR>");
                                client.printf("  Flash Chip Mode = %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
                                diagdat="<BR>  Flash Size By ID = ";
                                diagdat+=ESP.getFlashChipRealSize()/1024;
                                diagdat+=" KBytes<BR>  Flash Size (IDE) = "; 
                                diagdat+=ESP.getFlashChipSize()/1024;
                                diagdat+=" KBytes<BR>  Flash Speed = ";
                                diagdat+=ESP.getFlashChipSpeed()/1000000;
                                diagdat+=" MHz<BR>  ESP8266 CPU Speed = ";
                                diagdat+=ESP.getCpuFreqMHz();
                                diagdat+=" MHz<BR>";
                                client.print(diagdat);
                                client.printf("  ESP8266 Chip id = %08X\n", ESP.getChipId());
                                diagdat="<BR>  System Instruction Cycles Per Second = ";
                                diagdat+=speedcnt*1000;  
                                diagdat+="<BR>  Last System Restart Reason = ";
                                diagdat+=ESP.getResetInfo();                                                              
                                diagdat+="<BR>  System VCC = ";
                                diagdat+=servolt1/1000, 3;
                                diagdat+=" V <BR>  Datalog File Size in Bytes = N/A";
                                //diagdat+=Dfsize;
                                diagdat+="<BR>  System Uptime =";
                                diagdat+=duration1;
                                client.print(diagdat);
                                client.print("<BR><FONT SIZE=-2>environmental.monitor.log@gmail.com<BR><FONT SIZE=-2>ESP8266 With 40 WS2812B 5050 LED's<BR><FONT SIZE=-2>Compiled Using ver. 1.6.5-1160-gef26c5f, built on Sep 30, 2015<BR>");
                                client.println("<IMG SRC=\"https://raw.githubusercontent.com/genguskahn/ESP8266-For-DUMMIES/master/SoC/DimmerDocs/organicw.gif\" WIDTH=\"370\" HEIGHT=\"240\" BORDER=\"1\"></body></html>");
                                diagdat = "";
                                duration1 = "";
  }
   else if(sPath=="/srestart")
  {
                                client.println("HTTP/1.1 200 OK"); 
                                client.println("Content-Type: text/html");
                                client.println("Connection: close");
                                client.println();
                                client.println("<!DOCTYPE HTML>");
                                client.print("<html><head><title>Webpage NeoPixel Controller</title></head><body>");
                                client.print("<font color=\"#000000\"><body bgcolor=\"#a0dFfe\">");
                                client.print("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">");
                                client.print("<h1>Environment Monitor<BR>Please wait 10 Seconds......<BR><FONT SIZE=+2>System Reset Page !!!! </h1><BR><a href=\"/graphic2\">Pressure Graph Page</a><BR><a href=\"/pressure\">Pressure Gauge Page</a>");
                                client.print("<BR><a href=\"/monitor\">Sensor Gauges Page</a><BR><a href=\"/graphic\">Sensor Graph Page</a><BR><a href=\"/table\">Sensor Datalog Page</a><BR><a href=\"/diag\">Diagnostics Information Page</a><BR><a href=\"/\">Monitor LCD Text Input Page</a><BR><BR><BR><BR>Restarted WiFiConnections = ");
                                client.print(ulReconncount);
                                client.print("<BR><BR><BR><FONT SIZE=-2>environmental.monitor.log@gmail.com<BR><FONT SIZE=-2>ESP8266 With DHT11 & BMP180 Sensors<BR>");
                                client.print("<FONT SIZE=-2>Compiled Using ver. 1.6.5-1160-gef26c5f, built on Sep 30, 2015<BR>");
                                client.println("<IMG SRC=\"https://raw.githubusercontent.com/genguskahn/ESP8266-For-DUMMIES/master/SoC/DimmerDocs/organicw.gif\" WIDTH=\"250\" HEIGHT=\"151\" BORDER=\"1\"></body></html>");


client.stop();
wsysreset();
  }
  else 
////////////////////////////
// 404 for non-matching path
////////////////////////////
  {
    sResponse="<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server, What did you sk for?.</p></body></html>";
    ulReqcount++;
    sHeader  = F("HTTP/1.1 404 Not found\r\nContent-Length: ");
    sHeader += sResponse.length();
    sHeader += F("\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
    
    // Send the response to the client
    client.print(sHeader);
    client.print(sResponse);
  }
  
  // and stop the client
  client.stop();
}

void colorWipe1(int R, int G, int B, int wait)
{
   for (boolean inversePattern = 0; inversePattern <=1; inversePattern++)            // Alternates each segment between Red & Blue
   {
      for (int burst = 0 ; burst <= maxBurst ; burst++)                        // Let's run through a few times before inverting color (red/blue)
      {
         //    Run lights starting from pixel 0
         for (int segment = numSegments-1; segment >= 0; segment--)      // Example (2 segments -> segment=0, segment=1)
         // for (int segment = 0; segment < numSegments; segment++)      // Example (2 segments -> segment=0, segment=1)
         {
            // to set pixel colors in each segment
            for (int pixel = node[segment]; pixel < node[segment+1]; pixel++)    // Example (2 segments -> pixel=0, pixel=15 && pixel=15, pixel=30)
            {
               boolean Blue;                              // Will the segment be BLUE? ... or RED?  ... let's find out!

               if (inversePattern == 1)
               {
                  //  if odd, Blue
                  if((segment%2)!=0)  Blue = true;
                  //  if even, Red
                  else if ((segment%2)==0)  Blue = false;
               }    // end if (inverse...

               else if (inversePattern == 0)
               {
                  //  if odd, Blue
                  if((segment%2)!=0)  Blue = false;
                  //  if even, Red
                  else if ((segment%2)==0)  Blue = true;
               }    // end if (!inverse....

               for (int tailPosition = 0; tailPosition <= tailSize; tailPosition++)      // Pixels within each segment
               {
                  if (!Blue)
                  {
                     strip.setPixelColor(pixel-tailPosition, (r[tailPosition]), (g[tailSize]), (b[tailSize]));    // Red ONLY
                  }    // end if Red...
                  if (Blue)
                  {
                     strip.setPixelColor(pixel-tailPosition, (r[tailSize]), (g[tailSize]), (b[tailPosition]));    // Blue ONLY
                  }    // end if Blue...

               }    // end for (int tailPosition...
               strip.show();
               delay(wait);
            }    // end for (int pixel...

         }    // end for (int segment...

         // Now, shrink the tail....
         for(int p = 1; p <= tailSize; p++)
         {
            for(int t = (strip.numPixels()-tailSize); t < (strip.numPixels()); t++)
            {
               int v = ((strip.numPixels()-t)+p);
               if (v > tailSize) v = tailSize;

               strip.setPixelColor(t, (r[v]), (g[v]), (b[v]));
            }

            strip.show();
            delay(wait);
            // No delay() here. It causes too long of a pause at ends of strip.
         }
         delay(wait);
         //    Run lights back towards 0
         for (int segment = 0; segment < numSegments; segment++)      // Example (2 segments -> segment=0, segment=1)

         //     for (int segment = (numSegments-1); segment >= 0; segment--)      // Example (2 segments -> segment=0, segment=1)
         {
            // to set pixel colors in each segment
            for (int pixel = node[segment+1]; pixel > node[segment]; pixel--)    // Example (2 segments -> pixel=0, pixel=15 && pixel=15, pixel=30)
            {
               boolean Blue;
               if (inversePattern == 1)
               {
                  //  if odd, Blue
                  if((segment%2)!=0)  Blue = true;
                  //  if even, Red
                  else if ((segment%2)==0)  Blue = false;
               }    // end if (inverse...

               else if (inversePattern == 0)
               {
                  //  if odd, Blue
                  if((segment%2)!=0)  Blue = false;
                  //  if even, Red
                  else if ((segment%2)==0)  Blue = true;
               }    // end if (!inverse....


               for (int tailPosition = tailSize; tailPosition >= 0; tailPosition--)      // Pixels within each segment
               {
                  if (!Blue)
                  {
                     strip.setPixelColor(pixel+tailPosition, (r[tailPosition]), (g[tailSize]), (b[tailSize]));    // Red ONLY
                  }    // end if Red...
                  if (Blue)
                  {
                     strip.setPixelColor(pixel+tailPosition, (r[tailSize]), (g[tailSize]), (b[tailPosition]));    // Blue ONLY
                  }    // end if Blue...

               }    // end for (int tailPosition...
               strip.show();
               // delay(wait);
            }    // end for (int pixel...

         }    // end for (int segment...
         // Now, shrink the tail....
         for(int p = 1; p <= tailSize; p++)
         {
            for(int t = (tailSize); t < 0; t++)
            {
               int v = (t+p);
               if (v > tailSize) v = tailSize;

               strip.setPixelColor(t, (r[v]), (g[v]), (b[v]));
            }

            strip.show();
            delay(wait);
            // No delay() here. It causes too long of a pause at ends of strip.
         }
      }
   }
}  // end void colorWipe1()

void neenaw(){
for (int i=0;i<3;i++){
  if (i==2){showType=0;return;}
 int flash = random(minFlashes, maxFlashes);

 // Pick new number of segments
 numSegments = random(2, maxNumberOfSegments);   // Randomly choose how many segments in bar
 int wait = random(0,numSegments)/3;
 maxBurst = random(3, numSegments);

 // defining node (end pixels of segments) from number of segments defined
 for (int i = 0; i <= numSegments; i++)
 {
    node[i] = (i * (strip.numPixels() / numSegments));
 }    // end for  - numSegments - node array
 constrain (tailSize, 0, ((strip.numPixels() / numSegments)-1));
 for ( flash = 0 ; flash <= maxFlashes ; flash++)
 {
    colorWipe1(r[0], g[0], b[0], wait);
 }
 delay(20);
}
}


void startShow(int i) {
  switch(i){
    case 0: colorWipe(strip.Color(0, 0, 0), 50);    // Black/off
            break;
    case 1: colorWipe(strip.Color(255, 0, 0), 50);  // Red
            break;
    case 2: colorWipe(strip.Color(0, 255, 0), 50);  // Green
            break;
    case 3: colorWipe(strip.Color(0, 0, 255), 50);  // Blue
            break;
    case 4: colorWipe(strip.Color(127, 127, 127), 50); // White
            break;
    case 5: theaterChase(strip.Color(127,   0,   0), 50); // Red
            break;
    case 6: theaterChase(strip.Color(  0,   0, 127), 50); // Blue
            break;
    case 7: rainbow(20);
            break;
    case 8: rainbowCycle(20);
            break;
    case 9: theaterChaseRainbow(50);
            break;
    case 10: neenaw();
             break;
    default: colorWipe(strip.Color(0, 0, 0), 50);    // Black/off
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
