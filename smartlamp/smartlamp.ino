
#include <Arduino.h>
#include <HardwareSerial.h>

// rotina do ldr (millis)
unsigned long lastReading = 0;
const unsigned long interval = 2000;
const unsigned long interval_main = 1/00;

#define LED_BUILTIN 2

#define MODE_MANUAL 0
#define MODE_AUTO 1
#define RST_LED 2

int ledPin = 14;
// int ledChannel = 0;
int ledValue = 0;
int ledMode = MODE_MANUAL;

int ldrPin = 34;
int ldrMax = 4000;

int thresholdValue = 50;

void setup()
{
    Serial.begin(9600);
    Serial.setRxBufferSize(256);

    pinMode(LED_BUILTIN, OUTPUT);
    for (int i = 9; i >= 0; i--)
    {
        digitalWrite(LED_BUILTIN, i % 2 ? HIGH : LOW);
        delay(100);
    }

    //    ledcSetup(1, 5000, 8);
    //    ledcAttachPin(LED_BUILTIN, 1); // Led Builtin aceita PWM no ESP32

    pinMode(ledPin, OUTPUT);
    //    ledcSetup(ledChannel, 5000, 8);
    //    ledcAttachPin(ledPin, ledChannel);

    pinMode(ldrPin, INPUT);

    Serial.printf("DBG SmartLamp Initialized.\n");
}

void loop()
{
    static char serialBuffer[64];
    static int bufferIndex = 0;

    String serialCommand;

    ledValue = digitalRead(ledPin);


    //para tirar o delay, tive que trabalhar no buffer pra receber os comandos, pois o serial não tava conseguindo ler

    while (Serial.available() > 0) {
        char serialChar = Serial.read();
        if (serialChar == '\n' || serialChar == '\r') { 
            if (bufferIndex > 0) { 
                serialBuffer[bufferIndex] = '\0'; 
                
                processCommand(serialBuffer);
                bufferIndex = 0; // Reseta o buffer
            }
        } else if (bufferIndex < sizeof(serialBuffer) - 1) {
            serialBuffer[bufferIndex++] = serialChar;
        }
    }

    
//    while (Serial.available() > 0)
//    {
//        char serialChar = Serial.read();
//        serialCommand += serialChar;
//
//        if (serialChar == '\n')
//        {
//            processCommand(serialCommand);
//            serialCommand = "";
//        }
//    }



    // ldr de 2 em 2 segundos (millis)
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastReading >= interval)
    {
        lastReading = currentMillis;
        Serial.printf("LDR_VALUE PERIOD: %d\n", ldrGetValue());
    }

    ledUpdate();

    
//    delay(100);/
}



void processCommand(String command)
{
    command.trim();
    command.toUpperCase();

    Serial.println("DBG Received command: " + command);

    if (command.startsWith("SET_LED"))
    {
        int ledTmp = command.substring(7).toInt();
        if (ledTmp >= 0 && ledTmp <= 100)
        {
            ledValue = ledTmp;
            ledMode = MODE_MANUAL;
            ledUpdate();
            Serial.printf("RES SET_LED 1\n");
        }
        else
        {
            Serial.printf("RES SET_LED -1\n");
        }
    }

    else if (command.startsWith("SET_THRESHOLD"))
    {
        int thresholdTmp = command.substring(14).toInt();
        if (thresholdTmp >= 0 && thresholdTmp <= 100)
        {
            thresholdValue = thresholdTmp;
            ledMode = MODE_AUTO;
            ledUpdate();
            Serial.printf("RES SET_THRESHOLD 1\n");
        }
        else
        {
            Serial.printf("RES SET_THRESHOLD -1\n");
        }
    }

    else if (command == "RST_LED")
    {
        ledMode = RST_LED;
        ledUpdate();
        Serial.printf("RES GET_LED %d\n", ledValue);
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

void ledUpdate()
{
    if (ledMode == MODE_MANUAL || (ledMode == MODE_AUTO && ldrGetValue() < thresholdValue))
    {
        //        ledcWrite(ledChannel, 255.0*(ledValue/100.0));
        //        ledcWrite(1, 255.0*(ledValue/100.0));
        digitalWrite(ledPin, HIGH);
    }
    else
    {
        //        ledcWrite(ledChannel, 0);
        //        ledcWrite(1, 0);
        digitalWrite(ledPin, LOW);
    }
}

int ldrGetValue()
{
    int ldrAnalog = analogRead(ldrPin);
    int ldrValue = 100 * ldrAnalog / ldrMax;

    // Serial.printf("DBG LDR_MAX=%d, LDR_ANALOG=%d, LDR_VALUE=%d\n", ldrMax, ldrAnalog, ldrValue);

    return ldrValue > 100 ? 100 : ldrValue;
}
