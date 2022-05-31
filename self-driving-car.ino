#include <SPI.h>
#include <MFRC522.h>
#include <Arduino_FreeRTOS.h>

//RFID
#define SS_PIN 53
#define RST_PIN 5

// Motor
#define  motorPWM1 6 
#define  motorPWM2 2 
#define  in1 22
#define  in2 24
#define  in3 26
#define  in4 28

//Sensor
#define echoPin1 8
#define trigPin1 9
#define echoPin2 4
#define trigPin2 3

#define seatbelt 46
#define buzzPin 11

#define ldr A1
#define led 47

#define engineButton 40
 
long duration1; 
long duration2; 
int distance1;
int distance2;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
boolean isUnlocked;
boolean isRunning;

void task1(void*pvParameters);
void task2(void*pvParameters);
void task3(void*pvParameters);

void  setup ( ) 
{ 
  pinMode ( motorPWM1 ,  OUTPUT ) ;     
  pinMode ( motorPWM2 ,  OUTPUT ) ; 
  pinMode ( in1 ,  OUTPUT ) ; 
  pinMode ( in2 ,  OUTPUT ) ; 
  pinMode ( in3 ,  OUTPUT ) ; 
  pinMode ( in4 ,  OUTPUT ) ; 
  pinMode(trigPin1, OUTPUT); 
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);  
  pinMode(echoPin2, INPUT); 
  pinMode(buzzPin, OUTPUT);
  pinMode(seatbelt, INPUT);
  pinMode(engineButton, INPUT);
  pinMode(ldr, INPUT);
  pinMode(led, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
 
  isUnlocked = false;
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  isRunning = false;
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();

  xTaskCreate(task1, "startStop", 256, NULL, 3, NULL);
  xTaskCreate(task2, "movement", 256, NULL, 3, NULL);
  xTaskCreate(task3, "others", 256, NULL, 2, NULL);
} 

int readDistance1() {
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delay(10);
  digitalWrite(trigPin1, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration1 = pulseIn(echoPin1, HIGH);
  // Calculating the distance
  distance1 = duration1 * 0.034 / 2; 
  return distance1;
}

int readDistance2() {
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delay(10);
  digitalWrite(trigPin2, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration2 = pulseIn(echoPin2, HIGH);
  // Calculating the distance
  distance2 = duration2 * 0.034 / 2;
  return distance2;
}

void Stop(int del) {
  digitalWrite(in1, LOW) ;
  digitalWrite(in2, LOW) ;
  digitalWrite(in3, LOW) ;
  digitalWrite(in4, LOW) ;
  delay(del);
}

void left(int del) {
   digitalWrite(in1, HIGH) ;
   digitalWrite(in2, LOW) ;
   digitalWrite(in3, LOW) ;
   digitalWrite(in4, HIGH) ;
   analogWrite ( motorPWM1 ,  90 ) ;
   analogWrite ( motorPWM2 ,  90 ) ; 
   delay(del);
}

void right(int del){
   digitalWrite(in1, LOW) ;
   digitalWrite(in2, HIGH) ;
   digitalWrite(in3, HIGH) ;
   digitalWrite(in4, LOW) ; 
   analogWrite ( motorPWM1 ,  90 ) ;
   analogWrite ( motorPWM2 ,  90 ) ; 
   delay(del);
}

void forward(int del) {
  digitalWrite ( in1 ,  HIGH ) ;
  digitalWrite ( in2 ,  LOW ) ; 
  digitalWrite ( in3 ,  HIGH ) ;   
  digitalWrite ( in4 ,  LOW ) ; 
  analogWrite ( motorPWM1 ,  70 ) ;
  analogWrite ( motorPWM2 ,  70 ) ; 
  delay(del);
}

void back(int del){
  digitalWrite ( in1 ,  LOW ) ; 
  digitalWrite ( in2 ,  HIGH ) ;   
  digitalWrite ( in3 ,  LOW ) ; 
  digitalWrite ( in4 ,  HIGH ) ;  
  analogWrite ( motorPWM1 ,  70 ) ;
  analogWrite ( motorPWM2 ,  70 ) ; 
  delay(del);
}

void RIFDAccess()
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "0C 57 10 39") //change here the UID of the card/cards that you want to give access
  {
    if(isUnlocked && !isRunning)
      isUnlocked = false;
    else if(!isUnlocked && !isRunning)
      isUnlocked = true;
    Serial.println("Authorized access");
    Serial.println();
    delay(1000);
  }
 else
 {
    Serial.println("Access denied");
    delay(1000);
  }
}

void engineState()
{
  if(isUnlocked)
  {
    if(digitalRead(engineButton)==HIGH)
      isRunning = true;
    else 
      isRunning = false;
  }
}

void seatBelt(){
  if(digitalRead(seatbelt)==HIGH){
    digitalWrite(buzzPin, HIGH);
    delay(200);
    digitalWrite(buzzPin,LOW);
    delay(400);
  }
  else
    digitalWrite(buzzPin,LOW);
}

void lightSensor(){
  if(analogRead(ldr)>450)
    digitalWrite(led,HIGH);
  else
  digitalWrite(led,LOW);
}

void moveCar()
{
  if (readDistance1() < 20 && readDistance2() < 20) {
          Stop(300); 
          back(500);
          left(300);
          forward(200);
          //Serial.println("both");
        }
        else if (readDistance1() < 50) {
          Stop(300);
          left(200);
          //Serial.println("right");
        }
        else if (readDistance2() < 50) {
          Stop(300);
          right(200);
          //Serial.println("left");
        }
        else{
          forward(200);
          //Serial.println("forward");
        }
}

void task1(void*pvParameters){
  (void)pvParameters;
  while(1){
    RIFDAccess();
    engineState();
    if(isUnlocked)
      digitalWrite(LED_BUILTIN, HIGH);
    else
      digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

void task2(void*pvParameters){
  (void)pvParameters;
  while(1){
    Serial.println("task2");
    if(isUnlocked && isRunning)
      moveCar();
    else
      Stop(1000);
      vTaskDelay(50/portTICK_PERIOD_MS);
  }
}

void task3(void*pvParameters){
  (void)pvParameters;
  while(1){
    Serial.println("task3");
    if(isUnlocked && isRunning){
      lightSensor();
      seatBelt();
    }
    else{
      digitalWrite(led,LOW);
      digitalWrite(buzzPin,LOW);
    }
    vTaskDelay(2000/portTICK_PERIOD_MS);
  }
}

void  loop ( ) 
{ 
//  RIFDAccess();
//  engineState();
//  if(isUnlocked && isRunning)
//  {
//    lightSensor();
//    seatBelt();
//    moveCar();
//  }
//  else
//  {
//    Stop(1000);
//    digitalWrite(led,LOW);
//    digitalWrite(buzzPin,LOW);
//  }
//  if(isUnlocked)
//    digitalWrite(LED_BUILTIN, HIGH);
//  else
//    digitalWrite(LED_BUILTIN, LOW);
}
