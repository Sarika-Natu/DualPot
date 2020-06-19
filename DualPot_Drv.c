/*H**********************************************************************
* FILENAME : DualPot_Drv.c                                          //TBD
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
#include <stdio.h>
#include "DualPot_Drv.h"

/********************************************************************
* FUNCTION   : void DualPotDrv_Init(void)
* PURPOSE    : Initialize DualPot Driver
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
void DualPotDrv_Init(void){
    PeriodicModuleInit();                   /* Initialize periodic module */
    PeriodicIruptDisable();                 /* Disabling interrupts */

    PinModuleInit();                        /* Initialize pin module */

    if(UD_FREQ <= PeriodicFreqHzMax){
        /* Setting UP-DOWN rolling frequency and assign UP-DOWN interrupt handler*/
        PeriodicConfig(UD_FREQ, ISR_UPDWN_Handler());
    }
    if(INC_FREQ <= PeriodicFreqHzMax){
        /* Setting increment rolling frequency and assign INC interrupt handler*/
        PeriodicConfig(INC_FREQ, ISR_INCEDGE_Handler());
    }

    curr_Tap = MID_TAP;                     /* Initialize tap value to 128 at power-up */
    chAflag = False;                        /* Initialize channel A flag */
    chBflag = False;                        /* Initialize channel B flag */
    prevIncr = True;                        /* Initialize previous increment value */

    cs = True;                              /* Initialize chip select */
    PinWrite(PinCSA, True);             /* Write chip select value to channel A */
    PinWrite(PinCSB, True);             /* Write chip select value to channel B */

    updwn_ctrl = False;                       /* Initialize UP/DWN control variable */
    PinWrite(PinUDA, False);            /* Write UP/DWN control value to channel A */
    PinWrite(PinUDB, False);            /* Write UP/DWN control value to channel B */

    incr_ctrl = True;                        /* Initialize increment control variable */
    PinWrite(PinINCA, True);            /* Write INC control value to channel A */
    PinWrite(PinINCB, True);            /* Write INC control value to channel B */

    PeriodicIruptEnable();                  /* Enabling interrupts */

}

/********************************************************************
* FUNCTION   : bool DualPotDrv_Main(u8 channel,f32 resistance)
* PURPOSE    : Main function of Dual Pot driver
* PARAMETERS : u8 channel           //channel for resistance setting
*              f32 resistance       //desired value of the resistance
* RETURN     : bool
**********************************************************************/
bool DualPotDrv_Main(u8 channel,f32 resistance) {
    u8 tapVal;

    tapVal = getTap(resistance);            /* Get tap value for the provided resistance */

    while (curr_Tap >= tapVal) {            /* Run while loop till desired tap value is not attained */
        setWiper(channel);                  /* setting inputs to control wiper terminal(WA/WB) */
    }

    if(curr_Tap == tapVal){                 /* if desired tap value is achieved return success else fail */
        return True;
    } else{
        return False;
    }


}
/********************************************************************
* FUNCTION   : u8 getTap(f32 resistance)
* PURPOSE    : Calculate tap value for desired resistance
* PARAMETERS : f32 resistance       //desired value of the resistance
* RETURN     : u8
**********************************************************************/
u8 getTap(f32 resistance) {
    f32 r = resistance/MAX_RESISTANCE;
    return (u8)(r * FULL_TAP);      /* return tap value for the provided resistance */
}

/********************************************************************
* FUNCTION   : void setWiper(u8 channel)
* PURPOSE    : Calculate tap value for desired resistance
* PARAMETERS : u8 channel       //channel for resistance setting
* RETURN     : void
**********************************************************************/
void setWiper(u8 channel) {
    PinT Pin_updwn;
    PinT Pin_cs;
    PinT Pin_Incr;

    if(channel == chA){                     /* Allocate pins for CS, UD, INC; if channel A is requested */
        Pin_cs = PinCSA;
        Pin_updwn = PinUDA;
        Pin_Incr = PinINCA;
        chAflag = True;                    /* set flag for channel A */
    }else if(channel == chB){              /* Allocate pins for CS, UD, INC; if channel B is requested */
        Pin_cs = PinCSB;
        Pin_updwn = PinUDB;
        Pin_Incr = PinINCB;
        chBflag = True;                     /* set flag for channel B */
    }else{
        /* do nothing */
    }

    if((True == chAflag) || (True == chBflag)){     /* check for valid channel is selected */
        cs = False;                                 /* setting chip select of channel to low  - enabling */
        PinWrite(Pin_cs, cs);
        updwn_ctrl = True;                          /* setting UP/DWN control signal as down */
        PinWrite(Pin_updwn, updwn_ctrl);
        incr_ctrl = True;                           /* setting increment control signal to high */
        PinWrite(Pin_Incr, incr_ctrl);
        PeriodicStart();                            /* start timer */
    }

    if(False == updwn_ctrl){                          /* check if UP/DWN control signal is low */
        if(False == cs){                              /* check if chip select signal is low */
            if((prevIncr == True) && (incr_ctrl == False)){   /* check for falling edge of increment control signal */
                /* decrementing the tap value by one position
                 * as wiper terminal should be moved one tap location towards low terminal */
                curr_Tap--;
                prevIncr = incr_ctrl;               /* storing current INC value */
            }
        }
    }

}

/********************************************************************
* FUNCTION   : void DualPotDrv_DeInit(void)
* PURPOSE    : De-initialize DualPot Driver
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
void DualPotDrv_DeInit(void){
    PeriodicStop();                         /* timer stop */
    PinWrite(PinCSA, True);             /* setting chip select of channel B to high */
    PinWrite(PinCSB, True);             /* setting chip select of channel B to high */
    PinWrite(PinUDA, False);            /* setting UP/DWN control signal of channel A to low*/
    PinWrite(PinUDB, False);            /* setting UP/DWN control signal of channel B to low*/
    PinWrite(PinINCA, True);            /* setting Increment control signal of channel A to low*/
    PinWrite(PinINCB, True);            /* setting Increment control signal of channel B to low*/
}

/********************************************************************
* FUNCTION   : void * ISR_UPDWN_Handler(void)
* PURPOSE    : ISR
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
void * ISR_UPDWN_Handler(void){
    //PeriodicStop();
    PeriodicIruptFlagClear();
    updwn_ctrl = False;
    if(True == chAflag){
        PinWrite(PinUDA, updwn_ctrl);
    }else if(True == chBflag){
        PinWrite(PinUDB, updwn_ctrl);
    }

    return NULL;
}

/********************************************************************
* FUNCTION   : void * ISR_INCEDGE_Handler(void)
* PURPOSE    : ISR
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
void * ISR_INCEDGE_Handler(void){
    //PeriodicStop();
    PeriodicIruptFlagClear();
    incr_ctrl = !incr_ctrl;
    if(True == incr_ctrl){
        prevIncr = True;
    }
    if(True == chAflag){
        PinWrite(PinUDA, incr_ctrl);
    }else if(True == chBflag){
        PinWrite(PinUDB, incr_ctrl);
    }

    return NULL;
}


