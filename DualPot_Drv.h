/******************************************************************************/
//	DualPot_Drv.h
/******************************************************************************/

#ifndef MOTIV_DUALPOT_DUALPOT_DRV_H
#define MOTIV_DUALPOT_DUALPOT_DRV_H

/******************************************************************************/
//	includes
/******************************************************************************/
#include "Pin.h"
#include "Generic.h"
#include "Periodic.h"

/******************************************************************************/
//	variables
/******************************************************************************/
static u8 curr_Tap;       /* variable to store current Wiper tap value*/
u8 tapVal;
bool cs;                  /* Wiper chip select input*/
bool updwn_ctrl;          /* Wiper up/down control input*/
bool incr_ctrl;           /* Wiper increment control input*/
bool prevIncr;            /* variable to store previous value increment control input*/
bool chAflag;             /* flag for channel A selection */
bool chBflag;             /* flag for channel B selection */
bool MoveDownFlag;
bool MoveUpFlag;
bool updwn50usFlag;

/******************************************************************************/
//	macros
/******************************************************************************/
#define chA 1
#define chB 2

#define MAX_RESISTANCE ((f32)10000)
#define FULL_TAP 255
#define MID_TAP 128
#define MIN_TAP 0

#define TIMER_FREQ ((f32)40000)     /* Rollover frequency to attain 25us signal*/


/******************************************************************************/
//	service functions
/******************************************************************************/
void DualPotDrv_Init(void);
bool DualPotDrv_Main(u8 channel ,f32 resistance);
void DualPotDrv_DeInit(void);

u8 getTap(f32 resistance);
void setWiper(void);
void generateSig(u8 channel);

void ISR_Timer25us_Handler(void);

#endif //MOTIV_DUALPOT_DUALPOT_DRV_H
