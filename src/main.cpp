#include <Arduino.h>
#include <LiquidCrystal.h>
#include "Label.h"
#include "Button.h"
#include "Hour.h"
#include <EEPROM.h>

#define BLINK_FREQ 15
#define ENTER 2
#define INC 3
#define DOORS_UP 10
#define DOORS_DOWN 11
#define DOORS_BUTTON_UP A1
#define DOORS_BUTTON_DOWN A2
#define MOTOR_SENSOR 0
#define DOOR_MOVEMENT_RANGE 20
/*
 * deklaracja zmiennnych i enumów
 */

enum screen {MAIN_SCREEN, SETUP_SCREEN, OPEN_HOUR_SET, CLOSE_HOUR_SET, CURRENT_HOUR_SET};
enum button {CURRENT, CLOSE, OPEN};
enum eeprom_adresses {CURRENT_H, CURRENT_M, CLOSE_H, CLOSE_M, OPEN_H, OPEN_M, DOORS_POSITION};

int SCREEN;
int BUTTON;

bool CALIBRATED = false;
uint8_t doorsPosition;
int cursorPos = 0;

byte full[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};
byte arrowUp[8] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
};
byte arrowDown[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100,
};

bool DOORS_MOVING_FLAG = false;

/*
 * deklaracja funckji pomocniczych
 */

//funckje renderuyjace wydoki
void mainScreenRender();
void setupScreenRender();
void currentTimeSetupRender();
void closeTimeSetupRender();
void openTimeSetupRender();


void openDoors();
void closeDoors();
void stopDoors();
void blinkWord(int, int, int);
void buttonOnMainScreen();
void navigateToOpenHourSetScreen();
void navigateToCurrentHourSetScreen();
void navigateToCloseHourSetScreen();
void enter();
void inc();
void changeScreen(int);
void renderLabel(Label label);


/*
 * Tworzenie obiektów
 */
LiquidCrystal lcd(8,9,4,5,6,7);

Hour currentTime(5,0,12,0);
Hour openTime(0,1,6,0);
Hour closeTime(11,1,18,0);
Hour currentTimeCopy(5,1,12,0);
Hour openTimeCopy(5,1,6,0);
Hour closeTimeCopy(5,1,18,0);

Button mainScreenButton(new Label(6,1,"UST"));
Button currentHourSetButton(new Label(0,1, "OBEC"));
Button closeHourSetButton(new Label(6,1, "ZAMK"));
Button openHourSetButton(new Label(12,1, "OTWI"));

/*
 * ====================================SETUP==================================================
 */

void setup() {
  cli();
  Serial.begin(9600);
  pinMode(DOORS_BUTTON_DOWN, INPUT_PULLUP);
  pinMode(DOORS_BUTTON_UP, INPUT_PULLUP);
  pinMode(DOORS_UP, OUTPUT);
  pinMode(DOORS_DOWN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ENTER, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENTER), enter, FALLING);
  pinMode(INC, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INC), inc, FALLING);
  
  // put your setup code here, to run once:
  lcd.begin(16,2);
  lcd.createChar(0, full);
  lcd.createChar(1, arrowUp);
  lcd.createChar(2, arrowDown);

  mainScreenButton.setOnPush(buttonOnMainScreen);
  currentHourSetButton.setOnPush(navigateToCurrentHourSetScreen);
  openHourSetButton.setOnPush(navigateToOpenHourSetScreen);
  closeHourSetButton.setOnPush(navigateToCloseHourSetScreen);

  //Setup timera1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  TIMSK1 |= (1 << OCIE1A);

  doorsPosition = (EEPROM.read(DOORS_POSITION) == 255)?DOOR_MOVEMENT_RANGE:EEPROM.read(DOORS_POSITION);

  if(EEPROM.read(CURRENT_H) < 24){
    currentTime.setHour(EEPROM.read(CURRENT_H));
  }else{
    EEPROM.write(CURRENT_H, 12);
  }
  if(EEPROM.read(OPEN_H) < 24){
    openTime.setHour(EEPROM.read(OPEN_H));
  }else{
    EEPROM.write(OPEN_H, 6);
  }
  if(EEPROM.read(CLOSE_H) < 24){
    closeTime.setHour(EEPROM.read(CLOSE_H));
  }else{
    EEPROM.write(CLOSE_H, 18);
  }
  if(EEPROM.read(CURRENT_M) < 60){
    currentTime.setMin(EEPROM.read(CURRENT_M));
  }else{
    EEPROM.write(CURRENT_M, 0);
  }
  if(EEPROM.read(OPEN_M) < 60){
    openTime.setMin(EEPROM.read(OPEN_M));
  }else{
    EEPROM.write(OPEN_M, 0);
  }
  if(EEPROM.read(CLOSE_M) < 60){
    closeTime.setMin(EEPROM.read(CLOSE_M));
  }else{
    EEPROM.write(CLOSE_M, 0);
  }


  sei();
  SCREEN = MAIN_SCREEN;
}

void loop() {
  lcd.clear();

  if(digitalRead(DOORS_BUTTON_DOWN) == LOW && digitalRead(DOORS_BUTTON_UP) == HIGH && SCREEN == MAIN_SCREEN && doorsPosition > 0){
    closeDoors();
    doorsPosition--;
  }else if(digitalRead(DOORS_BUTTON_UP) == LOW && digitalRead(DOORS_BUTTON_DOWN) == HIGH && SCREEN == MAIN_SCREEN && doorsPosition < 2*DOOR_MOVEMENT_RANGE){
    openDoors();
    doorsPosition++;
    if(doorsPosition == DOOR_MOVEMENT_RANGE-3){
        Serial.println("Dodałem");
        doorsPosition+=3;
    }
  }
  // }else if (digitalRead(DOORS_BUTTON_DOWN) == HIGH && digitalRead(DOORS_BUTTON_UP) == HIGH) {
  //   stopDoors();
  // }
  else if(currentTime.getTimeLabel().getLabel() == openTime.getTimeLabel().getLabel()){
    while(doorsPosition < 2*DOOR_MOVEMENT_RANGE){
      openDoors();
      doorsPosition++;
      if(doorsPosition == DOOR_MOVEMENT_RANGE-3){
        Serial.println("Dodałem");
        doorsPosition+=3;
      }
    }
  }
  else if(currentTime.getTimeLabel().getLabel() == closeTime.getTimeLabel().getLabel()){
    while(doorsPosition > 0){
      closeDoors();
      doorsPosition--;
    }
  }else{
    if(EEPROM.read(DOORS_POSITION) != doorsPosition){
      EEPROM.update(DOORS_POSITION, doorsPosition);
      Serial.println(EEPROM.read(DOORS_POSITION));
    }
  }

  if(!CALIBRATED){
    switch(SCREEN){
      case MAIN_SCREEN:
        mainScreenRender();
        break;
      case SETUP_SCREEN:
        setupScreenRender();
        break;
      case OPEN_HOUR_SET:
        openTimeSetupRender();
        break;
      case CLOSE_HOUR_SET:
        closeTimeSetupRender();
        break;
      case CURRENT_HOUR_SET:
        currentTimeSetupRender();
        break; 
    }
  }else{
    lcd.setCursor(0,0);
    lcd.print("KALIBRACJA!!!");
    delay(1000);
    CALIBRATED = false;
  }

  // if(digitalRead(DOORS_UP) || digitalRead(DOORS_DOWN)){
  //   Serial.println(analogRead(MOTOR_SENSOR));
  // }
  delay(20);
}

ISR(TIMER1_COMPA_vect){
  static int seconds;
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN)^1);
  seconds++;
  if(seconds == 60){
    currentTime.incrementMinutes();
    EEPROM.update(CURRENT_H, currentTime.getHour());
    EEPROM.update(CURRENT_M, currentTime.getMin());
    seconds = 0;
  }
}

void renderLabel(Label label){
  lcd.setCursor(label.getX(), label.getY());
  lcd.print(label.getLabel());
}

void blinkWord(int x, int y, int len){
  for(int i = x; i < x+len; i++){
    lcd.setCursor(i,y);
    lcd.write(byte(0));
  }
}

void mainScreenRender(){
  static bool blinkHelper;
  static int blinkCountHelper;

  currentTime.setLabelPosition(5,0);
  closeTime.setLabelPosition(11,1);
  openTime.setLabelPosition(0,1);

  renderLabel(currentTime.getTimeLabel());
  renderLabel(closeTime.getTimeLabel());
  renderLabel(openTime.getTimeLabel());

  if(blinkHelper){
    blinkWord(6,1,3);}
  else{
    //setupButton.render();
    renderLabel(*mainScreenButton.getLabel());
  }
  if(blinkCountHelper > BLINK_FREQ){
    blinkHelper = blinkHelper^true;
    blinkCountHelper = 0;
  }
  blinkCountHelper++;
}

void setupScreenRender(){
  static bool blinkHelper;
  static int blinkCountHelper;

  lcd.setCursor(3,0);
  lcd.print("USTAW CZAS");

  renderLabel(*currentHourSetButton.getLabel());
  renderLabel(*closeHourSetButton.getLabel());
  renderLabel(*openHourSetButton.getLabel());
  
  if(blinkHelper){
    if(BUTTON == CURRENT){
      blinkWord(0,1,4);
    }else{
      renderLabel(*currentHourSetButton.getLabel());
    }
    if(BUTTON == OPEN){
      blinkWord(12,1,4);
    }else{
      renderLabel(*openHourSetButton.getLabel());
    }
    if(BUTTON == CLOSE){
      blinkWord(6,1,4);
    }else{
      renderLabel(*closeHourSetButton.getLabel());
    }
  }
  if(blinkCountHelper > BLINK_FREQ){
    blinkHelper = blinkHelper^true;
    blinkCountHelper = 0;
  }
  blinkCountHelper++;
}

void currentTimeSetupRender(){
  static bool blinkHelper;
  static int blinkCountHelper;
  lcd.print("OBECNY CZAS");

  renderLabel(currentTimeCopy.getTimeLabel());
  
  if(blinkHelper){
    blinkWord(5+cursorPos*3,1,2);
  }
  
  if(blinkCountHelper > BLINK_FREQ){
    blinkHelper = blinkHelper^true;
    blinkCountHelper = 0;
  }
  blinkCountHelper++;
}

void closeTimeSetupRender(){
  static bool blinkHelper;
  static int blinkCountHelper;
  lcd.print("CZAS ZAMKNIECIA");

  renderLabel(closeTimeCopy.getTimeLabel());

  if(blinkHelper){
    blinkWord(5+cursorPos*3,1,2);
  }
  
  if(blinkCountHelper > BLINK_FREQ){
    blinkHelper = blinkHelper^true;
    blinkCountHelper = 0;
  }
  blinkCountHelper++;
}

void openTimeSetupRender(){
  static bool blinkHelper;
  static int blinkCountHelper;
  lcd.print("CZAS OTWARCIA");

  renderLabel(openTimeCopy.getTimeLabel());

  if(blinkHelper){
    blinkWord(5+cursorPos*3,1,2);
  }
  
  if(blinkCountHelper > BLINK_FREQ){
    blinkHelper = blinkHelper^true;
    blinkCountHelper = 0;
  }
  blinkCountHelper++;
}

void buttonOnMainScreen(){
  changeScreen(SETUP_SCREEN);
}
void navigateToOpenHourSetScreen(){
  openTimeCopy.setHour(openTime.getHour());
  openTimeCopy.setMin(openTime.getMin());
  changeScreen(OPEN_HOUR_SET);
}
void navigateToCurrentHourSetScreen(){
  currentTimeCopy.setHour(currentTime.getHour());
  currentTimeCopy.setMin(currentTime.getMin());
  changeScreen(CURRENT_HOUR_SET);
}
void navigateToCloseHourSetScreen(){
  closeTimeCopy.setHour(closeTime.getHour());
  closeTimeCopy.setMin(closeTime.getMin());
  changeScreen(CLOSE_HOUR_SET);
}

void enter(){
  static long prevTime;
  long cTime = millis();
  long delta = cTime-prevTime;
  prevTime = cTime;
  if(delta > 500 && !DOORS_MOVING_FLAG){
    switch(SCREEN){
      case MAIN_SCREEN:
        mainScreenButton.onPush();
        break;
      case SETUP_SCREEN:
        if(BUTTON == CURRENT){
          currentHourSetButton.onPush();
        }else if(BUTTON == OPEN){
          openHourSetButton.onPush();
        }else {
          closeHourSetButton.onPush();
        }
        break;
      case OPEN_HOUR_SET:
        if(cursorPos == 1){
          cursorPos = 0;
          openTime.setHour(openTimeCopy.getHour());
          openTime.setMin(openTimeCopy.getMin());
          EEPROM.update(OPEN_H, (uint8_t)openTime.getHour());
          EEPROM.update(OPEN_M, (uint8_t)openTime.getMin());
          changeScreen(MAIN_SCREEN);
        } else{
          cursorPos++;
        }
        break;
      case CLOSE_HOUR_SET:
        if(cursorPos == 1){
          cursorPos = 0;
          closeTime.setHour(closeTimeCopy.getHour());
          closeTime.setMin(closeTimeCopy.getMin());
          EEPROM.update(CLOSE_H, (uint8_t)closeTime.getHour());
          EEPROM.update(CLOSE_M, (uint8_t)closeTime.getMin());
          changeScreen(MAIN_SCREEN);
        } else{
          cursorPos++;
        }
        break;
      case CURRENT_HOUR_SET:
        if(cursorPos == 1){
          cursorPos = 0;
          currentTime.setHour(currentTimeCopy.getHour());
          currentTime.setMin(currentTimeCopy.getMin());
          EEPROM.update(CURRENT_H, (uint8_t)currentTime.getHour());
          EEPROM.update(CURRENT_M, (uint8_t)currentTime.getMin());
          changeScreen(MAIN_SCREEN);
        }else{
          cursorPos++;
        }
        break;
    }
  }
}

void inc(){
  static long prevTime;
  long cTime = millis();
  long delta = cTime-prevTime;
  prevTime = cTime;
  if(delta > 200 && !DOORS_MOVING_FLAG){
    switch(SCREEN){
      case MAIN_SCREEN:
        doorsPosition = DOOR_MOVEMENT_RANGE;
        EEPROM.update(DOORS_POSITION, doorsPosition);
        CALIBRATED = true;
        break;
      case SETUP_SCREEN:
        if(BUTTON == CURRENT){
          BUTTON = CLOSE;
        }else if(BUTTON == CLOSE){
          BUTTON = OPEN;
        }else{
          BUTTON = CURRENT;
        }
        break;
       case OPEN_HOUR_SET:
        if(cursorPos == 0){
          openTimeCopy.incrementHours();
        }else{
          openTimeCopy.incrementMinutes();
        }
        break;
       case CLOSE_HOUR_SET:
        if(cursorPos == 0){
          closeTimeCopy.incrementHours();
        }else{
          closeTimeCopy.incrementMinutes();
        }
        break;
       case CURRENT_HOUR_SET:
        if(cursorPos == 0){
          currentTimeCopy.incrementHours();
        }else{
          currentTimeCopy.incrementMinutes();
        }
        break;
    }
  }
}

void changeScreen(int screen){
  SCREEN = screen;
}

void openDoors(){
  DOORS_MOVING_FLAG = true;
  lcd.setCursor(0,0);
  lcd.write(byte(2));
  lcd.setCursor(15,0);
  lcd.write(byte(2));
  digitalWrite(DOORS_DOWN, LOW);
  digitalWrite(DOORS_UP, HIGH); 
  delay(500);
  stopDoors();
}
void closeDoors(){
  DOORS_MOVING_FLAG = true;
  lcd.setCursor(1,0);
  lcd.write(byte(1));
  lcd.setCursor(14,0);
  lcd.write(byte(1));
  digitalWrite(DOORS_UP, LOW);
  digitalWrite(DOORS_DOWN, HIGH);
  delay(500);
  stopDoors();
}
void stopDoors(){
  DOORS_MOVING_FLAG = false;
  digitalWrite(DOORS_DOWN, LOW);
  digitalWrite(DOORS_UP, LOW);
}