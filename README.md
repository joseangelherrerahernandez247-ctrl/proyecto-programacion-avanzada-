Cerradura Digital 
Proyecto de cerradura digital funcional.

Descripción
Este proyecto consiste en una cerradura digital básica que permite ingresar una clave para activar un LED o servo.

Funcionamiento
El usuario ingresa una clave.
El sistema la compara.
Si es correcto, se activa el LED.
Si es incorrecto, no hace nada.
Uso del arduino mega
Conectar el arduino mega al equipo
Cargar el programa
Abrir el monitor serial
Escribir la clave: 1234
Presidente Entrar
Requisitos
Arduino mega 
CONDUJO
Resistencias
Cables
Ejecución
Cargar el código al dispositivo
Abrir el monitor serial
Ingresar la clave (ejemplo: 1234)
Evidencia
Las fotos del montaje están en la carpeta docs/.
Codigo final mejorado y corregido
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>
#include <RTClib.h>

// LCD I2C

LiquidCrystal_I2C lcd(0x27, 16, 2);

// RTC DS3231

RTC_DS3231 rtc;

// MICRO SD

#define SD_CS 53

// SERVO

Servo servo1;

// RFID RC522

#define RFID_SS_PIN 48
#define RST_PIN 49

MFRC522 rfid(RFID_SS_PIN, RST_PIN);

// Tarjeta RFID correcta
String tarjetaCorrecta = "63C16034";
String nombreTarjeta = "Equipo 10";

// TECLADO 4x4

const byte FILAS = 4;
const byte COLUMNAS = 4;

char teclas[FILAS][COLUMNAS] =
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pinesFilas[FILAS] = {2,3,4,5};
byte pinesColumnas[COLUMNAS] = {6,7,8,9};

Keypad teclado = Keypad(
  makeKeymap(teclas),
  pinesFilas,
  pinesColumnas,
  FILAS,
  COLUMNAS
);


// USUARIOS Y CONTRASEÑAS
String nombres[4] = {
  "Emmanuel",
  "Jose Chikis",
  "Zuriel",
  "Javier"
};

String claves[4] = {
  "2002",
  "2006",
  "1316",
  "2005"
};

String input = "";

// ESTADO PUERTA
bool puertaAbierta = false;

// TIEMPO AUTOMATICO

unsigned long tiempoApertura = 0;
const unsigned long tiempoLimite = 15000;

// LEDS
int ledVerde = 12;
int ledRojo = 13;

// INTENTOS
int intentos = 0;

// SETUP
void setup()
{
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  servo1.attach(11);
  servo1.write(0);

  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);

  digitalWrite(ledRojo, HIGH);
  digitalWrite(ledVerde, LOW);

  SPI.begin();

  // RFID
  rfid.PCD_Init();

  // RTC
  if(!rtc.begin())
  {
    lcd.clear();
    lcd.print("ERROR RTC");
    delay(2000);
  }

  // SD
  if(!SD.begin(SD_CS))
  {
    lcd.clear();
    lcd.print("ERROR SD");
    delay(2000);
  }
  else
  {
    guardarRegistro("Sistema iniciado");
  }

  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("BIENVENIDO");

  lcd.setCursor(1,1);
  lcd.print("PUERTA RFID");

  delay(2500);

  mostrarInicio();
}

// LOOP

void loop()
{
  leerRFID();

  if(puertaAbierta)
  {
    if(millis() - tiempoApertura >= tiempoLimite)
    {
      cerrarPuerta();
    }
  }

  char tecla = teclado.getKey();

  if(tecla)
  {
    Serial.println(tecla);

    if(!puertaAbierta)
    {
      if(tecla == '*')
      {
        input = "";
        lcd.setCursor(0,1);
        lcd.print("                ");
        return;
      }

      if(tecla == 'A' || tecla == 'B' || tecla == 'C' || tecla == 'D' || tecla == '#')
      {
        return;
      }

      if(input.length() < 4)
      {
        lcd.setCursor(input.length(),1);
        lcd.print(tecla);
        input += tecla;
      }

      if(input.length() == 4)
      {
        delay(300);

        int usuario = buscarUsuario(input);

        if(usuario != -1)
        {
          intentos = 0;

          String evento = "Acceso correcto - ";
          evento += nombres[usuario];
          evento += " - Keypad";

          guardarRegistro(evento);

          abrirPuerta(nombres[usuario]);
        }
        else
        {
          intentos++;

          String evento = "Acceso denegado - Clave incorrecta: ";
          evento += input;

          guardarRegistro(evento);

          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("CLAVE ERRONEA");

          lcd.setCursor(0,1);
          lcd.print("INTENTO ");
          lcd.print(intentos);

          delay(2000);

          if(intentos >= 3)
          {
            guardarRegistro("Sistema bloqueado por 3 intentos");
            bloqueoSistema();
          }

          input = "";
          mostrarInicio();
        }
      }
    }
  }
}

// BUSCAR USUARIO POR CLAVE

int buscarUsuario(String clave)
{
  for(int i = 0; i < 4; i++)
  {
    if(clave == claves[i])
    {
      return i;
    }
  }

  return -1;
}


// LEER RFID
void leerRFID()
{
  if(!rfid.PICC_IsNewCardPresent())
  {
    return;
  }

  if(!rfid.PICC_ReadCardSerial())
  {
    return;
  }

  String uid = "";

  for(byte i = 0; i < rfid.uid.size; i++)
  {
    if(rfid.uid.uidByte[i] < 0x10)
    {
      uid += "0";
    }

    uid += String(rfid.uid.uidByte[i], HEX);
  }

  uid.toUpperCase();

  Serial.print("UID: ");
  Serial.println(uid);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("TARJETA RFID");

  lcd.setCursor(0,1);
  lcd.print(uid);

  delay(1500);

  if(uid == tarjetaCorrecta)
  {
    String evento = "Acceso correcto - ";
    evento += nombreTarjeta;
    evento += " - RFID";

    guardarRegistro(evento);

    abrirPuerta(nombreTarjeta);
  }
  else
  {
    String evento = "Acceso denegado - RFID incorrecto: ";
    evento += uid;

    guardarRegistro(evento);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ACCESO");

    lcd.setCursor(0,1);
    lcd.print("DENEGADO");

    delay(2000);

    mostrarInicio();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}


// ABRIR PUERTA

void abrirPuerta(String nombre)
{
  puertaAbierta = true;
  tiempoApertura = millis();

  digitalWrite(ledRojo, LOW);
  digitalWrite(ledVerde, HIGH);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("BIENVENIDO");

  lcd.setCursor(0,1);
  lcd.print(nombre);

  delay(1500);

  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("ABRIENDO");

  servo1.write(90);

  delay(2000);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("PUERTA ABIERTA");

  lcd.setCursor(0,1);
  lcd.print("CIERRA EN 15s");

  input = "";
}

// CERRAR PUERTA

void cerrarPuerta()
{
  guardarRegistro("Puerta cerrada automaticamente");

  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("CERRANDO");

  servo1.write(0);

  digitalWrite(ledVerde, LOW);
  digitalWrite(ledRojo, HIGH);

  delay(2000);

  puertaAbierta = false;

  mostrarInicio();
}

// MOSTRAR INICIO
void mostrarInicio()
{
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("INGRESE CLAVE");

  lcd.setCursor(0,1);
  lcd.print("                ");

  input = "";
}

// BLOQUEO

void bloqueoSistema()
{
  for(int i = 10; i > 0; i--)
  {
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("SISTEMA");

    lcd.setCursor(0,1);
    lcd.print("BLOQUEADO ");
    lcd.print(i);

    delay(1000);
  }

  intentos = 0;

  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("RESTAURANDO");

  delay(2000);

  mostrarInicio();
}

// GUARDAR REGISTRO EN SD
void guardarRegistro(String evento)
{
  File archivo = SD.open("registro.txt", FILE_WRITE);

  if(archivo)
  {
    DateTime now = rtc.now();

    archivo.print(now.day());
    archivo.print("/");
    archivo.print(now.month());
    archivo.print("/");
    archivo.print(now.year());
    archivo.print(" ");

    if(now.hour() < 10) archivo.print("0");
    archivo.print(now.hour());
    archivo.print(":");

    if(now.minute() < 10) archivo.print("0");
    archivo.print(now.minute());
    archivo.print(":");

    if(now.second() < 10) archivo.print("0");
    archivo.print(now.second());

    archivo.print(" - ");
    archivo.println(evento);

    archivo.close();
  }
  else
  {
    Serial.println("No se pudo abrir registro.txt");
  }
}

Integrantes
José Ángel Herrera Hernández
Emmanuel Llanos Andrade
Javier Martín Sánchez Flores
Leonel Hernández Dimas
Zuriel Ignacio Delgado Islas
Michelle Daniela Torres Durant 
