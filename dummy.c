//
// Created by Sarika on 6/18/20.
//
#include "Pin.h"
#include "Generic.h"
#include "Periodic.h"

void	PeriodicConfig			(f32	FreqHz, PeriodicHandlerT Handler){}
void	PeriodicStart			(void){}
void	PeriodicStop			(void){}
void	PeriodicIruptDisable	(void){}
void	PeriodicIruptEnable	(void){}
void	PeriodicModuleInit	(void){}
void	PeriodicIruptFlagClear	(void){}

void    PinModuleInit   (void){}
void	PinWrite		(PinT Pin, bool Value){}