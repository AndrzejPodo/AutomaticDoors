#include <Arduino.h>
#include <Label.h>
#include <Hour.h>

void Hour::setLabelPosition(int x, int y){
    this->label->setX(x);
    this->label->setY(y);
}

void Hour::incrementMinutes(){
    if(this->min == 59){
    this->min = 0;
    this->incrementHours();
  }
  else this->min++;
}
void Hour::incrementHours(){
    if(this->hour == 23)this->hour = 0;
    else this->hour++;
}

Hour::Hour(int x, int y, int h, int m){
    String text = "";
    if(h < 10){
        text+="0";
    }
    text+=h;
    text+=":";
    if(m < 10){
        text += "0";
    }
    text+=m;

    this->label = new Label(x,y,text);
    this->min = m;
    this->hour = h;
}

Label Hour::getTimeLabel(){
    String text = "";
    if(this->hour < 10){
        text+="0";
    }
    text+=this->hour;
    text+=":";
    if(this->min < 10){
        text += "0";
    }
    text+=this->min;
    this->label->setLabel(text);
    return *this->label;
}

int Hour::getMin(){
    return this->min;
}
int Hour::getHour(){
    return this->hour;
}
void Hour::setMin(int min){
    this->min = min;
}
void Hour::setHour(int hour){
    this->hour = hour;
}