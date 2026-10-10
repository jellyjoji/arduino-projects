#include <DHT.h>


// ======================
// Pin 설정
// ======================

#define DHTPIN 4
#define DHTTYPE DHT11

#define RELAY_PIN 26


DHT dht(DHTPIN, DHTTYPE);


// ======================
// 기준값
// ======================

#define TEMP_LIMIT 27.5
#define HUM_LIMIT 70.0



void setup()
{
  Serial.begin(115200);

  dht.begin();


  pinMode(RELAY_PIN, OUTPUT);


  // Relay OFF
  digitalWrite(RELAY_PIN, LOW);


  Serial.println("====================");
  Serial.println("SMART GREEN HOUSE");
  Serial.println("TEMP + HUM CONTROL");
  Serial.println("====================");

}



void loop()
{

  float temperature = dht.readTemperature();

  float humidity = dht.readHumidity();



  if(isnan(temperature) || isnan(humidity))
  {
    Serial.println("DHT SENSOR ERROR");

    delay(2000);

    return;
  }



  // ======================
  // 값 출력
  // ======================

  Serial.print("Temperature : ");
  Serial.print(temperature);
  Serial.println(" C");


  Serial.print("Humidity : ");
  Serial.print(humidity);
  Serial.println(" %");



  Serial.println("--------------------");



  // ======================
  // Relay 제어
  // ======================Multi Sensor Challenge Game

sdknsladnl
Multi Sensor Challenge
    Serial.println("NORMAL");

    Serial.println("RELAY OFF");


    digitalWrite(RELAY_PIN, LOW);

  }



  Serial.println("====================");


  delay(2000);

}