/******************************************************************************/
//	DualPot_Drv.h
/******************************************************************************/

#ifndef MOTIV_DUALPOT_DRV_H
#define MOTIV_DUALPOT_DRV_H

/******************************************************************************/
//	includes
/******************************************************************************/
#include "Pin.h"
#include "Generic.h"
#include "Periodic.h"
#include <stdio.h>            /* for testing purpose */

/******************************************************************************/
//	variables
/******************************************************************************/

/******************************************************************************/
//	macros
/******************************************************************************/
#define chA 1U                      /* Channel A notation*/
#define chB 2U                      /* Channel B notation*/

#define FULL_TAP 255U               /* Value for max digital output resistance*/
#define MID_TAP 128U                /* Value for mid digital output resistance*/
#define MIN_TAP 0U                  /* Value for min digital output resistance*/

#define MAX_RESISTANCE ((f32)10000) /* Value for max input resistance*/
#define MIN_RESISTANCE ((f32)0)     /* Value for min input resistance*/
#define TIMER_FREQ ((f32)40000)     /* Rollover frequency to attain 25us signal*/


/******************************************************************************/
//	service functions
/******************************************************************************/
void DualPotDrv_Init(void);
bool DualPotDrv_Main(u8 channel ,f32 resistance);
void DualPotDrv_DeInit(void);
void ISR_Timer25us_Handler(void);         /* for testing purpose */
#endif //MOTIV_DUALPOT_DUALPOT_DRV_H
