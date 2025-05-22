#include <Wire.h>
#include <RTClib.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

// Pines
#define LED_R 5
#define LED_G 6
#define LED_B 7
#define DHTPIN 8
#define I2C_SDA 21
#define I2C_SCL 19

// PWM
const uint8_t chR = 0, chG = 1, chB = 2;
const uint8_t pwmResolution = 8;
const uint32_t pwmFrequency = 5000;

// DHT
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// RTC
RTC_DS3231 rtc;

// BMP180
Adafruit_BMP085_Unified bmp;

// Configurar PWM LED RGB
void SetupLEDRGB() {
  ledcSetup(chR, pwmFrequency, pwmResolution);
  ledcSetup(chG, pwmFrequency, pwmResolution);
  ledcSetup(chB, pwmFrequency, pwmResolution);
  ledcAttachPin(LED_R, chR);
  ledcAttachPin(LED_G, chG);
  ledcAttachPin(LED_B, chB);
}

// Función para controlar el color manualmente
void SetColor(uint8_t r, uint8_t g, uint8_t b) {
  ledcWrite(chR, 255 - r);  // Invertido para LED común ánodo
  ledcWrite(chG, 255 - g);
  ledcWrite(chB, 255 - b);
}

void setup() {
  Serial.begin(115200);

  // I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Inicializar sensores
  dht.begin();
  rtc.begin();
  bmp.begin();

  // Configurar LEDs
  SetupLEDRGB();

  // Si el RTC perdió la hora
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("Todos los sensores inicializados.");
}

void loop() {
  // Leer DHT11
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Leer fecha/hora
  DateTime now = rtc.now();

  // Leer BMP180
  sensors_event_t event;
  bmp.getEvent(&event);

  Serial.println("---- Sensores ----");

  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.println(" °C");

  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.println(" %");

  Serial.print("Fecha: ");
  Serial.print(now.day()); Serial.print("/");
  Serial.print(now.month()); Serial.print("/");
  Serial.print(now.year()); Serial.print("  ");
  Serial.print(now.hour()); Serial.print(":");
  Serial.print(now.minute()); Serial.print(":");
  Serial.println(now.second());

  if (event.pressure) {
    Serial.print("Presión: ");
    Serial.print(event.pressure);
    Serial.println(" hPa");

    float tempBar;
    bmp.getTemperature(&tempBar);

    float altitud = bmp.pressureToAltitude(1013.25, event.pressure);
    Serial.print("Altitud estimada: ");
    Serial.print(altitud);
    Serial.println(" m");
  } else {
    Serial.println("Error al leer presión.");
  }
  SetColor(255, 100, 0);  // Naranja
  Serial.println("------------------");
  delay(10000);  // Esperar 10 segundos
}
