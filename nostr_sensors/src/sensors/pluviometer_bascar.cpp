#include "sensors/pluviometer_bascar.h"

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static volatile int readings = 0;
static volatile int flux_adjust = 0;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

hw_timer_t *minTimer = NULL;
hw_timer_t *hrTimer = NULL;
hw_timer_t *dayTimer = NULL;

static volatile int mReading;
static volatile int hReading;
static volatile int dReading;

void IRAM_ATTR pulseCounter()
{
    portENTER_CRITICAL_ISR(&spinlock);  
    readings++;
    portEXIT_CRITICAL_ISR(&spinlock);
}

void IRAM_ATTR mTimer(){
    mReading = readings - mReading;
}

void pluviometer_init(){
    pinMode(PLUV_PIN, INPUT_PULLUP);        
    attachInterrupt(digitalPinToInterrupt(PLUV_PIN), pulseCounter, FALLING);
}

String pluviometer_read(){
    int reading_tmp;
    portENTER_CRITICAL_ISR(&spinlock);  
    reading_tmp = readings;
    portEXIT_CRITICAL_ISR(&spinlock);
    float reading_15, reading_1h, reading_1d;
}