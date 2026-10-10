// ============================================================
// ESP32 + RGBW 5선 Strip + WS2815 16x16 Panel + HC-SR04
// Strip: 밝기 변화로 Flow 착시 효과
// Panel: Strip Flow 종료 후 잠깐 대기 → 중심 파장
// ESP32 Arduino Core 3.x
// ============================================================

#include <Arduino.h>
#include <FastLED.h>
#include <math.h>

// ============================================================
// WS2815 PANEL
// ============================================================

#define PANEL_PIN       18
#define NUM_PANEL       256
#define MATRIX_SIZE     16
#define CHIPSET         WS2815
#define COLOR_ORDER     GRB

CRGB panelLeds[NUM_PANEL];

// ============================================================
// RGBW STRIP
// ============================================================

#define PIN_R 23
#define PIN_G 17
#define PIN_B 16
#define PIN_W 19

#define PWM_FREQ        5000
#define PWM_RESOLUTION  8

// true  : PWM 255 = OFF, 0 = ON
// false : PWM 0 = OFF, 255 = ON
const bool STRIP_INVERTED = true;

// ============================================================
// HC-SR04
// ============================================================

#define TRIG_PIN 21
#define ECHO_PIN 22

float distanceCm = -1;

// ============================================================
// EFFECT SETTINGS
// ============================================================

const float TRIGGER_DISTANCE = 15.0f;
const float RESET_DISTANCE   = 20.0f;

// Strip Flow 전체 시간
const unsigned long STRIP_FLOW_DURATION = 1400;

// Strip 끝에 도달한 느낌으로 유지하는 시간
const unsigned long STRIP_HOLD_DURATION = 500;

// Panel 파장 시간
const unsigned long PANEL_DURATION = 2200;

// 종료 후 대기
const unsigned long FINISH_DELAY = 500;

// 업데이트 간격
const unsigned long STRIP_INTERVAL  = 16;
const unsigned long PANEL_INTERVAL  = 25;
const unsigned long SENSOR_INTERVAL = 60;

// ============================================================
// STATE MACHINE
// ============================================================

enum AppState {
  STATE_IDLE,
  STATE_STRIP_FLOW,
  STATE_STRIP_HOLD,
  STATE_PANEL_WAVE,
  STATE_FINISHED
};

AppState currentState = STATE_IDLE;

// ============================================================
// TIME
// ============================================================

unsigned long stripStartTime = 0;
unsigned long stripHoldStartTime = 0;
unsigned long panelStartTime = 0;
unsigned long finishStartTime = 0;

unsigned long lastStripUpdate = 0;
unsigned long lastPanelUpdate = 0;
unsigned long lastSensorRead = 0;

bool sequenceTriggered = false;

// ============================================================
// PANEL VARIABLE
// ============================================================

uint8_t waveOffset = 0;

// ============================================================
// PWM
// ============================================================

void writePWM(uint8_t pin, uint8_t brightness) {

  uint8_t duty;

  if (STRIP_INVERTED) {
    duty = 255 - brightness;
  } else {
    duty = brightness;
  }

  ledcWrite(pin, duty);
}

void setStrip(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {

  writePWM(PIN_R, r);
  writePWM(PIN_G, g);
  writePWM(PIN_B, b);
  writePWM(PIN_W, w);
}

void stripOff() {
  setStrip(0, 0, 0, 0);
}

// ============================================================
// PANEL
// ============================================================

void panelOff() {

  fill_solid(panelLeds, NUM_PANEL, CRGB::Black);
  FastLED.show();
}

// ============================================================
// MATRIX XY
// ============================================================

uint16_t XY(uint8_t x, uint8_t y) {

  if (y % 2 == 0) {
    return y * MATRIX_SIZE + x;
  } else {
    return y * MATRIX_SIZE + (MATRIX_SIZE - 1 - x);
  }
}

// ============================================================
// SENSOR
// ============================================================

float readDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000);

  if (duration == 0) {
    return -1;
  }

  float cm = duration * 0.0343f / 2.0f;

  if (cm < 2 || cm > 400) {
    return -1;
  }

  return cm;
}

// ============================================================
// SEQUENCE START
// ============================================================

void startSequence() {

  stripOff();
  panelOff();

  currentState = STATE_STRIP_FLOW;

  stripStartTime = millis();
  lastStripUpdate = 0;

  sequenceTriggered = true;

  Serial.println();
  Serial.println("=== SEQUENCE START ===");
  Serial.println("Strip Flow START");
}

// ============================================================
// STRIP FLOW ILLUSION
// ============================================================
//
// 5선 RGBW Strip 전체 밝기를 이용한 착시.
//
// 밝기를:
// 약 → 강 → 약 → 강 → 약
//
// 식으로 여러 번 움직이게 하고,
// 시간이 지날수록 peak 밝기를 높여
// 빛이 앞으로 달려가는 느낌을 만듭니다.
//
// 마지막에는 매우 밝은 White 쪽으로 수렴합니다.
// ============================================================

void updateStripFlow(unsigned long now) {

  if (currentState != STATE_STRIP_FLOW) {
    return;
  }

  if (now - lastStripUpdate < STRIP_INTERVAL) {
    return;
  }

  lastStripUpdate = now;

  unsigned long elapsed = now - stripStartTime;

  if (elapsed >= STRIP_FLOW_DURATION) {

    // Flow가 끝에 도착했을 때
    // 밝은 상태로 잠깐 유지
    setStrip(180, 220, 255, 255);

    currentState = STATE_STRIP_HOLD;
    stripHoldStartTime = now;

    Serial.println("Strip reached END");
    Serial.println("Holding...");
    return;
  }

  float progress =
    (float)elapsed /
    (float)STRIP_FLOW_DURATION;

  // ----------------------------------------------------------
  // Flow pulse
  //
  // progress가 진행되면서 여러 개의 광량 pulse 발생
  // ----------------------------------------------------------

  const float pulseCount = 4.0f;

  float pulse =
    sinf(
      progress *
      PI *
      pulseCount
    );

  // 음수 제거
  if (pulse < 0) {
    pulse = 0;
  }

  // sharp peak
  pulse =
    pulse * pulse;

  // ----------------------------------------------------------
  // 뒤로 갈수록 전체 밝기 증가
  // ----------------------------------------------------------

  float baseBrightness =
    0.18f +
    progress * 0.50f;

  float pulseBrightness =
    pulse * 0.55f;

  float brightness =
    baseBrightness +
    pulseBrightness;

  if (brightness > 1.0f) {
    brightness = 1.0f;
  }

  // ----------------------------------------------------------
  // 끝으로 갈수록 White 증가
  // ----------------------------------------------------------

  float whiteMix = progress * progress;

  uint8_t masterBrightness =
    (uint8_t)(brightness * 255.0f);

  // ----------------------------------------------------------
  // 색상 이동
  //
  // Cyan → Blue → White
  // ----------------------------------------------------------

  uint8_t blue =
    masterBrightness;

  uint8_t green =
    (uint8_t)(
      masterBrightness *
      (0.55f + progress * 0.35f)
    );

  uint8_t red =
    (uint8_t)(
      masterBrightness *
      (0.05f + whiteMix * 0.75f)
    );

  uint8_t white =
    (uint8_t)(
      masterBrightness *
      whiteMix
    );

  // ----------------------------------------------------------
  // 빠른 shimmer 추가
  //
  // 흐르는 느낌을 조금 더 강화
  // ----------------------------------------------------------

  float shimmer =
    sinf(
      elapsed *
      0.045f
    );

  shimmer =
    (shimmer + 1.0f) * 0.5f;

  float shimmerAmount =
    0.82f +
    shimmer * 0.18f;

  red   *= shimmerAmount;
  green *= shimmerAmount;
  blue  *= shimmerAmount;
  white *= shimmerAmount;

  setStrip(
    red,
    green,
    blue,
    white
  );
}

// ============================================================
// STRIP END HOLD
// ============================================================

void updateStripHold(unsigned long now) {

  if (currentState != STATE_STRIP_HOLD) {
    return;
  }

  unsigned long elapsed =
    now - stripHoldStartTime;

  // 처음에는 밝게 유지
  if (elapsed < STRIP_HOLD_DURATION * 0.65f) {

    setStrip(
      160,
      210,
      255,
      255
    );
  }

  // 마지막 35%에서 Fade Out
  else {

    float fadeProgress =
      (
        elapsed -
        STRIP_HOLD_DURATION * 0.65f
      )
      /
      (
        STRIP_HOLD_DURATION * 0.35f
      );

    if (fadeProgress > 1.0f) {
      fadeProgress = 1.0f;
    }

    float fade =
      1.0f - fadeProgress;

    setStrip(
      160 * fade,
      210 * fade,
      255 * fade,
      255 * fade
    );
  }

  // ----------------------------------------------------------
  // Hold 종료 → Panel Wave
  // ----------------------------------------------------------

  if (elapsed >= STRIP_HOLD_DURATION) {

    stripOff();

    currentState = STATE_PANEL_WAVE;
    panelStartTime = now;

    waveOffset = 0;

    Serial.println("Strip HOLD END");
    Serial.println("Panel Wave START");
  }
}

// ============================================================
// PANEL WAVE
// ============================================================

void updatePanel(unsigned long now) {

  if (currentState != STATE_PANEL_WAVE) {
    return;
  }

  if (now - lastPanelUpdate < PANEL_INTERVAL) {
    return;
  }

  lastPanelUpdate = now;

  unsigned long elapsed =
    now - panelStartTime;

  if (elapsed >= PANEL_DURATION) {

    panelOff();

    currentState = STATE_FINISHED;
    finishStartTime = now;

    Serial.println("Panel Wave END");
    return;
  }

  fill_solid(
    panelLeds,
    NUM_PANEL,
    CRGB::Black
  );

  float progress =
    (float)elapsed /
    (float)PANEL_DURATION;

  // ----------------------------------------------------------
  // 여러 개의 Ripple이 연속으로 퍼지도록 구성
  // ----------------------------------------------------------

  const float maxRadius = 12.0f;

  float primaryRadius =
    progress * maxRadius;

  waveOffset += 4;

  for (int y = 0; y < MATRIX_SIZE; y++) {

    for (int x = 0; x < MATRIX_SIZE; x++) {

      int idx = XY(x, y);

      float dx = x - 7.5f;
      float dy = y - 7.5f;

      float dist =
        sqrtf(
          dx * dx +
          dy * dy
        );

      CRGB finalColor = CRGB::Black;

      // ======================================================
      // Main Wave
      // ======================================================

      float delta =
        fabsf(
          dist -
          primaryRadius
        );

      const float waveWidth = 1.8f;

      if (delta < waveWidth) {

        float strength =
          1.0f -
          delta / waveWidth;

        strength *= strength;

        uint8_t bright =
          (uint8_t)(
            255.0f *
            strength
          );

        uint8_t hue =
          waveOffset +
          (uint8_t)(dist * 15.0f);

        finalColor +=
          CHSV(
            hue,
            255,
            bright
          );
      }

      // ======================================================
      // Secondary Wave
      // ======================================================

      float secondaryRadius =
        primaryRadius - 3.0f;

      if (secondaryRadius > 0) {

        float delta2 =
          fabsf(
            dist -
            secondaryRadius
          );

        if (delta2 < 1.2f) {

          float strength2 =
            1.0f -
            delta2 / 1.2f;

          uint8_t bright2 =
            (uint8_t)(
              110 *
              strength2
            );

          finalColor +=
            CHSV(
              waveOffset + 70,
              255,
              bright2
            );
        }
      }

      panelLeds[idx] =
        finalColor;
    }
  }

  // ----------------------------------------------------------
  // Center flash
  // ----------------------------------------------------------

  if (progress < 0.20f) {

    float flash =
      1.0f -
      progress / 0.20f;

    uint8_t centerBrightness =
      flash * 220;

    for (int y = 6; y <= 9; y++) {
      for (int x = 6; x <= 9; x++) {

        panelLeds[XY(x, y)] +=
          CRGB(
            centerBrightness,
            centerBrightness,
            centerBrightness
          );
      }
    }
  }

  FastLED.show();
}

// ============================================================
// FINISHED
// ============================================================

void updateFinished(unsigned long now) {

  if (currentState != STATE_FINISHED) {
    return;
  }

  if (
    now - finishStartTime
    >= FINISH_DELAY
  ) {

    stripOff();
    panelOff();

    currentState = STATE_IDLE;

    Serial.println("IDLE");
  }
}

// ============================================================
// SENSOR UPDATE
// ============================================================

void updateSensor(unsigned long now) {

  if (
    now - lastSensorRead
    < SENSOR_INTERVAL
  ) {
    return;
  }

  lastSensorRead = now;

  float newDistance =
    readDistance();

  if (newDistance > 0) {
    distanceCm = newDistance;
  }

  // 손 감지
  if (
    distanceCm > 0 &&
    distanceCm < TRIGGER_DISTANCE
  ) {

    if (
      !sequenceTriggered &&
      currentState == STATE_IDLE
    ) {

      startSequence();
    }
  }

  // 손을 빼면 다음 트리거 허용
  if (
    distanceCm >
    RESET_DISTANCE
  ) {

    sequenceTriggered = false;
  }
}

// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(300);

  // Sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  // Panel
  FastLED.addLeds<
    CHIPSET,
    PANEL_PIN,
    COLOR_ORDER
  >(
    panelLeds,
    NUM_PANEL
  );

  FastLED.setBrightness(80);

  panelOff();

  // ESP32 Core 3.x PWM
  ledcAttach(
    PIN_R,
    PWM_FREQ,
    PWM_RESOLUTION
  );

  ledcAttach(
    PIN_G,
    PWM_FREQ,
    PWM_RESOLUTION
  );

  ledcAttach(
    PIN_B,
    PWM_FREQ,
    PWM_RESOLUTION
  );

  ledcAttach(
    PIN_W,
    PWM_FREQ,
    PWM_RESOLUTION
  );

  stripOff();

  currentState = STATE_IDLE;

  Serial.println();
  Serial.println("=======================");
  Serial.println("LED Fighter READY");
  Serial.println("=======================");
  Serial.println("Hand < 15cm = START");
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  unsigned long now = millis();

  updateSensor(now);

  updateStripFlow(now);

  updateStripHold(now);

  updatePanel(now);

  updateFinished(now);
}