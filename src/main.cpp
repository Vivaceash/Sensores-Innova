#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <DHT.h>
#include <RTClib.h>

// Pines LED RGB
#define LED_R 5
#define LED_G 6
#define LED_B 7

// Pines DHT11
#define DHTPIN 8
#define DHTTYPE DHT11

// I2C para sensores
#define I2C_SDA 21
#define I2C_SCL 19

// Configuración PWM LED RGB
const uint8_t chR = 0, chG = 1, chB = 2;
const uint8_t pwmResolution = 8;
const uint32_t pwmFrequency = 5000;

// Objetos sensores
DHT dht(DHTPIN, DHTTYPE);
RTC_DS3231 rtc;
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

AsyncWebServer server(80);

int currentPreset = 1;

// Función para configurar LED RGB con PWM
void SetupLEDRGB() {
  ledcSetup(chR, pwmFrequency, pwmResolution);
  ledcSetup(chG, pwmFrequency, pwmResolution);
  ledcSetup(chB, pwmFrequency, pwmResolution);

  ledcAttachPin(LED_R, chR);
  ledcAttachPin(LED_G, chG);
  ledcAttachPin(LED_B, chB);
}

void SetColor(uint8_t r, uint8_t g, uint8_t b) {
  ledcWrite(chR, 255 - r);
  ledcWrite(chG, 255 - g);
  ledcWrite(chB, 255 - b);
}

void SetPreset(int preset) {
  currentPreset = preset;
  switch(preset) {
    case 1: SetColor(0, 0, 255); break;   // Azul
    case 2: SetColor(255, 0, 0); break;   // Rojo
    case 3: SetColor(0, 255, 0); break;   // Verde
    default: SetColor(0, 0, 0); break;    // Apagado
  }
}

// Para enviar datos JSON con sensores
String getSensorJSON() {
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

void setup() {
  Serial.begin(115200);

  // Configurar I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Iniciar SPIFFS
  if(!SPIFFS.begin(true)) {
    Serial.println("Error iniciando SPIFFS");
    while(true);
  }

  // Configurar sensores
  dht.begin();

  if (!rtc.begin()) {
    Serial.println("No se encontró RTC");
    while (true);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!bmp.begin()) {
    Serial.println("No se encontró BMP180");
    while (true);
  }

  // Configurar LEDs RGB
  SetupLEDRGB();
  SetPreset(currentPreset);

  // Configurar AP WiFi
  WiFi.softAP("ESP32-TrampaAP", "12345678");
  Serial.print("IP AP: ");
  Serial.println(WiFi.softAPIP());

  // Rutas servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "application/javascript");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Endpoint para leer sensores JSON
  server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorJSON();
    request->send(200, "application/json", json);
  });

  // Endpoint para cambiar preset del LED RGB
  server.on("/preset", HTTP_GET, [](AsyncWebServerRequest *request){
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
  // nada aquí, todo por web y timer JS
}
