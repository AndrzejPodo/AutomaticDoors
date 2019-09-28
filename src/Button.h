#ifndef Button_h
#define Button_h

#include <Arduino.h>
#include <Label.h>


class Button{
    private:
        Label *label;
    public:
        Button(Label*);
        void (*onPush)();
        void setLabel(Label*);
        Label* getLabel();
        void setOnPush(void (*func)());
};

#endif