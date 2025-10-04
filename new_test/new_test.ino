#define MODE_MANUAL 0
#define MODE_AUTO   1

int contador_ldr = 0;
int ledPin = 23;
int ledChannel = 0;
int ledMode = MODE_MANUAL;

int ldrPin = 34;
int ldrMax = 4000;

int thresholdValue = 50;

void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledPin, OUTPUT);

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
  contador_ldr++;
  if (contador_ldr == 20) {
    Serial.printf("RES GET_LDR %d\n", ldrGetValue());
    contador_ldr = 0;
  }

}


void processCommand(String command) {
  command.trim();
  command.toUpperCase();

  // Serial.println("DBG Received command: " + command);

  if (command.startsWith("SET_LED ")) {
    int ledTmp = command.substring(8).toInt();
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
  else if (command.startsWith("SET_THRESHOLD ")) {
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
    // Liga o LED
    digitalWrite(ledPin, HIGH); 
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
    // Desliga o LED
    digitalWrite(ledPin, LOW); 
  }
}


int ldrGetValue() {
  int ldrAnalog = analogRead(ldrPin);
  int ldrValue = 100 * ldrAnalog / ldrMax;

  // Serial.printf("DBG LDR_MAX=%d, LDR_ANALOG=%d, LDR_VALUE=%d\n", ldrMax, ldrAnalog, ldrValue);

  return ldrValue > 100 ? 100 : ldrValue;
}
