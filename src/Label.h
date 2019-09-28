#ifndef Label_h
#define Label_h

#include <Arduino.h>

class Label{
  private:
   int x;
   int y;
   String label;
  public:
   Label(int, int, String);
   int getX();
   int getY();
   String getLabel();
   void setX(int);
   void setY(int);
   void setLabel(String);
   
   void render();
};

#endif