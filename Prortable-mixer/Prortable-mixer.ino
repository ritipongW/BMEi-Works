//old pin define
//int in1 = 5;
//int in2 = 4;
//int in3 = 3;
//int in4 = 2;
//int in5 = 9;
//int in6 = 10;


int in1 = 5; //pin for every 1 minutes
int in2 = 3; //pin for every 5 minutes
int in3 = 10; //pin for every 10 minutes
int in4 = 4; //pin for every 20 minutes
int in5 = 2; //pin for every 30 minutes
int in6 = 9; //no spin


long p = 0;

unsigned long last_time1 = 0, last_time2 = 0, last_time3 = 0, last_time4 = 0, last_time5 = 0;  // Check every 6 hr.
unsigned long myTime1, myTime2, myTime3, myTime4, myTime5;

long time_init = 60000;

long time_chk1 = time_init * 1, time_chk2 = time_init * 2, time_chk3 = time_init * 3, time_chk4 = time_init * 4, time_chk5 = time_init * 5;


String msg = "";

int StS1 = 0, StS2 = 0, StS3 = 0, StS4 = 0, StS5 = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(in5, OUTPUT);
  pinMode(in6, OUTPUT);
  StS1 = 0;
  StS2 = 0;
  StS3 = 0;
  StS4 = 0;
  StS5 = 0;

}



void loop() {

  if (StS1 == 0) {
    StS1 = 1;
    digitalWrite(in1, HIGH);
    delay(100);
    digitalWrite(in1, LOW);
  }
  chk_avalible1();

  if (StS2 == 0) {
    StS2 = 1;
    digitalWrite(in2, HIGH);
    delay(100);
    digitalWrite(in2, LOW);
  }
  chk_avalible2();

  if (StS3 == 0) {
    StS3 = 1;
    digitalWrite(in3, HIGH);
    delay(100);
    digitalWrite(in3, LOW);
  }
  chk_avalible3();

  if (StS4 == 0) {
    StS4 = 1;
    digitalWrite(in4, HIGH);
    delay(100);
    digitalWrite(in4, LOW);
  }
  chk_avalible4();

  if (StS5 == 0) {
    StS5 = 1;
    digitalWrite(in5, HIGH);
    delay(100);
    digitalWrite(in5, LOW);
  }
  chk_avalible5();
}

void chk_avalible1() {
  myTime1 = millis();
  if (myTime1 - last_time1 >= (time_chk1 / 2)) {
    last_time1 = myTime1;
    StS1 = 0;
  }
}

void chk_avalible2() {
  myTime2 = millis();
  if (myTime2 - last_time2 >= (time_chk2 / 2)) {
    last_time2 = myTime2;
    StS2 = 0;
  }
}

void chk_avalible3() {
  myTime3 = millis();
  if (myTime3 - last_time3 >= (time_chk3 / 2)) {
    last_time3 = myTime3;
    StS3 = 0;
  }
}

void chk_avalible4() {
  myTime4 = millis();
  if (myTime4 - last_time4 >= (time_chk4 / 2)) {
    last_time4 = myTime4;
    StS4 = 0;
  }
}

void chk_avalible5() {
  myTime5 = millis();
  if (myTime5 - last_time5 >= (time_chk5 / 2)) {
    last_time5 = myTime5;
    StS5 = 0;
  }
}
