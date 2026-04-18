/*
--------------------------------------------------
Proyecto: Cerradura Digital
Versión: 1.0 (Versión actualizada)
--------------------------------------------------
*/

const int led = 13;
String clave = "1234";
String entrada = "";

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    char tecla = Serial.read();
    
    if (tecla == '\n') {
      if (entrada == clave) {
        digitalWrite(led, HIGH);
        Serial.println("Acceso concedido");
      } else {
        digitalWrite(led, LOW);
        Serial.println("Clave incorrecta");
      }
      entrada = "";
    } else {
      entrada += tecla;
    }
  }
}