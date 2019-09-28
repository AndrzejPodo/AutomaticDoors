#include "Arduino.h"
#include "Label.h"

Label::Label(int x, int y, String label){
  this->x = x;
  this->y = y;
  this->label = label;
}

void Label::setX(int x){
  this->x = x;
}
void Label::setY(int y){
  this->y = y;
}
void Label::setLabel(String label){
  this->label = label;
}

int Label::getX(){
  return this->x;
}
int Label::getY(){
  return this->y;
}
String Label::getLabel(){
  return this->label;
}