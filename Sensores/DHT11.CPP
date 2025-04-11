#include <Wire.h>
#include <DHT.h>

// Definir el pin de conexión del sensor DHT11
#define DHTPIN 9  // Cambia este pin si necesitas usar otro

// Definir el tipo de sensor DHT (DHT11)
#define DHTTYPE DHT11

// Crear el objeto para el sensor DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Inicializar la comunicación serial
  Serial.begin(115200);
  
  // Inicializar el sensor DHT11
  dht.begin();
  
  Serial.println("Iniciando sensor DHT11...");
}

void loop() {
  // Esperar entre lecturas
  delay(2000);  // El DHT11 puede tomar lecturas cada 2 segundos
  
  // Leer los valores de temperatura y humedad
  float humedad = dht.readHumidity();
  float temperatura = dht.readTemperature();
  
  // Verificar si las lecturas son válidas
  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error al leer del sensor DHT11");
    return;
  }
  
  // Mostrar los resultados por el monitor serial
  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");
}
