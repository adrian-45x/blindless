/*
   BASTÓN PARA INVIDENTES
   Código creado por Adrián Morales Alfonso
   Marzo-abril 2020
   IES Vicente aleixandre
*/

//----------SUBSISTEMA ULTRASONIDO----------
//Variables del sensor de distancia
const int triggerPin = 11; // Pin donde conectamos el emisor
const int echoPin = 10; // Pin donde conectamos el receptor
//Variable distancia
float cmUltrasonido = 0;

//buzzer
const int buzzer =  6;   // el numero de pin del zumbador
long intervalBuzzer = 0; //Variable frecuencia del sonido
unsigned long previous_timeBuzzer = 0; //Variable que almacena el tiempo calculado para calcular el siguiente (buzzer)

//----------SUBSISTEMA LÁSER----------
#include <Wire.h>
#include <VL53L0X.h>
VL53L0X sensor;

int n_Samples = 5; // numero de muestras para promediar

//#define LONG_RANGE // Aumenta sensibilidad, +rango, -precision
//#define HIGH_SPEED // Mayor velocidad, menor precision
#define HIGH_ACCURACY // Alta precision, menor velocidad

//variables motor
const int motor = 9; // Pin al que conectamos el motor.
int motorState = 0; // variable para almacenar el estado del motor
long intervalmotor = 0; //Variable frecuencia del sonido
unsigned long previous_timemotor = 0; //Variable que almacena el tiempo calculado para calcular el siguiente (motor)

//Variables distancia
float cmLaser = 0;

//----------SUBSISTEMA ASISTENCIA----------
// Declaramos variables a los pines
const int boton = 4;
const int rojo =  11; //color rojo del led RGB
const int verde =  10; //color verde del led RGB

// Variables que cambian
int buttonState;             // Variable del estado del botón actual
int lastButtonState = LOW;   // Variable del último estado del botón
int rojoState = LOW; //Estado del color rojo
int verdeState = HIGH; //Estado del color verde
int buzzerState = HIGH; //Estado del buzzer

//Tiempo
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
//Variable para almacenar tiempo actual
unsigned long current_time = 0;

void setup() {
  Serial.begin(9600); // Abrimos el puerto serie

  //----------SUBSISTEMA ULTRASONIDO----------
  pinMode(triggerPin, OUTPUT); // Configuramos trigger como salida digital
  pinMode(echoPin, INPUT); // Configuramos echo como entrada digital

  //----------SUBSISTEMA LÁSER----------
  pinMode(motor, OUTPUT); // Declaramos el motor como una salida.
  Wire.begin();
  sensor.init();
  sensor.setTimeout(500);
  //###### Parametros Medida simple ##########
#if defined LONG_RANGE
  // Limite de la tasa de retorno (por defecto 0.25 MCPS)
  sensor.setSignalRateLimit(0.1);
  // Periodo de pulso laser (por defecto 14 y 10 PCLKs)
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  // reduce tiempo estimado a 20 ms (por defecto 33 ms)
  sensor.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  // incrementa tiempo estimado a 200 ms
  sensor.setMeasurementTimingBudget(200000);
#endif

  //----------SUBSISTEMA ASISTENCIA----------
  //Declaramos los pines como salidas o entradas
  pinMode(rojo, OUTPUT);
  pinMode(verde, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(boton, INPUT);
  Serial.begin(9600);
  //fijamos como estado principal el led verde encendido
  digitalWrite(rojo, rojoState);
  digitalWrite(verde, verdeState);
}

void loop() {
  current_time = millis(); //tiempo desde que se inicia el programa

  //Subsistema ultrasonido
  cmUltrasonido = distanciaUltrasonido ();
  subsistemaUltrasonido();

  //Subsistema láser
  cmLaser = distanciaLaser (n_Samples);
  subsistemaLaser();

  //Subsistema asistencia
  subsistemaAsistencia();
}

//--------------------FUNCIONES--------------------

//----------SUBSISTEMA ULTRASONIDO----------
//Función para obtener los cm del ultrasonido
float  distanciaUltrasonido() {
  // Variable para almacenar el tiempo de la onda y la distancia
  float duration, distance;
  //Inicializamos el sensor
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(5);
  // Enviamos una señal activando la salida trigger durante 10 microsegundos
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  // Medimos el ancho del pulso, cuando la lectura sea HIGH
  // devuelve el tiempo transcurrido en microsegundos
  duration = pulseIn(echoPin, HIGH, 15000);
  // Calculamos la distancia en cmUltrasonido y la guardamos en distance
  distance = duration * 0.01715;
  // devolvemos la distancia
  return distance;
  delay(100);
}

//Función que interpreta los cm y los traduce a un buzzer
void subsistemaUltrasonido() {
  intervalBuzzer = map(cmUltrasonido, 5, 245, 0, 1000);

  //frecuencia del pitido
  if (cmUltrasonido > 0) { //Si da valores, el buzzer da sonido
    if (current_time - previous_timeBuzzer > intervalBuzzer) { //se activa y desactiva el pitido con frecuencia de intervalBuzzer
      previous_timeBuzzer = current_time;

      if (buzzerState == 0) {
        buzzerState = 130; //Se activa el buzzer si está desactivado
      }
      else {
        buzzerState = 0; //Se desactiva el buzzer si está activado
      }
      analogWrite(buzzer, buzzerState); //Se cambia al estado del Buzzer
    }
  }
  else {
    analogWrite(buzzer, 0); //se desactiva para que no se quede encendido
  }
}

//----------SUBSISTEMA LÁSER----------
//Función para obtener los cm del láser
float distanciaLaser(int n) { // hacemos "n" mediciones

  float SUMA_n = 0;
  for (int i = 0; i < n; i++) {
    SUMA_n += sensor.readRangeSingleMillimeters();
  }
  return ( SUMA_n / n / 10); // Promedio en centimetros
}

//Función que interpreta los cm y los traduce a un motor vibrador
void subsistemaLaser() {
  if (sensor.timeoutOccurred()) {
    Serial.println(" TIME OUT");
  } else {
    if (cmLaser < 2) Serial.println("Fuera de Rango (d < 2 cm)");
    else if (cmLaser > 220) Serial.println("Fuera de Rango (d > 2 m)");
    else {
      Serial.print(cmLaser , 1); // cmLaser en cmLaser y 1 decimal
      Serial.println(" cmLaser ");
    }
  }
  intervalmotor = map(cmLaser , 10, 200, 0, 1000); // Escalamos los valores.
  if (cmLaser > 0) { //Si da valores, el motor da sonido
    if (current_time - previous_timemotor > intervalmotor) { //se activa y desactiva el pitido con frecuencia de intervalmotor
      previous_timemotor = current_time;
      if (motorState == 0) {
        motorState = 150; //Se activa el motor si está desactivado
      }
      else {
        motorState = 0; //Se desactiva el motor si está activado
      }
      analogWrite(motor, motorState); //Se cambia al estado del motor
    }
  }
  else {
    analogWrite(motor, 0); //se desactiva para que no se quede encendido
  }
}

//----------SUBSISTEMA ASISTENCIA----------
void subsistemaAsistencia() {
  // Lee el estado del botón
  int lectura_boton = digitalRead(boton);

  // Si la  lectura cambia debido al ruido lo ignora
  if (lectura_boton != lastButtonState) {
    // Resetea el tiempo de debounce
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // El boton debe ser presionado más que el debounce

    // Si el estado del botón ha cambiado:
    if (lectura_boton != buttonState) {
      buttonState = lectura_boton;

      // Cambia el estado del led si el estado nuevo del botón es HIGH
      if (buttonState == HIGH) {
        verdeState = !verdeState;
        rojoState = !rojoState;
        buzzerState = !buzzerState;

        if (buzzerState == HIGH) buzzerStateRojo();
        else buzzerStateVerde();
      }
    }
  }

  //Asocia el estado
  digitalWrite(rojo, rojoState);
  digitalWrite(verde, verdeState);


  // Almacena la lectura del botón
  lastButtonState = lectura_boton;
}

void buzzerStateVerde() { //Función de como sonaará el buzzer cuando cambie a verde
  analogWrite(buzzer, 50);
  delay(400);
  analogWrite(buzzer, 0);
}

void buzzerStateRojo() { //Función de como sonaará el buzzer cuando cambie a rojo
  for (int i = 0; i <= 2; i++) {
    analogWrite(buzzer, 150);
    delay(150);
    analogWrite(buzzer, 0);
    delay(150);
  }
}
