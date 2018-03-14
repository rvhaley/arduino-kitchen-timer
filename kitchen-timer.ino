#include <ClickEncoder.h>
#include <TimerOne.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <TM1637Display.h>

#define CLK 2
#define DIO 3

const int BTN_START_PIN = 8;
//const int BTN_STOP_PIN = 8;

const int ENC_PIN_1 = 4;
const int ENC_PIN_2 = 5;
const int ENC_PIN_3 = 6;

ClickEncoder *encoder;
int16_t last, value;

TM1637Display display(CLK, DIO);

uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };

bool started = false;
bool clockTimeSet = false;
bool finished;

int buttonStartState = 0;
//int buttonStopState = 0;

int timerHours = 0;
int timerMinutes =  0;
int timerSeconds = 0;

void timerIsr() {
  encoder->service();  
}

void setup() {
  Serial.begin(9600);
  
  encoder = new ClickEncoder(ENC_PIN_1, ENC_PIN_2, ENC_PIN_3);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 

  last = -1;

  //pinMode(ENC_PUSH_BUTTON, INPUT_PULLUP);
  pinMode(BTN_START_PIN, INPUT);
  //pinMode(BTN_STOP_PIN, INPUT);
}

void loop() {
  if (!started) {
    /*
    display.setBrightness(0x0f);

    data[0] = display.encodeDigit(timerHours / 10);
    data[1] = display.encodeDigit(timerHours % 10);
    data[2] = display.encodeDigit(timerMinutes / 10);
    data[3] = display.encodeDigit(timerMinutes % 10);
  
    display.setSegments(data);
    */

    value += encoder->getValue();
 
    if (value > last) {
      timerMinutes++;
      calculateHours(true);     
    } else if (value < last && timerMinutes > 0) {
      timerMinutes--;
      calculateHours(false);
    }
    
    if (value != last) {
      last = value;
      Serial.print("Encoder value: ");
      Serial.println(value);
    }

    ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open) {
      Serial.print("Button: ");
      #define VERBOSECASE(label) case label: Serial.println(#label);
      switch (b) {
        VERBOSECASE(ClickEncoder::Pressed);
        VERBOSECASE(ClickEncoder::Held)
        VERBOSECASE(ClickEncoder::Released)
        VERBOSECASE(ClickEncoder::Clicked)
        case ClickEncoder::DoubleClicked:
          Serial.println("ClickEncoder::DoubleClicked");
          encoder->setAccelerationEnabled(!encoder->getAccelerationEnabled());
          Serial.print("  Acceleration is ");
          Serial.println((encoder->getAccelerationEnabled()) ? "enabled" : "disabled");  
      } 
    }

      buttonStartState = digitalRead(BTN_START_PIN);
//    buttonStopState = digitalRead(BTN_STOP_PIN);

    if (buttonStartState == LOW) {
      //timerMinutes--;
      //calculateHours(false);
      //delay(300);
      Serial.println("Start button pressed");
      started = true;
    }
  /*    
    if (buttonStopState == HIGH) {
      //timerMinutes++;
      //calculateHours(true);
      //delay(300);
      Serial.println("Stop button pressed");
    }
  */  
  } else {
    finished = isTimerFinished();
    
    if (!finished) {
      Alarm.delay(1000);
      decreaseTime();
      calculateHours(false);
    }  
  }

    display.setBrightness(0x0f);

   
    display.showNumberDecEx(timeToInteger(), (0x80 >> 1), (timerMinutes / 10) == 0);
/*
    data[0] = display.encodeDigit(timerHours / 10);
    data[1] = display.encodeDigit(timerHours % 10);
    data[2] = display.encodeDigit(timerMinutes / 10);
    data[3] = display.encodeDigit(timerMinutes % 10);
  
    display.setSegments(data);
  */
}

int timeToInteger() {
  int result = 0;
  result += timerMinutes * 100;
  result += timerSeconds;
  return result;
}

void decreaseTime() {
  Serial.print(timerHours);
  Serial.print(" Hours ");
  Serial.print(timerMinutes);
  Serial.print(" Minutes ");
  Serial.print(timerSeconds);
  Serial.println(" Seconds");
  if (!finished)
    timerSeconds--;
}

void calculateHours(bool asce) {
  if (timerSeconds == -1 && !asce) {
    // decrement minutes
    if (timerMinutes > 0 || timerHours > 0) {
      timerMinutes--;
      timerSeconds = 59; 
    }   
  }
  
  if (timerMinutes == 60 && asce) {
    timerHours++;
    timerMinutes = 0;  
  } else if (timerMinutes == -1 && !asce && timerHours > 0) {
    timerHours--;
    timerMinutes = 59;
  }
}

bool isTimerFinished() {
  return timerHours == 0 && timerMinutes == 0 && timerSeconds == 0;  
}
  
