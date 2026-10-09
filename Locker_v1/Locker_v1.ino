#include <ESP32Servo.h>


// ===============================
// PIN 설정
// ===============================

// Joystick
#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 25


// Confirm Button
#define CHECK_BUTTON 26


// Door Switch
#define DOOR_SWITCH 27


// Buzzer
#define BUZZER 12


// KY-016 RGB
#define LED_G 5
#define LED_B 18


// MG996R Servo
#define SERVO_PIN 13


Servo lockServo;


// ===============================
// Password
// ===============================

int correctPassword[4] = {2,5,8,3};

int inputPassword[4];

int inputIndex = 0;

int currentNumber = 0;



// ===============================
// Setup
// ===============================

void setup()
{

  Serial.begin(115200);


  pinMode(JOY_SW, INPUT_PULLUP);

  pinMode(CHECK_BUTTON, INPUT_PULLUP);

  pinMode(DOOR_SWITCH, INPUT_PULLUP);


  pinMode(BUZZER, OUTPUT);


  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);


  digitalWrite(BUZZER,LOW);


  // Servo 초기화
  lockServo.attach(SERVO_PIN);

  lockDoor();


  blueLED();


  Serial.println("======================");
  Serial.println(" SMART SAFE SYSTEM ");
  Serial.println(" MG996R VERSION ");
  Serial.println("======================");

  Serial.println("SYSTEM READY");

}



// ===============================
// Loop
// ===============================

void loop()
{


  joystickRead();



  // Joystick 버튼 = 숫자 저장

  if(digitalRead(JOY_SW)==LOW)
  {

    delay(200);

    saveNumber();

  }



  // KY-004 확인

  if(digitalRead(CHECK_BUTTON)==LOW)
  {

    delay(200);

    checkPassword();

  }



  doorCheck();


}



// ===============================
// Joystick 입력
// ===============================

void joystickRead()
{

  int y = analogRead(JOY_Y);



  if(y > 3000)
  {

    currentNumber++;

    if(currentNumber > 9)
      currentNumber=0;


    showNumber();


    delay(300);

  }



  if(y < 1000)
  {

    currentNumber--;

    if(currentNumber < 0)
      currentNumber=9;


    showNumber();


    delay(300);

  }

}



// ===============================
// 숫자 표시
// ===============================

void showNumber()
{

  blueLED();


  Serial.print("CURRENT NUMBER : ");

  Serial.println(currentNumber);

}



// ===============================
// 숫자 저장
// ===============================

void saveNumber()
{

  if(inputIndex < 4)
  {

    inputPassword[inputIndex]=currentNumber;


    Serial.print("INPUT ");

    Serial.print(inputIndex+1);

    Serial.print(" : ");

    Serial.println(currentNumber);



    inputIndex++;

    currentNumber=0;

  }

}



// ===============================
// Password 확인
// ===============================

void checkPassword()
{

  Serial.println();

  Serial.println("CHECKING PASSWORD...");


  bool correct=true;


  for(int i=0;i<4;i++)
  {

    if(inputPassword[i]!=correctPassword[i])
    {
      correct=false;
    }

  }



  if(correct)
  {

    Serial.println("PASSWORD OK");


    greenLED();


    openDoor();


  }

  else
  {

    Serial.println("WRONG PASSWORD");


    buzzerAlarm();


  }



  delay(1000);


  resetPassword();


  lockDoor();


  blueLED();


}



// ===============================
// Servo 제어
// ===============================

void openDoor()
{

  Serial.println("SERVO OPEN");


  lockServo.write(90);


  delay(3000);


}



void lockDoor()
{

  Serial.println("SERVO LOCK");


  lockServo.write(10);


}



// ===============================
// Buzzer KY-012
// ===============================

void buzzerAlarm()
{

  Serial.println("BUZZER ON");


  for(int i=0;i<3;i++)
  {

    digitalWrite(BUZZER,HIGH);

    delay(300);


    digitalWrite(BUZZER,LOW);

    delay(300);

  }


}



// ===============================
// KY-021 Door Switch
// ===============================

void doorCheck()
{

  if(digitalRead(DOOR_SWITCH)==LOW)
  {

    Serial.println("DOOR CLOSED");

    delay(500);

  }

}



// ===============================
// Password Reset
// ===============================

void resetPassword()
{

  inputIndex=0;


  for(int i=0;i<4;i++)
  {
    inputPassword[i]=0;
  }

}



// ===============================
// KY-016 LED
// Green + Blue Only
// ===============================

void blueLED()
{

  digitalWrite(LED_G,LOW);

  digitalWrite(LED_B,HIGH);

}



void greenLED()
{

  digitalWrite(LED_G,HIGH);

  digitalWrite(LED_B,LOW);

}