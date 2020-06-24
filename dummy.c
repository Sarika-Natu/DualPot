//
// Created by Sarika on 6/18/20.
//
#include <stdio.h>
#include "Pin.h"
#include "Periodic.h"


void	PeriodicModuleInit	(void){
    printf("PeriodicModuleInit\n");
}
void	PeriodicConfig			(f32	FreqHz, PeriodicHandlerT Handler){
    printf("PeriodicConfig: FreqHz= %f\n", FreqHz);
}
void	PeriodicStart			(void){
    printf("PeriodicStart\n");
}
void	PeriodicStop			(void){
    printf("PeriodicStop\n");
}
void	PeriodicIruptEnable	(void){
    printf("PeriodicIruptEnable\n");
}
void	PeriodicIruptDisable	(void){
    printf("PeriodicIruptDisable\n");
}
void	PeriodicIruptFlagClear	(void){
    printf("PeriodicIruptFlagClear\n");
}



void    PinModuleInit   (void){
    printf("PinModuleInit Called!\n");
}
void	PinWrite		(PinT Pin, bool Value){
    printf("Pin number is: %d, Written with value: %d\n", Pin, Value);
}