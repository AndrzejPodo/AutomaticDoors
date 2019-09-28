#include "Arduino.h"
#include "Button.h"

Button::Button(Label* label){
    this->label = label;
}

void Button::setLabel(Label* label){
    this->label = label;
}
Label* Button::getLabel(){
    return this->label;
}
void Button::setOnPush(void (*func)()){
    this->onPush = func;
}