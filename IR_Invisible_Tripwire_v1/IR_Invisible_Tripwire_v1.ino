#define IR_TX_PIN 18
#define IR_RX_PIN 27

#define ACTIVE_BUZZER 26
#define PASSIVE_BUZZER 25

#define LED_R 23
#define LED_G 17
#define LED_B 16

bool previousBeam = true;


// ------------------------
// RGB LED
// ------------------------

void setRGB(bool red, bool green, bool blue) {

  digitalWrite(LED_R, red);
  digitalWrite(LED_G, green);
  digitalWrite(LED_B, blue);
}


// ------------------------
// 경고음
// ------------------------

void alarmSound() {

  // Active buzzer ON
  digitalWrite(ACTIVE_BUZZER, HIGH);

  // Passive buzzer 첫 번째 경고음
  tone(PASSIVE_BUZZER, 1000);
  delay(150);

  // 높은 음
  tone(PASSIVE_BUZZER, 1800);
  delay(150);

  // 다시 낮은 음
  tone(PASSIVE_BUZZER, 1000);
  delay(150);

  // 높은 음
  tone(PASSIVE_BUZZER, 1800);
  delay(150);

  noTone(PASSIVE_BUZZER);

  digitalWrite(ACTIVE_BUZZER, LOW);
}


// ------------------------
// IR Beam 검사
// ------------------------

bool checkIRBeam() {

  bool beamDetected = false;

  // 38kHz IR 송신
  tone(IR_TX_PIN, 38000);

  unsigned long startTime = micros();

  // 3ms 동안 Receiver 확인
  while (micros() - startTime < 3000) {

    if (digitalRead(IR_RX_PIN) == LOW) {

      beamDetected = true;

    }

  }

  noTone(IR_TX_PIN);

  return beamDetected;
}


// ------------------------
// SETUP
// ------------------------

void setup() {

  Serial.begin(115200);

  pinMode(IR_TX_PIN, OUTPUT);
  pinMode(IR_RX_PIN, INPUT);

  pinMode(ACTIVE_BUZZER, OUTPUT);
  pinMode(PASSIVE_BUZZER, OUTPUT);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  digitalWrite(ACTIVE_BUZZER, LOW);

  // 시작 상태 = GREEN
  setRGB(false, true, false);

  Serial.println();
  Serial.println("=======================");
  Serial.println(" IR INVISIBLE TRIPWIRE");
  Serial.println("=======================");
  Serial.println("SYSTEM READY");
}


// ------------------------
// LOOP
// ------------------------

void loop() {

  bool beamDetected = checkIRBeam();


  // ------------------------
  // IR이 방금 끊긴 순간
  // ------------------------

  if (previousBeam == true &&
      beamDetected == false) {

    Serial.println();
    Serial.println("!!! TRIPWIRE HIT !!!");

    // RED
    setRGB(true, false, false);

    alarmSound();
  }


  // ------------------------
  // IR이 다시 연결됨
  // ------------------------

  if (previousBeam == false &&
      beamDetected == true) {

    Serial.println("BEAM RESTORED");

    // GREEN
    setRGB(false, true, false);
  }


  previousBeam = beamDetected;

  delay(50);
}