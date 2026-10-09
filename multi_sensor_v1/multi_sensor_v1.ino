// ======================================================
// EMOTION PIANO - HARDWARE TEST
// ESP32
//
// Touch C : GPIO4
// Touch D : GPIO27
// Touch E : GPIO32
// Touch G : GPIO13
//
// RGB R : GPIO14
// RGB G : GPIO25
// RGB B : GPIO18
//
// Passive Buzzer : GPIO26
// Button         : GPIO23
// ======================================================


// ------------------------------------------------------
// Capacitive Touch Pins
// ------------------------------------------------------

#define TOUCH_C 4
#define TOUCH_D 27
#define TOUCH_E 32
#define TOUCH_G 13


// ------------------------------------------------------
// RGB LED
// ------------------------------------------------------

#define LED_R 14
#define LED_G 25
#define LED_B 18


// ------------------------------------------------------
// Passive Buzzer
// ------------------------------------------------------

#define BUZZER_PIN 26


// ------------------------------------------------------
// Button
// ------------------------------------------------------

#define BUTTON_PIN 23


// ======================================================
// Piano frequencies
// ======================================================

#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_G4 392


// ======================================================
// Touch calibration
// ======================================================

uint32_t baseC = 0;
uint32_t baseD = 0;
uint32_t baseE = 0;
uint32_t baseG = 0;


// 기본값의 70% 이하가 되면 TOUCH로 판단
float touchRatio = 0.70;


// 이전 터치 상태
bool lastC = false;
bool lastD = false;
bool lastE = false;
bool lastG = false;


// ======================================================
// SETUP
// ======================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);


  Serial.println();
  Serial.println("================================");
  Serial.println("   EMOTION PIANO HARDWARE TEST");
  Serial.println("================================");


  // RGB LED
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);


  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);


  // 처음 LED OFF
  setColor(0, 0, 0);


  // ====================================================
  // RGB + BUZZER STARTUP TEST
  // ====================================================

  Serial.println();
  Serial.println("RGB / BUZZER START TEST");


  // C = RED
  Serial.println("C");
  setColor(255, 0, 0);
  tone(BUZZER_PIN, NOTE_C4);
  delay(300);


  // D = GREEN
  Serial.println("D");
  setColor(0, 255, 0);
  tone(BUZZER_PIN, NOTE_D4);
  delay(300);


  // E = BLUE
  Serial.println("E");
  setColor(0, 0, 255);
  tone(BUZZER_PIN, NOTE_E4);
  delay(300);


  // G = YELLOW
  Serial.println("G");
  setColor(255, 255, 0);
  tone(BUZZER_PIN, NOTE_G4);
  delay(500);


  noTone(BUZZER_PIN);

  setColor(0, 0, 0);


  // ====================================================
  // TOUCH CALIBRATION
  // ====================================================

  Serial.println();
  Serial.println("================================");
  Serial.println("DO NOT TOUCH THE TOUCH WIRES!");
  Serial.println("CALIBRATING...");
  Serial.println("================================");


  delay(2000);


  calibrateTouch();


  Serial.println();
  Serial.println("CALIBRATION FINISHED");
  Serial.println();


  Serial.print("C baseline : ");
  Serial.println(baseC);

  Serial.print("D baseline : ");
  Serial.println(baseD);

  Serial.print("E baseline : ");
  Serial.println(baseE);

  Serial.print("G baseline : ");
  Serial.println(baseG);


  Serial.println();
  Serial.println("================================");
  Serial.println("READY");
  Serial.println("================================");

  Serial.println("GPIO4  = C");
  Serial.println("GPIO27 = D");
  Serial.println("GPIO32 = E");
  Serial.println("GPIO13 = G");

  Serial.println();
  Serial.println("GPIO23 BUTTON = TEST MELODY");
  Serial.println();


  // 대기 상태 = 약한 파랑
  setColor(0, 0, 30);
}


// ======================================================
// LOOP
// ======================================================

void loop()
{
  // ----------------------------------------------------
  // Read touch sensors
  // ----------------------------------------------------

  uint32_t valueC = touchRead(TOUCH_C);
  uint32_t valueD = touchRead(TOUCH_D);
  uint32_t valueE = touchRead(TOUCH_E);
  uint32_t valueG = touchRead(TOUCH_G);


  // ----------------------------------------------------
  // Calculate thresholds
  // ----------------------------------------------------

  uint32_t thresholdC = baseC * touchRatio;
  uint32_t thresholdD = baseD * touchRatio;
  uint32_t thresholdE = baseE * touchRatio;
  uint32_t thresholdG = baseG * touchRatio;


  // ----------------------------------------------------
  // Detect touch
  // ----------------------------------------------------

  bool touchC = valueC < thresholdC;
  bool touchD = valueD < thresholdD;
  bool touchE = valueE < thresholdE;
  bool touchG = valueG < thresholdG;


  // ====================================================
  // BUTTON TEST
  // ====================================================

  if (digitalRead(BUTTON_PIN) == LOW)
  {
    Serial.println();
    Serial.println("BUTTON PRESSED");
    Serial.println("TEST MELODY");


    playTestMelody();


    // 버튼을 놓을 때까지 기다림
    while (digitalRead(BUTTON_PIN) == LOW)
    {
      delay(10);
    }
  }


  // ====================================================
  // TOUCH C
  // ====================================================

  if (touchC && !lastC)
  {
    Serial.println();
    Serial.println("TOUCH C");

    Serial.print("Touch Value : ");
    Serial.println(valueC);

    Serial.println("NOTE : C4 / 262 Hz");


    tone(BUZZER_PIN, NOTE_C4);


    // RED
    setColor(255, 0, 0);
  }


  // ====================================================
  // TOUCH D
  // ====================================================

  else if (touchD && !lastD)
  {
    Serial.println();
    Serial.println("TOUCH D");

    Serial.print("Touch Value : ");
    Serial.println(valueD);

    Serial.println("NOTE : D4 / 294 Hz");


    tone(BUZZER_PIN, NOTE_D4);


    // GREEN
    setColor(0, 255, 0);
  }


  // ====================================================
  // TOUCH E
  // ====================================================

  else if (touchE && !lastE)
  {
    Serial.println();
    Serial.println("TOUCH E");

    Serial.print("Touch Value : ");
    Serial.println(valueE);

    Serial.println("NOTE : E4 / 330 Hz");


    tone(BUZZER_PIN, NOTE_E4);


    // BLUE
    setColor(0, 0, 255);
  }


  // ====================================================
  // TOUCH G
  // GPIO13
  // ====================================================

  else if (touchG && !lastG)
  {
    Serial.println();
    Serial.println("TOUCH G");

    Serial.print("Touch Value : ");
    Serial.println(valueG);

    Serial.println("NOTE : G4 / 392 Hz");


    tone(BUZZER_PIN, NOTE_G4);


    // YELLOW
    setColor(255, 255, 0);
  }


  // ====================================================
  // NOTHING TOUCHED
  // ====================================================

  if (!touchC &&
      !touchD &&
      !touchE &&
      !touchG)
  {
    noTone(BUZZER_PIN);


    // IDLE = 약한 파랑
    setColor(0, 0, 30);
  }


  // ----------------------------------------------------
  // Save previous state
  // ----------------------------------------------------

  lastC = touchC;
  lastD = touchD;
  lastE = touchE;
  lastG = touchG;


  // ====================================================
  // Print Raw Touch Values
  // ====================================================

  static unsigned long lastPrint = 0;


  if (millis() - lastPrint > 500)
  {
    lastPrint = millis();


    Serial.print("C:");
    Serial.print(valueC);

    Serial.print("   D:");
    Serial.print(valueD);

    Serial.print("   E:");
    Serial.print(valueE);

    Serial.print("   G:");
    Serial.println(valueG);
  }


  delay(10);
}


// ======================================================
// TOUCH CALIBRATION
// ======================================================

void calibrateTouch()
{
  const int samples = 30;


  uint64_t sumC = 0;
  uint64_t sumD = 0;
  uint64_t sumE = 0;
  uint64_t sumG = 0;


  for (int i = 0; i < samples; i++)
  {
    sumC += touchRead(TOUCH_C);
    sumD += touchRead(TOUCH_D);
    sumE += touchRead(TOUCH_E);
    sumG += touchRead(TOUCH_G);


    delay(30);
  }


  baseC = sumC / samples;
  baseD = sumD / samples;
  baseE = sumE / samples;
  baseG = sumG / samples;
}


// ======================================================
// BUTTON TEST MELODY
// ======================================================

void playTestMelody()
{
  // C
  setColor(255, 0, 0);
  tone(BUZZER_PIN, NOTE_C4);
  delay(300);


  // D
  setColor(0, 255, 0);
  tone(BUZZER_PIN, NOTE_D4);
  delay(300);


  // E
  setColor(0, 0, 255);
  tone(BUZZER_PIN, NOTE_E4);
  delay(300);


  // G
  setColor(255, 255, 0);
  tone(BUZZER_PIN, NOTE_G4);
  delay(500);


  noTone(BUZZER_PIN);


  // IDLE
  setColor(0, 0, 30);
}


// ======================================================
// RGB LED
// ======================================================

void setColor(int r, int g, int b)
{
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}