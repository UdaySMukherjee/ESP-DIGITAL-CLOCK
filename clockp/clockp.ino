#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// ---------- WiFi ----------
const char* ssid = "Airtel_Zerotouch";
const char* password = "Airtel@123";

// ---------- Pins ----------
#define BUTTON_PIN D1
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Time ----------
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // IST (UTC+5:30)

// ---------- State ----------
bool showClock = true;
unsigned long lastSwitch = 0;
const unsigned long switchInterval = 5000; // 5 seconds

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  timeClient.begin();
  timeClient.update();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected!");
  display.display();
  delay(1000);
}

void loop() {
  timeClient.update();
  unsigned long now = millis();

  // Alternate every 5 seconds
  if (now - lastSwitch > switchInterval) {
    showClock = !showClock;
    lastSwitch = now;
  }

  if (showClock) {
    drawClockUI();
  } else {
    drawDHTUI();
  }

  delay(100); // smoother rendering
}

// ========== UI FUNCTIONS ==========

void drawClockUI() {
  drawHeader(" Time ");

  int hour24 = timeClient.getHours();
  int minute = timeClient.getMinutes();
  int dayofweek = ((timeClient.getEpochTime() / 86400L + 4) % 7) + 1;

  // Convert to 12-hour
  String period = "AM";
  int hour12 = hour24;
  if (hour24 >= 12) {
    period = "PM";
    if (hour24 > 12) hour12 = hour24 - 12;
  } else if (hour24 == 0) {
    hour12 = 12;
  }

  // Time Display
  display.setTextSize(3);
  display.setCursor(10, 20);
  if (hour12 < 10) display.print("0");
  display.print(hour12);
  display.print(":");
  if (minute < 10) display.print("0");
  display.print(minute);
  display.setTextSize(1);
  display.print(" ");
  display.print(period);

  // Day of Week
  display.setCursor(0, 50);
  switch (dayofweek) {
    case 1: display.print("Monday, "); break;
    case 2: display.print("Tuesday, "); break;
    case 3: display.print("Wednesday, "); break;
    case 4: display.print("Thursday, "); break;
    case 5: display.print("Friday, "); break;
    case 6: display.print("Saturday, "); break;
    case 7: display.print("Sunday, "); break;
  }

  // Date (DD.MM.YYYY)
  time_t rawTime = timeClient.getEpochTime();
  struct tm* timeinfo = localtime(&rawTime);
  if (timeinfo->tm_mday < 10) display.print("0");
  display.print(timeinfo->tm_mday);
  display.print(".");
  if ((timeinfo->tm_mon + 1) < 10) display.print("0");
  display.print(timeinfo->tm_mon + 1);
  display.print(".");
  display.print(timeinfo->tm_year + 1900);

  display.display();
}

void drawDHTUI() {
  drawHeader(" Temp & Humidity ");

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  display.setTextSize(2);
  if (isnan(temp) || isnan(hum)) {
    display.setCursor(10, 25);
    display.setTextSize(1);
    display.println("Sensor Error ");
  } else {
    display.setCursor(5, 20);
    display.print("T: ");
    display.print(temp, 1);
    display.print(" C");

    display.setCursor(5, 44);
    display.print("H: ");
    display.print(hum, 1);
    display.print(" %");
  }

  display.display();
}

void drawHeader(String title) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(2, 2);
  display.println(title);
  display.drawLine(0, 11, 128, 11, WHITE);
}
