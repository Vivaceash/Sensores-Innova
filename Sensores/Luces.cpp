#include <Arduino.h>

// Pines del sensor GY-31 (TCS3200/TCS230)
#define S0 14
#define S1 15
#define S2 16
#define S3 17
#define OUT 18

// Pines para el LED RGB
#define LED_R 4
#define LED_G 5
#define LED_B 13

// Tolerancia para detección
#define TOLERANCIA 10

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


// Funciones
String detectarColor(int rojo, int verde, int azul);
void mostrarColorEnLED(String colorDetectado);

// Variables para PWM
const int ledChannel_R = 0;  // Canal de PWM para el LED rojo
const int ledChannel_G = 1;  // Canal de PWM para el LED verde
const int ledChannel_B = 2;  // Canal de PWM para el LED azul

void setup() {
  Serial.begin(115200);

  // Configurar los pines del sensor
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);

  // Configurar los pines del LED
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Configurar los canales PWM para los LEDs
  ledcSetup(ledChannel_R, 5000, 8); // Canal 0, frecuencia de 5kHz, resolución de 8 bits
  ledcSetup(ledChannel_G, 5000, 8); // Canal 1
  ledcSetup(ledChannel_B, 5000, 8); // Canal 2

  // Asignar los canales a los pines de salida
  ledcAttachPin(LED_R, ledChannel_R);
  ledcAttachPin(LED_G, ledChannel_G);
  ledcAttachPin(LED_B, ledChannel_B);

  Serial.println("Comenzando lectura de colores...");
}

void loop() {
  // Lectura del sensor
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  int rojo = pulseIn(OUT, HIGH);

  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  int verde = pulseIn(OUT, HIGH);

  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  int azul = pulseIn(OUT, HIGH);

  // Detectar color
  String colorDetectado = detectarColor(rojo, verde, azul);

  // Mostrar resultados
  Serial.print("Rojo: "); Serial.println(rojo);
  Serial.print("Verde: "); Serial.println(verde);
  Serial.print("Azul: "); Serial.println(azul);
  Serial.print("Color Detectado: "); Serial.println(colorDetectado);
  Serial.println("--------------");

  // Controlar LED
  mostrarColorEnLED(colorDetectado);

  delay(1000);
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
      ledcWrite(ledChannel_R, 255 - coloresLED[i].r); // Canal para el LED rojo
      ledcWrite(ledChannel_G, 255 - coloresLED[i].g); // Canal para el LED verde
      ledcWrite(ledChannel_B, 255 - coloresLED[i].b); // Canal para el LED azul      
      return;
    }
  }
}
