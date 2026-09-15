// ESP32 LED Controller - Simple Serial Protocol
// Upload this to your ESP32

const int LED_PIN = 2;  // D2 pin

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("ESP32 LED Controller Ready");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "LED_ON") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("OK:LED_ON");
    }
    else if (command == "LED_OFF") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("OK:LED_OFF");
    }
    else if (command.startsWith("BLINK:")) {
      int times = command.substring(6).toInt();
      if (times <= 0) times = 3;
      for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(300);
        digitalWrite(LED_PIN, LOW);
        delay(300);
      }
      Serial.println("OK:BLINK:" + String(times));
    }
    else if (command == "STATUS") {
      int state = digitalRead(LED_PIN);
      Serial.println("OK:STATUS:" + String(state == HIGH ? "ON" : "OFF"));
    }
    else {
      Serial.println("ERROR:Unknown command");
    }
  }
}
