#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <DHT.h>
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Definir pines I2C
#define I2C_SDA 21
#define I2C_SCL 19

// Definir pines para sensores
#define DHTPIN 9
#define S0 14
#define S1 15
#define S2 16
#define S3 17
#define OUT 18
#define LED_R 4
#define LED_G 5
#define LED_B 13

// Configuración de sensores
#define DHTTYPE DHT11

// Definir objetos para los sensores
RTC_DS3231 rtc;
Adafruit_BMP085_Unified bmp(10);
DHT dht(DHTPIN, DHTTYPE);

// Estructura para detectar colores
struct Color {
  String nombre;
  int rMin, rMax;
  int gMin, gMax;
  int bMin, bMax;
};

// Estructura para LED RGB
struct ColorRGB {
  String nombre;
  int r, g, b;
};

// Tabla de colores detectables
Color colores[] = {
  {"Rojo", 50, 360, 150, 212, 112, 162},
  {"Verde", 10, 60, 100, 130, 10, 60},
  {"Azul", 10, 60, 10, 60, 100, 130},
  {"Amarillo oscuro", 32, 40, 40, 58, 56, 87},
  {"Amarillo claro", 42, 71, 52, 84, 65, 116},
  {"Naranja", 41, 54, 100, 167, 95, 159},
  {"Cian", 93, 174, 58, 88, 35, 56},
  {"Magenta", 79, 141, 112, 212, 71, 128},
  {"Blanco", 27, 37, 26, 38, 22, 32},
  {"Verde claro", 120, 160, 70, 100, 100, 130}  // Nuevo rango ampliado
};

// Tabla para representar colores en el LED RGB
ColorRGB coloresLED[] = {
  {"Rojo", 255, 0, 0},
  {"Verde", 0, 255, 0},
  {"Azul", 0, 0, 255},
  {"Amarillo oscuro", 200, 120, 0},
  {"Amarillo claro", 255, 255, 100},
  {"Naranja", 255, 64, 0},
  {"Cian", 0, 255, 255},
  {"Magenta", 255, 0, 255},
  {"Blanco", 255, 255, 255},
  {"Sin color detectado", 0, 0, 0},  // LED apagado
  {"Verde claro", 0, 255, 0}  // Nuevo color verde en LED
};

// Variables para PWM
const uint8_t ledChannel_R = 0;
const uint8_t ledChannel_G = 1;
const uint8_t ledChannel_B = 2;
const uint8_t pwmResolution = 8;
const uint32_t pwmFrequency = 5000;

// Variables para control de tiempo
unsigned long ultimaLecturaColor = 0;
unsigned long ultimaLecturaSensores = 0;
const unsigned long INTERVALO_COLOR = 1000;    // 1 segundo
const unsigned long INTERVALO_SENSORES = 10000; // 10 segundos

// Variables para WiFi y servidor web
const char *ssid = "ESP32-Access-Point";
const char *password = "123456789";
AsyncWebServer server(80);

// Prototipos de funciones
String detectarColor(int rojo, int verde, int azul);
void mostrarColorEnLED(String colorDetectado);
void leerSensorColor();
void imprimirDatosSensores();
void initI2C();
void initRTC();
void initSensores();
void initColorSensor();
void initLEDRGB();

// Variables globales
String colorDetectado = "Sin color detectado";
float temperatura, humedad, altitud, presion;

void initI2C() {
  Wire.begin(I2C_SDA, I2C_SCL);
}

void initRTC() {
  if (!rtc.begin()) {
    Serial.println("Error: RTC no encontrado");
    while (1) delay(10);
  }
  
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void initSensores() {
  dht.begin();
  
  if (!bmp.begin()) {
    Serial.println("Error: BMP180 no encontrado");
    while (1) delay(10);
  }
}

void initColorSensor() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);
}

void initLEDRGB() {
  ledcSetup(ledChannel_R, pwmFrequency, pwmResolution);
  ledcSetup(ledChannel_G, pwmFrequency, pwmResolution);
  ledcSetup(ledChannel_B, pwmFrequency, pwmResolution);
  ledcAttachPin(LED_R, ledChannel_R);
  ledcAttachPin(LED_G, ledChannel_G);
  ledcAttachPin(LED_B, ledChannel_B);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  initI2C();
  initRTC();
  initSensores();
  initColorSensor();
  initLEDRGB();

  // Conectar al WiFi en modo Access Point
  WiFi.softAP(ssid, password);
  Serial.println("ESP32 en modo Access Point");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Servidor web para mostrar los datos
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body>";
    html += "<h1>Color Detectado: " + colorDetectado + "</h1>";
    
    // Mostrar temperatura, humedad, presión y altitud
    html += "<h2>Datos de Sensores</h2>";
    html += "<p>Temperatura: " + String(temperatura) + " °C</p>";
    html += "<p>Humedad: " + String(humedad) + " %</p>";
    html += "<p>Presión: " + String(presion) + " hPa</p>";
    html += "<p>Altitud: " + String(altitud) + " m</p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
  unsigned long tiempoActual = millis();
  
  // Leer sensor de color cada 1 segundo
  if (tiempoActual - ultimaLecturaColor >= INTERVALO_COLOR) {
    leerSensorColor();
    ultimaLecturaColor = tiempoActual;
  }
  
  // Leer sensores cada 10 segundos
  if (tiempoActual - ultimaLecturaSensores >= INTERVALO_SENSORES) {
    imprimirDatosSensores();
    ultimaLecturaSensores = tiempoActual;
  }
  
  delay(10);
}

void leerSensorColor() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  int rojo = pulseIn(OUT, HIGH);

  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  int verde = pulseIn(OUT, HIGH);

  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  int azul = pulseIn(OUT, HIGH);

  colorDetectado = detectarColor(rojo, verde, azul);

  // Solo imprimir si se detectó un color específico
  if (colorDetectado != "Sin color detectado") {
    Serial.println("\n--- Datos del Sensor de Color ---");
    Serial.print("Rojo: "); Serial.println(rojo);
    Serial.print("Verde: "); Serial.println(verde);
    Serial.print("Azul: "); Serial.println(azul);
    Serial.print("Color Detectado: "); Serial.println(colorDetectado);
  }
  
  mostrarColorEnLED(colorDetectado);
}

String detectarColor(int rojo, int verde, int azul) {
  for (int i = 0; i < sizeof(colores) / sizeof(colores[0]); i++) {
    if (rojo >= colores[i].rMin && rojo <= colores[i].rMax &&
        verde >= colores[i].gMin && verde <= colores[i].gMax &&
        azul >= colores[i].bMin && azul <= colores[i].bMax) {
      return colores[i].nombre;
    }
  }
  return "Sin color detectado";
}

void mostrarColorEnLED(String colorDetectado) {
  for (int i = 0; i < sizeof(coloresLED) / sizeof(coloresLED[0]); i++) {
    if (coloresLED[i].nombre == colorDetectado) {
      ledcWrite(ledChannel_R, 255 - coloresLED[i].r);
      ledcWrite(ledChannel_G, 255 - coloresLED[i].g);
      ledcWrite(ledChannel_B, 255 - coloresLED[i].b);
      return;
    }
  }
}

void imprimirDatosSensores() {
  // Leer datos de DHT11
  humedad = dht.readHumidity();
  temperatura = dht.readTemperature();
  
  // Leer datos de BMP180
  sensors_event_t event;
  bmp.getEvent(&event);

  // Verificar cada sensor por separado
  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error: DHT11");
    return;
  }
  if (!event.pressure) {
    Serial.println("Error: BMP180");
    return;
  }

  presion = event.pressure;
  altitud = bmp.pressureToAltitude(1013.25, presion);

  // Imprimir fecha y hora
  DateTime now = rtc.now();
  Serial.print("\n");
  Serial.print(now.day(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.year(), DEC);
  Serial.print("  ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);

  // Imprimir encabezados
  Serial.println("\n-------- DHT11 --------    -------- BMP180 --------");
  
  // Imprimir temperatura y presión
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" °C    ");
  Serial.print("Presión: ");
  Serial.print(presion);
  Serial.println(" hPa");
  
  // Imprimir humedad y altitud
  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.print(" %         ");
  Serial.print("Altitud: ");
  Serial.print(altitud);
  Serial.println(" m");
}
