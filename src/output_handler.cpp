#include <Arduino.h>

#include "output_handler.h"

#define MD1_OUT1 32 //GPIO #32
#define MD1_OUT2 33 //GPIO #33
#define MD2_OUT1 25 //GPIO #25
#define MD2_OUT2 26 //GPIO #26

char output_s[100];

// setup()で行う必要あり
void OutputSetup()
{
    // Setup Pins
    pinMode(MD1_OUT1, OUTPUT);
    pinMode(MD1_OUT2, OUTPUT);
    pinMode(MD2_OUT1, OUTPUT);
    pinMode(MD2_OUT2, OUTPUT);
}

void HandleOutput(motion predict_motion)
{
    if (predict_motion == PAPER)
    {
        OutputType01();
    }
    else if (predict_motion == ROCK)
    {
        OutputType10();
    }
    else
    {
        OutputType00();
    }
}

void OutputType00()
{
    Serial.println("out1 = 0, out2 = 0;");
    digitalWrite(MD1_OUT1, LOW);
    digitalWrite(MD1_OUT2, LOW);
    digitalWrite(MD2_OUT1, LOW);
    digitalWrite(MD2_OUT2, LOW);
}

void OutputType10()
{
    Serial.println("out1 = 1, out2 = 0;");
    digitalWrite(MD1_OUT1, HIGH);
    digitalWrite(MD1_OUT2, LOW);
    digitalWrite(MD2_OUT1, HIGH);
    digitalWrite(MD2_OUT2, LOW);
}

void OutputType01()
{
    Serial.println("out1 = 0, out2 = 1;");
    digitalWrite(MD1_OUT1, LOW);
    digitalWrite(MD1_OUT2, HIGH);
    digitalWrite(MD2_OUT1, LOW);
    digitalWrite(MD2_OUT2, HIGH);
}

void OutputType11()
{
    Serial.println("out1 = 1, out2 = 1;");
    digitalWrite(MD1_OUT1, HIGH);
    digitalWrite(MD1_OUT2, HIGH);
    digitalWrite(MD2_OUT1, HIGH);
    digitalWrite(MD2_OUT2, HIGH);
}