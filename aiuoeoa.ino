#include <SPI.h>
#include <LoRa.h>

#define trig A0
#define limrd A1
#define limru A2

#define limld A4
#define limlu A3

#define echol 8
#define echor 7

#define runl 5
#define stopr 3
#define downl 6
#define dirr 4


char* message;
int parkcount;
bool upl;
bool upr;


void setup() {

  pinMode(trig, OUTPUT);
  for (int i = 3; i < 7; i++) {
    pinMode(i, OUTPUT);
  }
  for (int i = 7; i < 9; i++) {
    pinMode(i, INPUT);
  }
  for (int i = 0; i < 2; i++) {
    pinMode(i, INPUT);
  }

  LoRa.setSPIFrequency(5000000);

  if (!LoRa.begin(433E6)) {
    while (1);
  }
  message = calloc(10, sizeof(char));
  LoRa.onReceive(readcomm);
  LoRa.receive();
  parkcount = 0;
}
void clearmsg() {
  for (int i = 0; i < 10; i++) {
    *(message + i) = '\0';
  }
}

void readcomm(int packetsize) {
  clearmsg();
  if (LoRa.available()) {
    for (int i = 0; i < 9; i++) {
      *(message + i) = (char)LoRa.read();
    }
  }
  if (verify()) {
    Serial.println("wow car");
    parkcount-=(parkcount>0)?1:0;
  }
}

bool verify() {
  char* keywod = "1   696";
  bool valid = true;
  for (int i = 0; i < 7; i++) {
    if (*(message + i) != *(keywod + i)) {
      valid = false;
    }
  }
  if (*(message + 7) != '\r') {
    valid = false;
  }
  return valid;
}

bool blocked(bool right) {
  digitalWrite(trig, 0);
  delayMicroseconds(2);
  digitalWrite(trig, 1);
  delayMicroseconds(10);
  digitalWrite(trig, 0);

  float duration = (right) ? pulseIn(echor, 1) : pulseIn(echol, 1);
  if (duration * .0343 / 2 < 50) {
    return true;
  }
  return false;
}


bool lup = false;
bool rup = false;
bool count = false;
void loop() {

  int type = PIND & 3;
  if (type == 1) {  //car
    if (parkcount < 2) {
      //rup = true;
    } else {
      lup = true;
    }
  } else if (type == 2) {  //other
    //rup = true;
  } else if (type == 3) {  //vip
      lup = true;
  }
  handlemotor(type);
}
/*
void temphandlemotor(int type) {

  if (lup) {
    digitalWrite(runl, 1);
    digitalWrite(downl, 0);
  }
  if (rup) {
    digitalWrite(stopr, 0);
    digitalWrite(upr, 1);
  }

  delay(2000);
  //digitalWrite(stopr, 1);
  digitalWrite(runl, 0);
  while (lup || rup) {

    if (!blocked(0) && lup) {
      lup = false;
    }
    if (!blocked(1) && rup) {
      if (type == 1 || type == 3) {
        count = true;
      }
      rup = false;
    }

    if (!lup && !rup) {
      break;
    }
  }
  digitalWrite(stopr, 0);
  digitalWrite(runl, 1);
  digitalWrite(downl, 1);
  digitalWrite(upr, 0);

  delay(2000);
  //digitalWrite(stopr, 1);
  digitalWrite(runl, 0);
  parkcount += (count) ? 1 : 0;
  count = false;
}*/


void handlemotor(int type) {
  while (1) {
    if (lup && !digitalRead(limlu)) {
      digitalWrite(runl, 1);
      digitalWrite(downl, 0);
    } else if (!lup && !digitalRead(limld)) {
      digitalWrite(runl, 1);
      digitalWrite(downl, 1);
    } else {
      digitalWrite(runl, 0);
    }
    if (rup && !digitalRead(limru)) {
      //digitalWrite(stopr, 0);
      digitalWrite(upr, 1);
    } else if (!rup && !digitalRead(limrd)) {
      //digitalWrite(stopr, 0);
      digitalWrite(upr, 0);
    } else {
      //digitalWrite(stopr, 1);
    }

    delay(800);
    if (!blocked(0) && lup) {
      lup = false;
    }
    if (!blocked(1) && rup) {
      if (type == 1 || type == 3) {
        count = true;
      }
      rup = false;
    }

    if (digitalRead(limrd) && digitalRead(limld) && !lup && !rup) {
      break;
    }
  }
  parkcount += (count) ? 1 : 0;
  count = false;
}