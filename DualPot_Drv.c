/*H**********************************************************************
* FILENAME : DualPot_Drv.c
* DESCRIPTION : Dual channel digital potentiometer driver
* PUBLIC FUNCTIONS :
*           void DualPotDrv_Init(void)
*           bool DualPotDrv_Main(u8 channel ,f32 resistance)
*           void DualPotDrv_DeInit(void)
* NOTES : This application driver means to control
*         a dual-channel digital potentiometer (MAX5389, 10 kΩ model)
* AUTHOR : Sarika Natu         DATE : 19 Jun 2020
* CHANGES :
* VERSION   DATE      WHO     DETAIL
* 0.1.0   19Jun2020   SN      Dual pot driver implementation
*H***********************************************************************/

/******************************************************************************/
//	includes
/******************************************************************************/
#include "DualPot_Drv.h"

/******************************************************************************
 *	variables
 ******************************************************************************/
struct DigiPot{
    u8 channel:8;
    u8 curr_Tap:8;       /* variable to store current Wiper tap value*/
    u8 tapVal:8;
    bool cs:1;                  /* Wiper chip select input*/
    bool updwn_ctrl:1;          /* Wiper up/down control input*/
    bool MoveDownFlag:1;
    bool MoveUpFlag:1;
}channelA,channelB;

bool prevIncr;            /* variable to store previous value increment control input*/
bool incr_ctrl;           /* Wiper increment control input*/
bool updwn50usFlag;

/******************************************************************************
 *	local functions
 ******************************************************************************/
static u8 getTap(f32 resistance);
static void setWiper(void);
static void generateSig(void);

void ISR_Timer25us_Handler(void);

/********************************************************************
* FUNCTION   : void DualPotDrv_Init(void)
* PURPOSE    : Initialize DualPot Driver
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
void DualPotDrv_Init(void){
    PeriodicModuleInit();       /* Initialize periodic module */
    PeriodicIruptDisable();     /* Disabling interrupts */

    PinModuleInit();            /* Initialize pin module */

    if(TIMER_FREQ <= PeriodicFreqHzMax){
        /* Setting UP-DOWN rolling frequency and assign UP-DOWN interrupt handler*/
        PeriodicConfig(TIMER_FREQ, ISR_Timer25us_Handler);
    }

    channelA.curr_Tap = MID_TAP;      /* Initialize tap value to 128 at power-up */
    channelB.curr_Tap = MID_TAP;      /* Initialize tap value to 128 at power-up */

    channelA.channel = chNone;         /* Initialize channel A flag */
    channelB.channel = chNone;         /* Initialize channel B flag */

    channelA.MoveUpFlag = False;
    channelA.MoveDownFlag = False;
    channelB.MoveUpFlag = False;
    channelB.MoveDownFlag = False;

    updwn50usFlag = False;

    channelA.cs = True;                 /* Initialize chip select */
    channelB.cs = True;
    PinWrite(PinCSA, channelA.cs);  /* Write chip select value to channel A */
    PinWrite(PinCSB, channelB.cs);  /* Write chip select value to channel B */

    channelA.updwn_ctrl = False;         /* Initialize UP/DWN control variables */
    channelB.updwn_ctrl = False;
    PinWrite(PinUDA, channelA.updwn_ctrl);   /* Write UP/DWN control value to channel A */
    PinWrite(PinUDB, channelB.updwn_ctrl);   /* Write UP/DWN control value to channel B */

    prevIncr = True;                     /* Initialize previous increment value */
    incr_ctrl = True;                    /* Initialize increment control variable */
    PinWrite(PinINCA, True);         /* Write INC control value to channel A */
    PinWrite(PinINCB, True);         /* Write INC control value to channel B */

    PeriodicIruptEnable();                /* Enabling interrupts */
    PeriodicStart();                      /* start timer */

}

/********************************************************************
* FUNCTION   : bool DualPotDrv_Main(u8 channel,f32 resistance)
* PURPOSE    : Main function of Dual Pot driver
* PARAMETERS : u8 channel           //channel for resistance setting
*              f32 resistance       //desired value of the resistance
* RETURN     : bool
**********************************************************************/
bool DualPotDrv_Main(u8 channel,f32 resistance) {

    bool retVal = False;                    /* return value */

    /* Checking if resistance and requested channel is in range */
    if((resistance <= MAX_RESISTANCE) && ((channel == chA) || (channel == chB))){
        if(channel == chA){
            channelA.channel = chA;
            channelA.tapVal = getTap(resistance);   /* Get tap value for the provided resistance */
        }
        if (channel == chB){
            channelB.channel = chB;
            channelB.tapVal = getTap(resistance);   /* Get tap value for the provided resistance */
        }

        setWiper();                           /* Move towards desired wiper location */

        if(channelA.tapVal <= MID_TAP){       /* Check digital resistive value is less than LA */
            channelA.MoveDownFlag = True;
        }else {
            if(channelA.tapVal > MID_TAP){     /* Check digital resistive value nearer to HA */
                channelA.MoveUpFlag = True;
            }/*ELSE: Do nothing*/
        }

        if(channelB.tapVal <= MID_TAP){         /* Check digital resistive value is less than LB */
            channelB.MoveDownFlag = True;
        }else {
            if(channelB.tapVal > MID_TAP){       /* Check digital resistive value nearer to HB */
                channelB.MoveUpFlag = True;
            }/*ELSE: Do nothing*/
        }

        generateSig();                            /* setting initial inputs to control wiper terminal(WA/WB) */

        if(channelA.curr_Tap == channelA.tapVal) {    /* if desired tap value is achieved return success else fail */
            channelA.MoveDownFlag = False;
            channelA.MoveUpFlag = False;
            channelA.channel = chNone;
        }
        if(channelB.curr_Tap == channelB.tapVal){

            channelB.MoveDownFlag = False;
            channelB.MoveUpFlag = False;
            channelB.channel = chNone;
        }
        if((channelA.channel == chNone) && (channelB.channel == chNone)){
            PeriodicStop();                         /* timer stop */
            retVal = True;
        }
    } else{
        retVal = False;
    }
    return retVal;
}

/********************************************************************
* FUNCTION   : void DualPotDrv_DeInit(void)
* PURPOSE    : De-initialize DualPot Driver
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
void DualPotDrv_DeInit(void){

    PinWrite(PinCSA, True);             /* setting chip select of channel B to high */
    PinWrite(PinCSB, True);             /* setting chip select of channel B to high */
    PinWrite(PinUDA, False);            /* setting UP/DWN control signal of channel A to low*/
    PinWrite(PinUDB, False);            /* setting UP/DWN control signal of channel B to low*/
    PinWrite(PinINCA, True);            /* setting Increment control signal of channel A to low*/
    PinWrite(PinINCB, True);            /* setting Increment control signal of channel B to low*/
}

/********************************************************************
* FUNCTION   : u8 getTap(f32 resistance)
* PURPOSE    : Calculate tap value for desired resistance
* PARAMETERS : f32 resistance       //desired value of the resistance
* RETURN     : u8
**********************************************************************/
u8 getTap(f32 resistance) {
    return (u8)(resistance/MAX_RESISTANCE * FULL_TAP);  /* return tap value for the provided resistance */
}

/********************************************************************
* FUNCTION   : static void setWiper(void)
* PURPOSE    : Calculate tap value for desired resistance
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
static void setWiper(void){
    if(False == channelA.cs){                            /* check if chip select signal is low */
        if((prevIncr == True) && (incr_ctrl == False)){  /* check for falling edge of increment control signal */
            if((channelA.MoveDownFlag == True) && (channelA.curr_Tap != MIN_TAP)){
                /* decrementing the tap value by one position
                * as wiper terminal should be moved one tap location towards low terminal */
                channelA.curr_Tap--;

            }else {
                if((channelA.MoveUpFlag == True) && (channelA.curr_Tap != FULL_TAP)){
                    /* incrementing the tap value by one position
                    * as wiper terminal should be moved one tap location towards high terminal */
                    channelA.curr_Tap++;
                }/*ELSE: Do nothing*/
            }
            prevIncr = incr_ctrl;               /* storing current INC value */
        }
    }
    if(False == channelB.cs){                              /* check if chip select signal is low */
        if((prevIncr == True) && (incr_ctrl == False)){   /* check for falling edge of increment control signal */
            if((channelB.MoveDownFlag == True) && (channelB.curr_Tap == MIN_TAP)){
                /* decrementing the tap value by one position
                * as wiper terminal should be moved one tap location towards low terminal */
                channelB.curr_Tap--;

            }else {
                if((channelB.MoveUpFlag == True)&& (channelB.curr_Tap == FULL_TAP)){
                    /* incrementing the tap value by one position
                    * as wiper terminal should be moved one tap location towards high terminal */
                    channelB.curr_Tap++;
                }/*ELSE: Do nothing*/
            }
            prevIncr = incr_ctrl;               /* storing current INC value */
        }
    }
}

/********************************************************************
* FUNCTION   : void generateSig(void)
* PURPOSE    : Calculate tap value for desired resistance
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
static void generateSig(void) {

    if(chA == channelA.channel){     /* check for valid channel is selected */
        if(channelA.MoveDownFlag == True){
            channelA.updwn_ctrl = False;                          /* setting UP/DWN control signal as down */
        } else {
            if(channelA.MoveUpFlag == True){
                channelA.updwn_ctrl = True;                          /* setting UP/DWN control signal as Up */
            }/*ELSE: Do nothing*/
        }
        incr_ctrl = True;                           /* setting increment control signal to high */
        channelA.cs = False;                                 /* setting chip select of channel to low  - enabling */
        PinWrite(PinCSA, channelA.cs);
        PinWrite(PinUDA, channelA.updwn_ctrl);
        PinWrite(PinINCA, incr_ctrl);
        }

    if(chB == channelB.channel){
        if(channelB.MoveDownFlag == True){
            channelB.updwn_ctrl = False;                          /* setting UP/DWN control signal as down */
        } else {
            if(channelB.MoveUpFlag == True){
                channelB.updwn_ctrl = True;                          /* setting UP/DWN control signal as Up */
            }/*ELSE: Do nothing*/
        }
        incr_ctrl = True;                           /* setting increment control signal to high */
        channelB.cs = False;                                 /* setting chip select of channel to low  - enabling */
        PinWrite(PinCSB, channelB.cs);
        PinWrite(PinUDB, channelB.updwn_ctrl);
        PinWrite(PinINCB, incr_ctrl);
        }
}

/********************************************************************
* FUNCTION   : void ISR_Timer25us_Handler(void)
* PURPOSE    : ISR
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
void ISR_Timer25us_Handler(void){

    if((chA == channelA.channel) || (chB == channelB.channel)){

        /* UP/Down logic*/
        if(updwn50usFlag == True){
            if(chA == channelA.channel){
                updwn50usFlag = False;
                if(channelA.MoveDownFlag == True){
                    channelA.updwn_ctrl = False;                     /* Clear UP/DWN control signal */
                }else{
                    if(channelA.MoveUpFlag == True) {
                        channelA.updwn_ctrl = True;                     /* Set UP/DWN control signal */
                    }/*ELSE: Do nothing*/
                }
                PinWrite(PinUDA, channelA.updwn_ctrl);

            }
            if(chB == channelB.channel){
                updwn50usFlag = False;
                if(channelB.MoveDownFlag == True){
                    channelB.updwn_ctrl = False;                     /* Clear UP/DWN control signal */
                }else {
                    if(channelB.MoveUpFlag == True){
                        channelB.updwn_ctrl = True;                     /* Set UP/DWN control signal */
                    }/*ELSE: Do nothing*/
                }
                PinWrite(PinUDB, channelB.updwn_ctrl);
            }
        }else{
            updwn50usFlag = True;
        }

        /* Inc logic*/
        incr_ctrl = !incr_ctrl;                     /* Invert INC control signal */
        if(True == incr_ctrl){
            prevIncr = True;                        /* Store INC value if its TRUE for detect falling edge */
        }

        /* Writing to respective pins*/
        if(chA == channelA.channel){
            PinWrite(PinUDA, incr_ctrl);
        }else {
            if(chB == channelB.channel){
                PinWrite(PinUDB, incr_ctrl);
            }/*ELSE: Do nothing*/
        }
    }

    PeriodicIruptFlagClear();                       /* Clear interrupt flag */
}


