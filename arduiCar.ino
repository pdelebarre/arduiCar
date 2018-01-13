
// voiture avec driver L298N

#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>


LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

//#define BACKLIGHT_PIN     13

int ENA=5;//connected to Arduino's port 5(output pwm)
int IN1=1;//connected to Arduino's port 1
int IN2=3;//connected to Arduino's port 3
int ENB=6;//connected to Arduino's port 6(output pwm)
int IN3=4;//connected to Arduino's port 4
int IN4=7;//connected to Arduino's port 7

int SERVOPIN=9;
Servo servo;
int angle=90;
int coef=1; //direction of the sensor

int trig=12;
int echo=11;

int CRUISESPEED=120;
int TURNTIME=1500;
int TURNSPEED=100;

int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);

decode_results results;

boolean manual=false;
boolean IR=false;



void setup(){ 
  Serial.begin(9600);           // set up Serial library at 9600 bps

  irrecv.enableIRIn(); 

  // Switch on the backlight
  // pinMode ( BACKLIGHT_PIN, OUTPUT );
  //digitalWrite ( BACKLIGHT_PIN, HIGH );

  lcd.begin(16,2);               // initialize the lcd 

  // Print a message to the LCD.

  display("Bonjour!");
  // delay(3000);


  pinMode(ENA,OUTPUT);//output 
  pinMode(ENB,OUTPUT); 
  pinMode(IN1,OUTPUT); 
  pinMode(IN2,OUTPUT); 
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT); 

  servo.attach(SERVOPIN);
  servo.write(angle);

  pinMode(echo,INPUT);
  pinMode(trig,OUTPUT);
  digitalWrite(trig,LOW);

  attachInterrupt(0, readIR, CHANGE);


}

void loop()
{ 

  /* if (irrecv.decode(&results)) {
   Serial.println(results.value, HEX);
   //display(String(results.value,HEX).substring(0,5));
   translateIR();
   delay(500);
   irrecv.resume(); // Receive the next value
   } else {*/

  if(IR) {

    if (irrecv.decode(&results)) {
      Serial.println(results.value, HEX);
      //display(String(results.value,HEX).substring(0,5));
      translateIR();
      delay(500);
      irrecv.resume(); // Receive the next value
    }

    IR=false;
  } 
  else{
    if(!manual) {
      //setSpeed(CRUISESPEED);

      int path=findPath();
      delay(1000);
      if(path==-2) goBackward();
      if(path==0) goForward();
      if(path==-1) turnFwdLeft();
      if(path==1) turnFwdRight();
    }
  }
}

void readIR() {  
  IR=true;
}

void translateIR(){
  switch(results.value)
  {

  case 0xFFA25D:  
    Serial.println(" ON/OFF         "); 
    manual=!manual;
    break;

  case 0xFF629D:  
    Serial.println(" CH             "); 
    break;

  case 0xFFE21D:  
    Serial.println(" CH+            "); 
    break;

  case 0xFF22DD:  
    Serial.println(" PREV           "); 
    break;

  case 0xFF02FD:  
    Serial.println(" NEXT           "); 
    break;

  case 0xFFC23D:  
    Serial.println(" PLAY/PAUSE     "); 
    break;

  case 0xFFE01F:  
    Serial.println(" VOL-           "); 
    break;

  case 0xFFA857:  
    Serial.println(" VOL+           "); 
    break;

  case 0xFF906F:  
    Serial.println(" EQ             "); 
    break;

  case 0xFF6897:  
    Serial.println(" 0              "); 
    break;

  case 0xFF9867:  
    Serial.println(" 100+           "); 
    break;

  case 0xFFB04F:  
    Serial.println(" 200+           "); 
    break;

  case 0xFF30CF:  
    Serial.println(" 1: fwd left    "); 
    if(manual) turnFwdLeft();
    break;

  case 0xFF18E7:  
    Serial.println(" 2: go forward  "); 
    if(manual) goForward();
    break;

  case 0xFF7A85:  
    Serial.println(" 3: fwd right   "); 
    if(manual) turnFwdRight();
    break;

  case 0xFF10EF:  
    Serial.println(" 4              "); 
    break;

  case 0xFF38C7:  
    Serial.println(" 5: stop        "); 
    manual=true;
    stop();
    break;

  case 0xFF5AA5:  
    Serial.println(" 6              "); 
    break;

  case 0xFF42BD:  
    Serial.println(" 7: back left   "); 
    if(manual) turnBackLeft();
    break;

  case 0xFF4AB5:  
    Serial.println(" 8              "); 
    if(manual) goBackward();
    break;

  case 0xFF52AD:  
    Serial.println(" 9              "); 
    if(manual) turnBackRight();
    break;

  default: 
    Serial.println(" other button   ");

  }

  delay(500);
  if(manual) stop();


} //END translateIR


int findPath() {
  stop();
  int go=1;
  //check right
  int right=10000;
  int p=0;
  for(int i=0;i<4;i++) {
    p=ping(15*i);
    if(p<right) right = p;
  }
  //check center
  int center=10000;
  for(int i=5;i<9;i++) {
    p=ping(15*i);
    if(p<center) center = p;
  }
  if(center>right) go=0;
  //check left
  int left=10000;
  for(int i=10;i<14;i++) {
    p=ping(15*i);
    if(p<left) left = p;
  }

  if((left<18)&&(center<18)&&(right<18)) go=-2; 
  else {
    if((left>center) && (left>right)) go=-1;
    if((center>left)&&(center>right)) go=0;
    if((right>center)&&(right>left)) go=1;
  }

  Serial.print(right);
  Serial.print(":");
  Serial.print(center);
  Serial.print(":");
  Serial.print(left);
  Serial.print(" so ");
  Serial.print("I'll go ");
  Serial.println(go);

  return go;

}

float ping(int angle) {
  servo.write(angle);

  digitalWrite(trig,LOW);
  delayMicroseconds(2);
  digitalWrite(trig,HIGH);
  delayMicroseconds(10);
  digitalWrite(trig,LOW);
  int distance=pulseIn(echo,HIGH)/58;
  delay(200);

  Serial.print(angle);
  Serial.print(":");
  Serial.println(distance);

  return distance;
}



float ping() {
  setSpeed(0);
  // rotate servo
  angle+=coef*10;
  if (angle>150  || angle < 30) coef=-coef;
  //Serial.println(angle);
  servo.write(angle);

  digitalWrite(trig,LOW);
  delayMicroseconds(2);
  digitalWrite(trig,HIGH);
  delayMicroseconds(10);
  digitalWrite(trig,LOW);
  int distance=pulseIn(echo,HIGH)/58;

  Serial.print(angle);
  Serial.print(":");
  Serial.println(distance);

  return distance;
}

void goForward(){
  display("go forward      ");

  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,HIGH);
  digitalWrite(IN4,LOW);

  analogWrite(ENA, CRUISESPEED);// motor speed  
  analogWrite(ENB, CRUISESPEED);// motor speed 
 delay(1000); 
}

void goBackward(){
  display("go backward     ");

  digitalWrite(IN1,LOW);
  digitalWrite(IN2,HIGH);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,HIGH);

  analogWrite(ENA, CRUISESPEED);// motor speed  
  analogWrite(ENB, CRUISESPEED);// motor speed 
 delay(1000); 

}

void turnBackRight(){
  display("turn back right  ");

  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,HIGH);
  analogWrite(ENA, 0);// motor speed 
  analogWrite(ENB, TURNSPEED);

  delay(TURNTIME);
}

void turnBackLeft(){
  display("turn back left  ");

  digitalWrite(IN1,LOW);
  digitalWrite(IN2,HIGH);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
  analogWrite(ENA, TURNSPEED);
  analogWrite(ENB, 0);// motor speed 

  delay(TURNTIME);
}  


void turnFwdRight(){
  display("turn fwd right ");

  digitalWrite(IN1,LOW);// rotate reverse
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,HIGH);// rotate reverse
  digitalWrite(IN4,LOW);

  analogWrite(ENB, TURNSPEED);
  analogWrite(ENA, 0);// motor speed 

  delay(TURNTIME);
}

void turnFwdLeft(){
  display("turn fwd left ");

  digitalWrite(IN1,HIGH);// rotate reverse
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);// rotate reverse
  digitalWrite(IN4,LOW);
  analogWrite(ENA, TURNSPEED);
  analogWrite(ENB, 0);// motor speed 

  delay(TURNTIME);
}  

void stop(){
  display("stop");
  setSpeed(0);
}

void setSpeed(int s) {
  analogWrite(ENA, s);// motor speed  
  analogWrite(ENB, s);// motor speed  
}

void display(String str) {
  lcd.home();
  lcd.clear();
  lcd.print(str);
}



