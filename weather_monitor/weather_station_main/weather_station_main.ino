/*

Code written by ISHANT MEHNDIRATTA
Hardware Used :
                ESP32 devkit v1
                0.96" oled Display 128*64
                PushButton
                10k ohm Resistor

Connections :
                display to ESP32
                  VCC - 3.3V
                  GND - GND
                  SCL - GPIO22
                  SDA - GPIO21
                  my display doesnt have reset pin
                ESP32 to button
                  output to GPIO4
  
*/
#include <WiFi.h>
#include "time.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


const int buttonPin = 4;  // the number of the pushbutton pin
int buttonState = 0; // to store HIGH and LOW states
int displayMode = 0; // can be initilised to 0, 1 or 2

// Replace with your network credentials (STATION)
const char* ssid = "EACCESS";
const char* password = "hostelnet";

// details for fetching time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800; // GMT + 5hr 30min 
const int   daylightOffset_sec = 0; // 0 for india

String serverPath = "https://api.openweathermap.org/data/2.5/weather?q=Patiala,IN&units=metric&appid=a21084f64df1d054c7af8d3a29ae4b6d";
String jsonBuffer;

// global variables that stores data fetched by update function
String gtimestr = "11:11";
String gdatestr = "15-01-2006";
String gtemp = "00.00C";
String gfeel = "00.00 C";
String gpressure = "0000mBar";
String gtemp_min = "00.00C";
String gtemp_max = "00.00C";

String httpGETRequest(const char* serverName) {
  WiFiClientSecure client;
  client.setInsecure();  // allow HTTPS without certificate verification

  HTTPClient http;
  http.begin(client, serverName);  // HTTPS

  int httpResponseCode = http.GET();
  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return payload;
}
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(600);
  }
  Serial.println(WiFi.localIP());
}


//----------data fetching----------
void getLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    gtimestr = "00:00";
  }

  char timeStr[6];  // HH:MM + null
  strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
  gtimestr = String(timeStr);
}

void getLocalDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain date");
    gdatestr = "00/00/0000";
  }

  char dateStr[11];  // DD/MM/YYYY + null
  strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
  gdatestr = String(dateStr);
}

void getLocalWeather(){
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      // Serial.println(double(myObject["main"]["temp"]));
    
      gtemp = (String((double) myObject["main"]["temp"]) + " C");
      gfeel = (String((double) myObject["main"]["feels_like"]) + " C");
      gtemp_min = (String((double) myObject["main"]["temp_min"]) + " C");
      gtemp_max = (String((double) myObject["main"]["temp_max"]) + " C");
      gpressure = (String((double) myObject["main"]["pressure"]) + " mBar");
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}
// function thats calls data-fetching functions and updates data in global variables
void updateData() {
  static unsigned long lastUpdate = 0;  // remembers last update time
  unsigned long currentMillis = millis();

  // Only update if 30 seconds (30000 ms) have passed
  if (currentMillis - lastUpdate >= 30000) {
    Serial.println("🔄 Updating time, date, and weather...");
    
    getLocalTime();
    getLocalDate();
    getLocalWeather();
    
    lastUpdate = currentMillis;
  }
}


//----------Display Design----------

void display_0(){
//time date
  display.setTextColor(WHITE);

  display.setTextSize(4);
  display.setCursor(5, 5);
  display.println(gtimestr);

  display.setTextSize(2);
  display.setCursor(5, 48);
  display.println(gdatestr);

  display.display();
}

void display_1(){
// feels like
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(4, 0);
  display.println("Feels like");
  display.setTextSize(3);
  display.setCursor(0,30);
  display.println(gfeel);

  display.display();
}

void display_2(){
//weather
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Temperature Data : ");
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.println("current T");
  display.setCursor(70, 25);
  display.println(gtemp);
  display.setCursor(0, 40);
  display.println("Pressure ");
  display.setCursor(55, 40);
  display.println(gpressure);

  display.display();
}


void display_startup(){
  // Clear the buffer.
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("BOOTING......");
  display.setTextSize(3);
  display.setCursor(35, 25);
  display.println(">_<");
  display.display();
  delay(10000);
}



void setup() {
  Serial.begin(115200);
  updateData();
  pinMode(buttonPin, INPUT);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0*3C is address of iic display used by me. use the address finder code first to confirm!
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  display_startup();
  updateData();
}

void loop() {

  updateData();
  

  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    // change display
    displayMode = (displayMode + 1) % 3;
    delay(500);
  }
  // Call display functions based on mode
  if (displayMode == 0) {
    display.clearDisplay();
    display_0();
  } else if (displayMode == 1) {
    display.clearDisplay();
    display_1();
  } else if (displayMode == 2) {
    display.clearDisplay();
    display_2();
  }
}