
#include <Arduino.h>

#define LED_BUILTIN 2  

#define MODE_MANUAL 0
#define MODE_AUTO   1

<<<<<<<< Updated upstream:smartlamp/smartlamp.ino
int ledPin = 22;
int ledChannel = 0;
int ledValue = 10;
========
int contador_ldr = 0;
int ledPin = 14;
//int ledChannel = 0;
//int ledValue = 40;
>>>>>>>> Stashed changes:new_test/new_test.ino
int ledMode = MODE_MANUAL;

int ldrPin = 14;
int ldrMax = 4000;

int thresholdValue = 50;

void setup() {
    Serial.begin(9600);

<<<<<<<< Updated upstream:smartlamp/smartlamp.ino
    pinMode(LED_BUILTIN, OUTPUT);
========
//    pinMode(LED_BUILTIN, OUTPUT);/
    pinMode(ledPin, OUTPUT);
>>>>>>>> Stashed changes:new_test/new_test.ino
    for (int i=9; i>=0; i--) {
        digitalWrite(LED_BUILTIN, i % 2 ? HIGH : LOW);
        delay(50);
    }

<<<<<<<< Updated upstream:smartlamp/smartlamp.ino
//    ledcSetup(1, 5000, 8);
//    ledcAttachPin(LED_BUILTIN, 1); // Led Builtin aceita PWM no ESP32
    
    pinMode(ledPin, OUTPUT);
//    ledcSetup(ledChannel, 5000, 8);
//    ledcAttachPin(ledPin, ledChannel);
========
    // --- CÓDIGO ATUALIZADO ---
    // Substitui a chamada de ledcSetup e ledcAttachPin
    // ledcAttachChannel(pin, freq, resolution, channel)
//    ledcAttachCh/annel(LED_BUILTIN, 5000, 8, 1);
//    ledcAttachChann/el(ledPin, 5000, 8, ledChannel);
    // -------------------------
>>>>>>>> Stashed changes:new_test/new_test.ino
    
    Serial.printf("DBG SmartLamp Initialized.\n");
}

 

void loop() {
    String serialCommand;

    while (Serial.available() > 0) {
        char serialChar = Serial.read();
        serialCommand += serialChar; 

        if (serialChar == '\n') {
            processCommand(serialCommand);
            serialCommand = "";
        }
    }

    ledUpdate();
    delay(100);
}


void processCommand(String command) {
    command.trim();
    command.toUpperCase();

     Serial.println("DBG Received command: " + command);

    if (command.startsWith("SET_LED")) {
        int ledTmp = command.substring(7).toInt();
        if (ledTmp >= 0 && ledTmp <= 100) {
            ledValue = ledTmp;
            ledMode  = MODE_MANUAL;
            ledUpdate();
            Serial.printf("RES SET_LED 1\n");
        }
        else {
            Serial.printf("RES SET_LED -1\n");
        }
    }
    else if (command.startsWith("SET_THRESHOLD")) {
        int thresholdTmp = command.substring(14).toInt();
        if (thresholdTmp >= 0 && thresholdTmp <= 100) {
            thresholdValue = thresholdTmp;
            ledMode = MODE_AUTO;
            ledUpdate();
            Serial.printf("RES SET_THRESHOLD 1\n");
        }
        else {
            Serial.printf("RES SET_THRESHOLD -1\n");
        }
    }

    else if (command == "GET_LDR")
      Serial.printf("RES GET_LDR %d\n", ldrGetValue());

    else if (command == "GET_LED")
      Serial.printf("RES GET_LED %d\n", ledValue);
    
    else if (command == "GET_THRESHOLD")
      Serial.printf("RES GET_THRESHOLD %d\n", thresholdValue);
    
    else
      Serial.println("ERR Unknown command.");
      
}


void ledUpdate() {
    if (ledMode == MODE_MANUAL || (ledMode == MODE_AUTO && ldrGetValue() < thresholdValue)) {
//        ledcWrite(ledChannel, 255.0*(ledValue/100.0));
//        ledcWrite(1, 255.0*(ledValue/100.0));
          digitalWrite(LED_BUILTIN, HIGH);
    }
    else {
//        ledcWrite(ledChannel, 0);
//        ledcWrite(1, 0);
<<<<<<<< Updated upstream:smartlamp/smartlamp.ino
          digitalWrite(LED_BUILTIN, LOW);
========

>>>>>>>> Stashed changes:new_test/new_test.ino
    }
}


int ldrGetValue() {
    int ldrAnalog = analogRead(ldrPin);
    int ldrValue = 100*ldrAnalog/ldrMax;

    // Serial.printf("DBG LDR_MAX=%d, LDR_ANALOG=%d, LDR_VALUE=%d\n", ldrMax, ldrAnalog, ldrValue);

    return ldrValue > 100 ? 100 : ldrValue;
}
