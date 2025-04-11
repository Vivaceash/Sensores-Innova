#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

// Definir el objeto para el sensor BMP180
Adafruit_BMP085_Unified bmp;

// Función de configuración
void setup() {
  Serial.begin(115200); // Inicia la comunicación serial a 115200 baudios

  // Inicializar la comunicación I2C
  Wire.begin(21, 19); // SDA en GPIO 21, SCL en GPIO 19

  // Inicializar el sensor BMP180
  if (!bmp.begin()) {
    Serial.println("No se pudo encontrar el sensor BMP180");
    while (1);
  }

  Serial.println("Sensor BMP180 inicializado correctamente");
}

// Función loop para monitoreo constante
void loop() {
  // Variables para los datos de temperatura y presión
  sensors_event_t event;
  bmp.getEvent(&event); // Obtener el evento de presión

  if (event.pressure) {
    // Obtener la temperatura
    float temperatura;
    bmp.getTemperature(&temperatura);

    // Imprimir los datos en la consola serial
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");

    Serial.print("Presión: ");
    Serial.print(event.pressure);
    Serial.println(" hPa");

    // Establecer la presión a nivel del mar (en hPa) estándar
    float presionNivelDelMar = 1013.25;

    // Calcular la altitud
    float altitud = bmp.pressureToAltitude(presionNivelDelMar, event.pressure);

    Serial.print("Altitud estimada: ");
    Serial.print(altitud);
    Serial.println(" metros");

    Serial.println("--------------");
  } else {
    Serial.println("Error leyendo los datos del BMP180");
  }

  delay(2000); // Esperar 2 segundos antes de la siguiente lectura
}
