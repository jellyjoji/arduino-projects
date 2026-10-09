#include <FastLED.h>

// ======================================================
// 16x16 WS2815 LED PANEL
// ======================================================

#define LED_DATA_PIN 18

#define MATRIX_WIDTH   16
#define MATRIX_HEIGHT  16
#define NUM_LEDS       256

#define BRIGHTNESS 40

CRGB leds[NUM_LEDS];


// ======================================================
// KY-023 JOYSTICK
// ======================================================

#define JOY_X   34
#define JOY_Y   35
#define JOY_SW  32

int centerX = 2048;
int centerY = 2048;

// 중앙에서 이 값 이상 움직여야 방향으로 인식
const int DEAD_ZONE = 650;


// 조이스틱 방향이 반대로 나오면 변경
bool INVERT_X = false;
bool INVERT_Y = false;


// ======================================================
// KY-027 MAGIC LIGHT CUP
// ======================================================

#define KY027_SIGNAL 27

// 센서가 반대로 동작하면 LOW로 변경
#define KY027_ACTIVE_LEVEL HIGH


// ======================================================
// LED MATRIX 방향 설정
// ======================================================

// 대부분의 flexible matrix는 지그재그 방식
bool SERPENTINE = true;

// 좌우가 반대로 나오면 true
bool FLIP_X = false;

// 상하가 반대로 나오면 true
bool FLIP_Y = false;

// 첫 줄 LED 진행 방향이 반대일 경우 true
bool FIRST_ROW_REVERSED = false;


// ======================================================
// 방향 정의
// ======================================================

enum Direction
{
  DIR_CENTER,
  DIR_UP,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT
};


// ======================================================
// XY 좌표 → 실제 LED 번호
// ======================================================

uint16_t XY(int x, int y)
{
  if (FLIP_X)
  {
    x = MATRIX_WIDTH - 1 - x;
  }

  if (FLIP_Y)
  {
    y = MATRIX_HEIGHT - 1 - y;
  }


  bool reversed = false;

  if (SERPENTINE)
  {
    reversed = (y % 2 == 1);

    if (FIRST_ROW_REVERSED)
    {
      reversed = !reversed;
    }
  }


  if (reversed)
  {
    return
      y * MATRIX_WIDTH +
      (MATRIX_WIDTH - 1 - x);
  }

  return y * MATRIX_WIDTH + x;
}


// ======================================================
// 픽셀 하나 켜기
// ======================================================

void setPixel(int x, int y, CRGB color)
{
  if (x < 0 || x >= MATRIX_WIDTH)
    return;

  if (y < 0 || y >= MATRIX_HEIGHT)
    return;

  leds[XY(x, y)] = color;
}


// ======================================================
// UP 화살표 좌표를 다른 방향으로 회전
// ======================================================

void arrowPixel(
  int x,
  int y,
  Direction direction,
  CRGB color
)
{
  int tx = x;
  int ty = y;


  switch (direction)
  {
    case DIR_UP:

      tx = x;
      ty = y;

      break;


    case DIR_DOWN:

      tx = 15 - x;
      ty = 15 - y;

      break;


    case DIR_LEFT:

      tx = y;
      ty = 15 - x;

      break;


    case DIR_RIGHT:

      tx = 15 - y;
      ty = x;

      break;


    default:

      return;
  }


  setPixel(tx, ty, color);
}


// ======================================================
// 화살표 그리기
// ======================================================

void drawArrow(
  Direction direction,
  CRGB color
)
{
  FastLED.clear();


  if (direction == DIR_CENTER)
  {
    return;
  }


  // ----------------------------------
  // 화살표 머리
  //
  //        ██
  //       ████
  //      ██████
  //     ████████
  //    ██████████
  //
  // ----------------------------------

  for (int row = 0; row < 5; row++)
  {
    int y = 1 + row;

    int startX = 7 - row;
    int endX   = 8 + row;


    for (int x = startX; x <= endX; x++)
    {
      arrowPixel(
        x,
        y,
        direction,
        color
      );
    }
  }


  // ----------------------------------
  // 화살표 몸통
  // ----------------------------------

  for (int y = 5; y <= 13; y++)
  {
    arrowPixel(6, y, direction, color);
    arrowPixel(7, y, direction, color);
    arrowPixel(8, y, direction, color);
    arrowPixel(9, y, direction, color);
  }
}


// ======================================================
// 방향에 따른 색
// ======================================================

CRGB getDirectionColor(Direction direction)
{
  switch (direction)
  {
    case DIR_UP:

      return CRGB::Blue;


    case DIR_DOWN:

      return CRGB::Red;


    case DIR_LEFT:

      return CRGB::Green;


    case DIR_RIGHT:

      return CRGB::Yellow;


    default:

      return CRGB::Black;
  }
}


// ======================================================
// 조이스틱 중앙 자동 보정
// ======================================================

void calibrateJoystick()
{
  long totalX = 0;
  long totalY = 0;

  const int SAMPLE_COUNT = 100;


  Serial.println();
  Serial.println("Joystick Calibration");
  Serial.println("조이스틱을 움직이지 마세요.");


  for (int i = 0; i < SAMPLE_COUNT; i++)
  {
    totalX += analogRead(JOY_X);
    totalY += analogRead(JOY_Y);

    delay(5);
  }


  centerX = totalX / SAMPLE_COUNT;
  centerY = totalY / SAMPLE_COUNT;


  Serial.print("Center X = ");
  Serial.println(centerX);

  Serial.print("Center Y = ");
  Serial.println(centerY);

  Serial.println("Calibration Finished");
}


// ======================================================
// 조이스틱 방향 판단
// ======================================================

Direction readJoystickDirection(
  int xValue,
  int yValue
)
{
  int dx = xValue - centerX;
  int dy = yValue - centerY;


  if (INVERT_X)
  {
    dx = -dx;
  }

  if (INVERT_Y)
  {
    dy = -dy;
  }


  // 중앙
  if (
      abs(dx) < DEAD_ZONE &&
      abs(dy) < DEAD_ZONE
     )
  {
    return DIR_CENTER;
  }


  // X축 움직임이 더 큰 경우
  if (abs(dx) > abs(dy))
  {
    if (dx > 0)
    {
      return DIR_RIGHT;
    }

    return DIR_LEFT;
  }


  // Y축 움직임이 더 큰 경우

  if (dy > 0)
  {
    return DIR_UP;
  }

  return DIR_DOWN;
}


// ======================================================
// 방향 이름
// ======================================================

const char* directionName(Direction direction)
{
  switch (direction)
  {
    case DIR_UP:

      return "UP";


    case DIR_DOWN:

      return "DOWN";


    case DIR_LEFT:

      return "LEFT";


    case DIR_RIGHT:

      return "RIGHT";


    default:

      return "CENTER";
  }
}


// ======================================================
// SETUP
// ======================================================

void setup()
{
  Serial.begin(115200);

  delay(500);


  // --------------------------------------------------
  // KY-023
  // --------------------------------------------------

  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);

  pinMode(
    JOY_SW,
    INPUT_PULLUP
  );


  // ESP32 ADC = 12bit
  analogReadResolution(12);


  // --------------------------------------------------
  // KY-027
  // --------------------------------------------------

  pinMode(
    KY027_SIGNAL,
    INPUT
  );


  // --------------------------------------------------
  // WS2815
  //
  // DATA = GPIO18
  // --------------------------------------------------

  FastLED.addLeds<
    WS2815,
    LED_DATA_PIN,
    GRB
  >(leds, NUM_LEDS);


  FastLED.setBrightness(
    BRIGHTNESS
  );


  FastLED.clear();
  FastLED.show();


  // --------------------------------------------------
  // Joystick 중앙 자동 보정
  // --------------------------------------------------

  calibrateJoystick();


  Serial.println();
  Serial.println(
    "Joystick LED Direction System START"
  );
}


// ======================================================
// LOOP
// ======================================================

void loop()
{
  // --------------------------------------------------
  // Joystick 읽기
  // --------------------------------------------------

  int xValue =
    analogRead(JOY_X);

  int yValue =
    analogRead(JOY_Y);


  Direction direction =
    readJoystickDirection(
      xValue,
      yValue
    );


  // --------------------------------------------------
  // KY-027
  // --------------------------------------------------

  int cupState =
    digitalRead(KY027_SIGNAL);


  bool cupTriggered =
    (cupState == KY027_ACTIVE_LEVEL);


  // --------------------------------------------------
  // 기본 화살표 색상
  // --------------------------------------------------

  CRGB color =
    getDirectionColor(direction);


  // --------------------------------------------------
  // KY-027 움직임 감지
  //
  // 현재 화살표를 흰색으로 표시
  // --------------------------------------------------

  if (
      cupTriggered &&
      direction != DIR_CENTER
     )
  {
    color = CRGB::White;
  }


  // --------------------------------------------------
  // LED Panel 표시
  // --------------------------------------------------

  drawArrow(
    direction,
    color
  );


  FastLED.show();


  // --------------------------------------------------
  // Serial Monitor 출력
  // --------------------------------------------------

  Serial.print("X=");
  Serial.print(xValue);

  Serial.print(" Y=");
  Serial.print(yValue);

  Serial.print(" Direction=");
  Serial.print(
    directionName(direction)
  );


  Serial.print(" KY027=");
  Serial.print(cupState);


  Serial.print(" Button=");


  if (digitalRead(JOY_SW) == LOW)
  {
    Serial.println("PRESSED");
  }
  else
  {
    Serial.println("RELEASED");
  }


  delay(50);
}