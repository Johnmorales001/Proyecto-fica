#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BluetoothSerial.h>
#include <Keypad.h>

// Configuración de la pantalla OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configuración de Bluetooth
BluetoothSerial SerialBT;

const uint8_t ROWS = 4;
const uint8_t COLS = 4;
int tecla = 0;
char keys[ROWS][COLS] = {
  {'1', '2', '3', '<'},
  {'4', '5', '6', '='},
  {'7', '8', '9', '>'},
  {':', '0', ';', '?' }
};
uint8_t colPins[COLS] = { 23, 19, 18, 5 };
uint8_t rowPins[ROWS] = { 12, 4, 2, 15 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int numero = 0, num = 0, opera = 0, aux = 0;
int error, clave = 1245, armado = 0, salir = 0;
int led_armado = 25;
int sirena = 14;
int s1 = 34, s2 = 35, s3 = 32, s4 = 33, s11, s22, s33, s44, alarma = 0;

void setup() {
  Serial.begin(9600);
  SerialBT.begin("ESP32_BT");
  Serial.println("Bluetooth Iniciado");
  
  Wire.begin(22, 21); // SDA = 22, SCL = 21
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error al inicializar la pantalla OLED"));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  pinMode(led_armado, OUTPUT);
  pinMode(led_desarmado, OUTPUT);
  pinMode(sirena, OUTPUT);
  pinMode(s1, INPUT_PULLUP);
  pinMode(s2, INPUT_PULLUP);
  pinMode(s3, INPUT_PULLUP);
  pinMode(s4, INPUT_PULLUP);
  digitalWrite(led_armado, LOW);
  digitalWrite(led_desarmado, HIGH);
  digitalWrite(sirena, LOW);

  menu();
}

void loop() {
  if (Serial.available()) {
    String receivedData = Serial.readStringUntil('\n');  // Lee los datos hasta el salto de línea
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Tecla presionada: ");
    display.print(receivedData);  // Muestra los datos recibidos en la pantalla OLED
    display.display();
    delay(100);  // Se realizó un pequeño retardo para permitir la actualización de la pantalla
    armado = !armado; 
    if (receivedData == "LED_ON") {
    digitalWrite(led_armado, HIGH);       
    digitalWrite(led_desarmado, LOW);     
} else if (receivedData == "LED_OFF") {
    digitalWrite(led_armado, LOW);       
    digitalWrite(led_desarmado, HIGH);    
}
  }
  tecla = 16;
  int key = keypad.getKey();
  if (key) { 
    tecla = key - '0';
    Serial.print("Tecla presionada: ");
    Serial.println(key);
  }

  // Se inicializa los casos para el menú de opciones
  switch (tecla) {
    case 1:
      seguridad();
      salir = 0;
      numero = 0;
      num = 0;
      while (salir == 0) {
        tecla = 16;
        int key = keypad.getKey();
        if (key) { 
          tecla = key - '0';
          Serial.print("Tecla de seguridad presionada: ");
          Serial.println(key);
        }
        if (tecla == 10) { salir = 1; }
        seguridad_code(); // Se llama a la función para manejar la seguridad
      }
      menu();
      delay(1000);
      tecla = 16;
      break;

    case 2:
      calculadora();
      salir = 0;
      numero = 0;
      num = 0;
      opera = 0;
      aux = 0;
      while (salir == 0) {
        tecla = 16;
        int key = keypad.getKey();
        if (key) { 
          tecla = key - '0';
          Serial.print("Tecla de calculadora presionada: ");
          Serial.println(key);
        }
        if (tecla == 10) { salir = 1; }

        if (tecla >= 0 && tecla <= 9) {
          numero = numero * 10 + tecla;
          if (opera == 0) {
            display.setCursor(0, 16);
          } else {
            display.setCursor(50, 16);
          }
          display.print(numero);
          display.display();
          Serial.print("Número ingresado: ");
          Serial.println(numero);
        }

        // Operación de suma
        if (tecla == 12) {
          opera = 1;
          aux = numero;
          numero = 0;
          display.setCursor(100, 16); 
          display.print("+");
          display.display();
          Serial.println("Operación: Suma");
        }

        // Operación de resta
        if (tecla == 13) {  // Tecla '=' para la resta
          opera = 2;
          aux = numero;
          numero = 0;
          display.setCursor(100, 16);
          display.print("-");
          display.display();
          Serial.println("Operación: Resta");
        }

        // Operación de multiplicación
        if (tecla == 14) {  // Tecla '>' para la multiplicación
          opera = 3;
          aux = numero;
          numero = 0;
          display.setCursor(100, 16);
          display.print("*");
          display.display();
          Serial.println("Operación: Multiplicación");
        }

        // Operación de división
        if (tecla == 15) {  // Tecla '?' para la división
          opera = 4;
          aux = numero;
          numero = 0;
          display.setCursor(100, 16);
          display.print("/");
          display.display();
          Serial.println("Operación: División");
        }

        // Calcula el resultado
        if (tecla == 11) {  // Se presiona la tecla ';' para confirmar
          if (opera == 1) { numero = aux + numero; }
          if (opera == 2) { numero = aux - numero; }
          if (opera == 3) { numero = aux * numero; }
          if (opera == 4) { 
            if (numero != 0) {
              numero = aux / numero; 
            } else {
              display.clearDisplay();
              display.setCursor(0, 16);
              display.print("Error: Div/0");
              display.display();
              Serial.println("Error: División entre 0");
              delay(2000);
              display.clearDisplay();
              calculadora();
              continue;
            }
          }

          display.clearDisplay();
          display.setCursor(0, 16);
          display.print("Resultado: ");
          display.print(numero);
          display.display();
          Serial.print("Resultado: ");
          Serial.println(numero);

          numero = 0;
          aux = 0;
          opera = 0;

          delay(2000);
          display.clearDisplay();
          calculadora();
        }
      }
      menu();
      delay(1000);
      tecla = 16;
      break;

    case 3:
      presentacion();
      delay(5000); // Se muestra el mensaje por 5 segundos
      menu();
      tecla = 16;
      break;
  }
}
 //Menu de opciones
void menu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Menu Principal:");
  display.setCursor(0, 10);
  display.print("1. Seguridad");
  display.setCursor(0, 20);
  display.print("2. Calculadora");
  display.setCursor(0, 30);
  display.print("3. Presentacion");
  display.display();
}

void calculadora() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Calculadora:");
  display.display();
}

void seguridad() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Acceso - Alarma");
  display.display();
}

void presentacion() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Hola! este programa");
  display.setCursor(0, 10);
  display.print("fue hecho por");
  display.setCursor(0, 20);
  display.print("John Morales &");
  display.setCursor(0, 30);
  display.print("Nathalia Maldonado");
  display.display();
}

void seguridad_code() {
  if (tecla < 10 && num < 4) { // Solo se permiten hasta 4 dígitos
    num++;
    if (num == 1) { 
      numero = tecla; 
    } else {
      numero = numero * 10 + tecla;
    }

    display.clearDisplay();
    display.setCursor(0, 16);
    display.print("Clave: ");
    display.print(numero);
    display.display();

    // Refleja el mensaje cuando se han alcanzado los 4 dígitos
    if (num == 4) {
      display.setCursor(0, 30);
      display.print("Presione ';' para confirmar");
      display.display();
    }
    Serial.print("Número ingresado: ");
    Serial.println(numero);
  }

  if (tecla == 11 && num == 4) { // Confirmación de la clave al presionar ';' y solo si se ingresaron 4 dígitos
    if (numero == clave) {
      display.clearDisplay();
      display.setCursor(0, 16);
      display.print("Clave correcta");
      display.display();
      Serial.println("Clave correcta");

      armado = !armado; // Cambia el estado de armado
      if (armado == 1) {
        display.setCursor(0, 26);
        display.print("Alarma ARMADA");
        digitalWrite(led_armado, HIGH); // Enciende LED de armado
        digitalWrite(led_desarmado, LOW); 
        digitalWrite(sirena, HIGH); // Si quieres activar la sirena al armar
      } else {
        display.setCursor(0, 26);
        display.print("Alarma DESARMADA");
        digitalWrite(led_armado, LOW); // 
        digitalWrite(led_desarmado, HIGH); 
        digitalWrite(sirena, LOW); // Desactiva la sirena al desarmar
      }
      display.display();
    } else {
      display.clearDisplay();
      display.setCursor(0, 16);
      display.print("Clave incorrecta");
      display.display();
      Serial.println("Clave incorrecta");

      delay(1000);
    }


    num = 0; // Reinicia el contador de dígitos
    numero = 0; // Reinicia el número ingresado
    delay(2000);
  }
}