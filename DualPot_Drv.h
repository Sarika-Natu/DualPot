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
bool cs;                  /* Wiper chip select input*/
bool updwn_ctrl;          /* Wiper up/down control input*/
bool incr_ctrl;           /* Wiper increment control input*/
bool prevIncr;            /* variable to store previous value increment control input*/
bool chAflag;             /* flag for channel A selection */
bool chBflag;             /* flag for channel B selection */

/******************************************************************************/
//	macros
/******************************************************************************/
#define chA 1
#define chB 2

#define MAX_RESISTANCE ((f32)10000)
#define FULL_TAP 255
#define MID_TAP 128

#define UD_FREQ ((f32)20000)      /* Rollover frequency to attain 50ms UP/DWN signal*/
#define INC_FREQ ((f32)40000)     /* Rollover frequency to attain 25ms INC signal*/


/******************************************************************************/
//	service functions
/******************************************************************************/
void DualPotDrv_Init(void);
bool DualPotDrv_Main(u8 channel ,f32 resistance);
void DualPotDrv_DeInit(void);

u8 getTap(f32 resistance);
void setWiper(u8 channel);

void * ISR_UPDWN_Handler(void);
void * ISR_INCEDGE_Handler(void);

#endif //MOTIV_DUALPOT_DUALPOT_DRV_H
