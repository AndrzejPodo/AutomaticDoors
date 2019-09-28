#ifndef Hour_h
#define Hour_h

#include <Arduino.h>
#include <Label.h>

class Hour{
    private:
        int min;
        int hour;
        Label *label;
    public:
        Hour(int, int, int, int);
        int getMin();
        int getHour();
        void setMin(int min);
        void setHour(int hour);
        Label getTimeLabel();
        void incrementMinutes();
        void incrementHours();
        void setLabelPosition(int,int);
};

#endif