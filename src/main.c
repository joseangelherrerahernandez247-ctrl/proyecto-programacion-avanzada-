/*
==================================================
Versión actualizada
Primera versión funcional del sistema

Proyecto: Cerradura Digital

Integrantes:
- Jose Angel Herrera Hernandez

Fecha: 15/05/2026

Descripción:
Sistema básico de cerradura digital mediante
comunicación serial. El sistema valida una clave
de acceso y activa un LED cuando la clave es correcta.
==================================================
*/

const int led = 13;

String clave = "1234";
String entrada = "";

void setup() {

  // Configuración del LED
  pinMode(led, OUTPUT);

  // Inicialización de comunicación serial
  Serial.begin(9600);

  Serial.println("=== Cerradura Digital ===");
  Serial.println("Ingrese la clave:");
}

void loop() {

  // Verifica si hay datos disponibles
  if (Serial.available()) {

    char tecla = Serial.read();

    // Cuando el usuario presiona ENTER
    if (tecla == '\n') {

      // Comparación de la clave
      if (entrada == clave) {

        digitalWrite(led, HIGH);

        Serial.println("Acceso concedido");
      }

      else {

        digitalWrite(led, LOW);

        Serial.println("Clave incorrecta");
      }

      // Limpia la entrada
      entrada = "";
    }

    else {

      // Guarda caracteres ingresados
      entrada += tecla;
    }
  }
}