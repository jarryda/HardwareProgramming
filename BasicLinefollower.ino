//////////////BASIC BOT-CH////////////////////

//////////////////GLOBAL/////////////////////////
const int QTEYE =7;
const int SONAR = 10; //LEFT OUTER QTI IS PIN 9
byte QTIREAD;    //declare outputs for qti read
#include <Servo.h>
Servo Left; //declare left servo
Servo Right; //declare right servo
long duration;  //storage for sonar calc
long cm;    //distance for sonar calc
int L;    //servo speed Left
int R;    //servo speed Right
/////////////////////SETUP//////////////////
void setup() {
  Serial.begin(9600);
Left.attach(13); //attach left to pin 13
Right.attach(12); //attach right to pin 12
}
///////////////////MATH//////////////////////////
long microsecondsToCentimeters(long microseconds) //calculates microseconds as centimeters
{
  return microseconds/29/2;
}
/////////////////////PING SENSOR//////////////////

void CenterPing()   //runs ping sensor
{
 pinMode(SONAR, OUTPUT);
 digitalWrite(SONAR, LOW);
 delayMicroseconds(2);
 digitalWrite(SONAR, HIGH);
 delayMicroseconds(5);
 digitalWrite(SONAR,LOW);
 pinMode(SONAR,INPUT);
 duration = pulseIn(SONAR, HIGH);
 cm = microsecondsToCentimeters(duration);
}

void PINGSERVO()    //designates servo speeds
{
  if (cm<10)
 {
  L=1500;
  R=1500;
 }
  else
  {
   L=L;
   R=R;
}
}
//////////////////QTI SUBS//////////////////////
void QTI()    //runs qti
{
pinMode(QTEYE, OUTPUT);
digitalWrite(QTEYE, HIGH);  // Charge line follower capacitors
delay(1);
pinMode(QTEYE, INPUT);  //stop charging
delay(1);
QTIREAD=digitalRead(QTEYE);
}
void QTISERVO()   //sets servo variables
{
  if(QTIREAD == 1)   //black
  {
    L=1550;
    R=1500;
  }
  else if (QTIREAD == 0) //white
  {
    L=1500;
    R=1450;
  }
}
void SERVO()    //run servo
{
Left.writeMicroseconds(L);
Right.writeMicroseconds(R);
}
///////////////////MAIN/////////////////////////
void loop() {
  QTI();
  QTISERVO(); 
  CenterPing();
  PINGSERVO();
  SERVO();
}