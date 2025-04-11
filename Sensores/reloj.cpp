#include <Wire.h>
#include <RTClib.h>

// Crear una instancia del RTC
RTC_DS3231 rtc;

void setup() {
  Serial.begin(115200);

  // Iniciar la comunicación I2C en los pines GPIO 1 y GPIO 2
  Wire.begin(2, 1); // SDA en GPIO 2, SCL en GPIO 1

  // Comprobar si el RTC está conectado
  if (!rtc.begin()) {
    Serial.println("No se pudo encontrar el RTC");
    while (1);
  }

  // Comprobar si el RTC está funcionando correctamente
  if (rtc.lostPower()) {
    Serial.println("RTC perdió alimentación, ajustando la hora...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Ajustar la fecha y hora al momento de la compilación
  }
}

void loop() {
  // Obtener la hora y fecha actual
  DateTime now = rtc.now();

  // Imprimir la hora en el monitor serial
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  delay(1000);  // Esperar un segundo
}
