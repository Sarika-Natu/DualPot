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

/******************************************************************************/
//	variables
/******************************************************************************/

/******************************************************************************/
//	macros
/******************************************************************************/
#define chA 1U
#define chB 2U

#define FULL_TAP 255U
#define MID_TAP 128U
#define MIN_TAP 0U

#define MIN_RESISTANCE ((f32)0)
#define MAX_RESISTANCE ((f32)10000)
#define TIMER_FREQ ((f32)40000)     /* Rollover frequency to attain 25us signal*/


/******************************************************************************/
//	service functions
/******************************************************************************/
void DualPotDrv_Init(void);
bool DualPotDrv_Main(u8 channel ,f32 resistance);
void DualPotDrv_DeInit(void);

#endif //MOTIV_DUALPOT_DUALPOT_DRV_H
