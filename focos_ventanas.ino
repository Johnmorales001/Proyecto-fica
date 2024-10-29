#include "DHT.h"
#include "BluetoothSerial.h"

// Configuración del DHT 11
#define DHTPIN 26    // Pin del sensor
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

// Configuración del Bluetooth
String device_name = "ESP32-prueba";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run make menuconfig to and enable it
#endif

// Verifica si el perfil SPP está habilitado
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

// Variables de los focos
bool lightState1 = false; 
bool lightState2 = false; 
bool lightState3 = false; 
bool lightState4 = false; 

// Pines de los pulsadores
const int buttonPin1 = 0;
const int buttonPin2 = 4;
const int buttonPin3 = 27;
const int buttonPin4 = 33;

// Variables que rastrea el estado anterior de los pulsadores
bool lastButtonState1 = HIGH;
bool lastButtonState2 = HIGH;
bool lastButtonState3 = HIGH;
bool lastButtonState4 = HIGH;

// Variables para el manejo de puertas y ventanas
bool ventana1Status = false;
bool ventana2Status = false;
bool puerta1Status = false;
bool puerta2Status = false;

// Variables de la entrada serial
String inputString = "";         // String que lee los datos entrantes
bool stringComplete = false;     // Cuando el string está completo

// Variables para el manejo del tiempo
unsigned long lastDHTReadTime = 0;
const unsigned long DHTReadInterval = 2000;  // Intervalo de 2 segundos para leer el DHT

void setup() {
  Serial.begin(115200);
  SerialBT.begin(device_name);  // Nombre del dispositivo Bluetooth
  Serial.printf("El dispositivo con nombre \"%s\" está iniciado.\nAhora puedes emparejarlo con Bluetooth!\n", device_name.c_str());

  // Configurar pines de focos como salida
  pinMode(18, OUTPUT); digitalWrite(18, LOW);
  pinMode(13, OUTPUT); digitalWrite(13, LOW);
  pinMode(14, OUTPUT); digitalWrite(14, LOW);
  pinMode(15, OUTPUT); digitalWrite(15, LOW);

  // Configurar pines de pulsadores como entrada
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(buttonPin4, INPUT_PULLUP);

  // Configurar pines para ventanas y puertas como entrada
  pinMode(32, INPUT_PULLUP);  // Ventana 1
  pinMode(25, INPUT_PULLUP);  // Ventana 2
  pinMode(21, INPUT_PULLUP);  // Puerta 1
  pinMode(19, INPUT_PULLUP);  // Puerta 2

  // Inicializar el DHT 11
  dht.begin();
}

void loop() {
  // Control de focos mediante los pulsadores
  bool currentButtonState1 = digitalRead(buttonPin1);
  bool currentButtonState2 = digitalRead(buttonPin2);
  bool currentButtonState3 = digitalRead(buttonPin3);
  bool currentButtonState4 = digitalRead(buttonPin4);

  if (currentButtonState1 == LOW && lastButtonState1 == HIGH) {
    lightState1 = !lightState1;
    digitalWrite(18, lightState1 ? HIGH : LOW);
    SerialBT.println(lightState1 ? "1" : "0"); // Enviar estado del foco 1 por Bluetooth
    delay(50);  // Anti-rebote
  }

  if (currentButtonState2 == LOW && lastButtonState2 == HIGH) {
    lightState2 = !lightState2;
    digitalWrite(13, lightState2 ? HIGH : LOW);
    SerialBT.println(lightState2 ? "2" : "3"); // Enviar estado del foco 2 
    delay(50);
  }

  if (currentButtonState3 == LOW && lastButtonState3 == HIGH) {
    lightState3 = !lightState3;
    digitalWrite(14, lightState3 ? HIGH : LOW);
    SerialBT.println(lightState3 ? "4" : "5"); // Enviar estado del foco 3 
    delay(50);
  }

  if (currentButtonState4 == LOW && lastButtonState4 == HIGH) {
    lightState4 = !lightState4;
    digitalWrite(15, lightState4 ? HIGH : LOW);
    SerialBT.println(lightState4 ? "6" : "7"); // Enviar estado del foco 4
    delay(50);
  }

  // Actualiza los estados anteriores de los pulsadores
  lastButtonState1 = currentButtonState1;
  lastButtonState2 = currentButtonState2;
  lastButtonState3 = currentButtonState3;
  lastButtonState4 = currentButtonState4;

  // Control de focos mediante Bluetooth
  if (SerialBT.available()) {
    char x = (char)SerialBT.read();
    switch (x) {
      case 'A': digitalWrite(18, HIGH); SerialBT.println("1"); break;
      case 'B': digitalWrite(18, LOW); SerialBT.println("0"); break;
      case 'C': digitalWrite(13, HIGH); SerialBT.println("2"); break;
      case 'D': digitalWrite(13, LOW); SerialBT.println("3"); break;
      case 'E': digitalWrite(14, HIGH); SerialBT.println("4"); break;
      case 'F': digitalWrite(14, LOW); SerialBT.println("5"); break;
      case 'G': digitalWrite(15, HIGH); SerialBT.println("6"); break;
      case 'H': digitalWrite(15, LOW); SerialBT.println("7"); break;
    }
  }

  // Monitoreo de puertas y ventanas con envío de estado por Bluetooth
  if (digitalRead(32) == LOW && !ventana1Status) {
    SerialBT.println("8"); // Ventana 1 se abre
    ventana1Status = true;
  } else if (digitalRead(32) == HIGH && ventana1Status) {
    SerialBT.println("9"); // Ventana 1 se cierra
    ventana1Status = false;
  }

  if (digitalRead(25) == LOW && !ventana2Status) {
    SerialBT.println("10"); // Ventana 2 se abre
    ventana2Status = true;
  } else if (digitalRead(25) == HIGH && ventana2Status) {
    SerialBT.println("11"); // Ventana 2 se cierra
    ventana2Status = false;
  }

  if (digitalRead(21) == LOW && !puerta1Status) {
    SerialBT.println("12"); // Puerta 1 se abre
    puerta1Status = true;
  } else if (digitalRead(21) == HIGH && puerta1Status) {
    SerialBT.println("13"); // Puerta 1 se cierra
    puerta1Status = false;
  }

  if (digitalRead(19) == LOW && !puerta2Status) {
    SerialBT.println("14"); // Puerta 2 se abre
    puerta2Status = true;
  } else if (digitalRead(19) == HIGH && puerta2Status) {
    SerialBT.println("15"); // Puerta 2 se cierra
    puerta2Status = false;
  }

  // Lectura de la humedad y temperatura del DHT 11 cada 2 segundos
  unsigned long currentMillis = millis();
  if (currentMillis - lastDHTReadTime >= DHTReadInterval) {
    lastDHTReadTime = currentMillis;

    float t = dht.readTemperature();  // Lee la temperatura
    float h = dht.readHumidity();     // Lee la humedad

    // Verifica si las lecturas son válidas
    if (isnan(t) || isnan(h)) {
      Serial.println("Error al leer del DHT");
    } else {
      // Envia los datos por Bluetooth
      SerialBT.print("Temp:");
      SerialBT.print(t);
      SerialBT.print("; Hum:");
      SerialBT.print(h);
      SerialBT.println(";");

      // Se imprime en el monitor serial para depuración
      Serial.print("Temp: ");
      Serial.print(t);
      Serial.print("°C, Hum: ");
      Serial.print(h);
      Serial.println("%");
    }
  }
}
