#include <DHT.h>
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD
#include <RTClib.h>

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
RTC_DS1307 RTC;     // Setup an instance of DS1307 naming it RTC


#define DHTPIN1 D5       // what digital pin the DHT11 is conected to
#define DHTTYPE1 DHT11   // there are multiple kinds of DHT sensors
#define DHTPIN2 D6       // what digital pin the DHT11 is conected to
#define DHTTYPE2 DHT11   // there are multiple kinds of DHT sensors

#define UVB D8           // define UVB lamp pin
#define HEAT D7          // define HEAT lamp pin


DHT dht1(DHTPIN1, DHTTYPE1);
DHT dht2(DHTPIN2, DHTTYPE2);

float Temperature1, Humidity1, Temperature2, Humidity2;

void setup() {

  Wire.begin(D2, D1); // Start the I2C D1 SCL and D2 SDA
  lcd.begin();        // Init LCD
  RTC.begin();        // Init RTC
  delay(100);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("STARTING SYSTEM");
  delay(2000);
  lcd.clear();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(UVB, OUTPUT);
  pinMode(HEAT, OUTPUT);
  pinMode(DHTPIN1, INPUT);
  pinMode(DHTPIN2, INPUT);
  digitalWrite(UVB, LOW);
  digitalWrite(HEAT, LOW);
  dht1.begin();       // Init dht1
  dht2.begin();       // Init dht2

}

//int timeSinceLastRead = 0;
int size = 10;
CircularBuffer<float,size> Humidity1; // buffer capacity is 5
CircularBuffer<float,size> Humidity2; // buffer capacity is 5
CircularBuffer<float,size> Temperature1; // buffer capacity is 5
CircularBuffer<float,size> Temperature2; // buffer capacity is 5
float currentTemp = 0.0;
float currentHumi = 0.0;

void lcdPrinter(Temperature1, Temperature2, Humidity1, Humidity2){
  //    // Report every 2 seconds.
  //  if(timeSinceLastRead > 2000)
  //  {
    digitalWrite(LED_BUILTIN, LOW);


  //////// LCD PRINT ////////////////////////
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T1:");
    lcd.setCursor(3, 0);
    lcd.print(Temperature1);
    lcd.setCursor(0, 1);
    lcd.print("T2:");
    lcd.setCursor(3, 1);
    lcd.print(Temperature2);

    lcd.setCursor(7, 0);
    lcd.print("/H1:");
    lcd.setCursor(11, 0);
    lcd.print(Humidity1);
    lcd.setCursor(7, 1);
    lcd.print("/H2:");
    lcd.setCursor(11, 1);
    lcd.print(Humidity2);

    delay(3000);
    lcd.clear();

    digitalWrite(LED_BUILTIN, HIGH);

    lcd.setCursor(0, 0);
    lcd.print("Date: ");
    lcd.setCursor(0, 1);
    lcd.print("Time: ");

    lcd.setCursor(6, 0);
    lcd.print(now.year(), DEC);
    lcd.print(":");
    lcd.print(now.month(), DEC);
    lcd.print(":");
    lcd.print(now.day(), DEC);

    lcd.setCursor(6, 1);
    lcd.print(now.hour(), DEC);
    lcd.print(":");
    lcd.print(now.minute(), DEC);
    lcd.print(":");
    lcd.print(now.second(), DEC);
    delay(3000);
    lcd.clear();
  //  delay(500);
  //
  //  timeSinceLastRead = 0;
  //  }
  //  delay(100);
  //  timeSinceLastRead += 100;
}


float filter(CircularBuffer<float,size> buffer) {
  float alpha = 0.5;
  float under = 1/alpha;
  float accumulator = 0.0;

  for(int i = 0; i<size-1; i++){
    accumulator += pow(1-alpha, i) * buffer[i];
  }
  return accumulator / under;
}


float PredictValueFromArrays(CircularBuffer<float,size> *buffer1, CircularBuffer<float,size> *bufer2){
  float tempValue1 = buffer1.pop();
  float tempValue2 = buffer1.pop();
  float predicted1 = filter(buffer1);
  float predicted2 = filter(buffer2);
  float predictedAverage = (predicted1 + predicted2) / 2;
  if(abs(predicted1 - tempValue1)<.05*predicted1)
    buffer1.push(tempValue1)
  else
    buffer1.push(predicted1)
  if(abs(predicted2 - tempValue2)<.1*predicted2)
    buffer2.push(tempValue2)
  else
    buffer2.push(predicted2)
  return(predictedAverage);
}


void loop(){
  DateTime now = RTC.now();
  Humidity1.push(dht1.readHumidity());
  Humidity2.push(dht2.readHumidity());
  Temperature1.push(dht1.readTemperature());
  Temperature2.push(dht2.readTemperature());

  currentTemp = PredictValueFromArrays(Temperature1, Temperature2)
  currentHumi = PredictValueFromArrays(Humidity1, Humidity2)

  if (now.hour() >= 9 && now.hour() < 19)
    digitalWrite(UVB, HIGH);
  else
    digitalWrite(UVB, LOW);

  if (currentTemp >= 30.0)
    digitalWrite(HEAT, LOW);
  if (currentTemp <= 29.0)
    digitalWrite(HEAT, HIGH);

  lcdPrinter(Temperature1, Temperature2, Humidity1, Humidity2)
}
