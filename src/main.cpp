#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <DHT.h>
#include <RTClib.h>
#include <Adafruit_NeoPixel.h>

// Pines
#define DHTPIN 8
#define DHTTYPE DHT11
#define I2C_SDA 21
#define I2C_SCL 19
#define LED_PIN 5       // DIN de la tira RGB NeoPixel
#define NUM_PIXELS 60   // Cambia este valor según el número de LEDs

// Objetos sensores
DHT dht(DHTPIN, DHTTYPE);
RTC_DS3231 rtc;
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

AsyncWebServer server(80);
int currentPreset = 1;

unsigned long lastPrintTime = 0;
const unsigned long printInterval = 10000;

// Variables para arcoiris
unsigned long lastRainbowUpdate = 0;
const unsigned long rainbowInterval = 20;
uint16_t rainbowIndex = 0;

// Función para establecer colores en toda la tira
void SetColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

// Función para generar color arcoíris (wheel)
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Función para animar el arcoíris
void RainbowCycle() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, Wheel((i + rainbowIndex) & 255));
  }
  pixels.show();
  rainbowIndex++;
}

// Función para apagar LEDs
void Apagar() {
  pixels.clear();
  pixels.show();
}

// Cambia el preset actual y actualiza el LED según preset
void SetPreset(int preset) {
  currentPreset = preset;
  switch (preset) {
    case 1: SetColor(0, 0, 255); break;   // Azul
    case 2: SetColor(255, 0, 0); break;   // Rojo
    case 3: SetColor(0, 255, 0); break;   // Verde
    case 4: rainbowIndex = 0; break;      // Arcoiris (se animará en loop)
    default: Apagar(); break;              // Apagar
  }
}

// Función para obtener los datos como JSON
String GetSensorJSON() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  sensors_event_t event;
  float presion = 0, altitud = 0;
  if (bmp.getEvent(&event) && event.pressure) {
    presion = event.pressure;
    altitud = bmp.pressureToAltitude(1013.25, presion);
  }

  DateTime now = rtc.now();

  String json = "{";
  json += "\"temp\":" + String(t, 2) + ",";
  json += "\"hum\":" + String(h, 2) + ",";
  json += "\"presion\":" + String(presion, 2) + ",";
  json += "\"altitud\":" + String(altitud, 2) + ",";
  json += "\"year\":" + String(now.year()) + ",";
  json += "\"month\":" + String(now.month()) + ",";
  json += "\"day\":" + String(now.day()) + ",";
  json += "\"hour\":" + String(now.hour()) + ",";
  json += "\"minute\":" + String(now.minute()) + ",";
  json += "\"second\":" + String(now.second());
  json += "}";
  return json;
}

// Función para imprimir en consola cada 10s
void PrintSensorData() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  sensors_event_t event;
  float presion = 0, altitud = 0;
  if (bmp.getEvent(&event) && event.pressure) {
    presion = event.pressure;
    altitud = bmp.pressureToAltitude(1013.25, presion);
  }

  DateTime now = rtc.now();

  Serial.println("===== Lectura periódica de sensores =====");
  Serial.print("Temperatura: "); Serial.print(t); Serial.println(" °C");
  Serial.print("Humedad: "); Serial.print(h); Serial.println(" %");
  Serial.print("Presión: "); Serial.print(presion); Serial.println(" hPa");
  Serial.print("Altitud: "); Serial.print(altitud); Serial.println(" m");
  Serial.print("Fecha: "); Serial.print(now.day()); Serial.print("/");
  Serial.print(now.month()); Serial.print("/"); Serial.println(now.year());
  Serial.print("Hora: "); Serial.print(now.hour()); Serial.print(":");
  Serial.print(now.minute()); Serial.print(":"); Serial.println(now.second());
  Serial.println("=========================================");
}

void setup() {
  Serial.begin(115200);

  // I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error iniciando SPIFFS");
    while (true);
  }

  // Sensores
  dht.begin();

  if (!rtc.begin()) {
    Serial.println("No se encontró RTC");
    while (true);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Actualiza fecha y hora al compilar
  }

  if (!bmp.begin()) {
    Serial.println("No se encontró BMP180");
    while (true);
  }

  // NeoPixel
  pixels.begin();
  pixels.clear();
  SetPreset(currentPreset);

  // Wi-Fi Access Point
  WiFi.softAP("ESP32-TrampaAP", "12345678");
  Serial.print("IP AP: ");
  Serial.println(WiFi.softAPIP());

  // Rutas
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/script.js", "application/javascript");
  });

  server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", GetSensorJSON());
  });

  server.on("/preset", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("color")) {
      int p = request->getParam("color")->value().toInt();
      SetPreset(p);
      request->send(200, "text/plain", "Preset cambiado");
    } else {
      request->send(400, "text/plain", "Falta parametro 'color'");
    }
  });

  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  unsigned long now = millis();
  if (now - lastPrintTime >= printInterval) {
    lastPrintTime = now;
    PrintSensorData();
  }

  // Animación arcoiris solo si preset == 4
  if (currentPreset == 4) {
    if (now - lastRainbowUpdate >= rainbowInterval) {
      lastRainbowUpdate = now;
      RainbowCycle();
    }
  }
}
