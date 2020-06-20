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

    if(TIMER_FREQ <= PeriodicFreqHzMax){
        /* Setting UP-DOWN rolling frequency and assign UP-DOWN interrupt handler*/
        PeriodicConfig(TIMER_FREQ, ISR_Timer25us_Handler);
    }

    curr_Tap = MID_TAP;                     /* Initialize tap value to 128 at power-up */
    chAflag = False;                        /* Initialize channel A flag */
    chBflag = False;                        /* Initialize channel B flag */
    prevIncr = True;                        /* Initialize previous increment value */
    MoveUpFlag = False;
    MoveDownFlag = False;
    updwn50usFlag = False;

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
    PeriodicStart();                        /* start timer */

}

/********************************************************************
* FUNCTION   : bool DualPotDrv_Main(u8 channel,f32 resistance)
* PURPOSE    : Main function of Dual Pot driver
* PARAMETERS : u8 channel           //channel for resistance setting
*              f32 resistance       //desired value of the resistance
* RETURN     : bool
**********************************************************************/
bool DualPotDrv_Main(u8 channel,f32 resistance) {
    if((resistance <= MAX_RESISTANCE) && ((channel == chA) || (channel == chB))){   /* Checking if resistance and channel is in range */
        tapVal = getTap(resistance);                /* Get tap value for the provided resistance */

        setWiper();

        if(tapVal <= MID_TAP){                      /* Check digital resistive value is less than LA/LB */
                MoveDownFlag = True;
        }else if(tapVal > MID_TAP){                 /* Check digital resistive value nearer to HA/HB */
                MoveUpFlag = True;
        }else{}

        generateSig(channel);                       /* setting inputs to control wiper terminal(WA/WB) */
        if(curr_Tap == tapVal){                     /* if desired tap value is achieved return success else fail */
            MoveDownFlag = False;
            MoveUpFlag = False;
            PeriodicStop();                         /* timer stop */
            return True;
        }
    } else{
        return False;
    }


    return False;


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
void setWiper(void){
    if(False == updwn_ctrl){                          /* check if UP/DWN control signal is low */
        if(False == cs){                              /* check if chip select signal is low */
            if((prevIncr == True) && (incr_ctrl == False)){   /* check for falling edge of increment control signal */
                if(MoveDownFlag == True){
                    /* decrementing the tap value by one position
                    * as wiper terminal should be moved one tap location towards low terminal */
                    curr_Tap--;

                }else if(MoveUpFlag == True){
                    /* incrementing the tap value by one position
                    * as wiper terminal should be moved one tap location towards high terminal */
                    curr_Tap++;
                }else{
                }
                prevIncr = incr_ctrl;               /* storing current INC value */
            }
        }
    }
}

/********************************************************************
* FUNCTION   : void generateSig(u8 channel)
* PURPOSE    : Calculate tap value for desired resistance
* PARAMETERS : u8 channel       //channel for resistance setting
* RETURN     : void
**********************************************************************/
void generateSig(u8 channel) {

    if(channel == chA){                     /* Allocate pins for CS, UD, INC; if channel A is requested */
        chAflag = True;                    /* set flag for channel A */
    }else if(channel == chB){              /* Allocate pins for CS, UD, INC; if channel B is requested */
        chBflag = True;                     /* set flag for channel B */
    }else{
        /* do nothing */
    }

    if((True == chAflag) || (True == chBflag)){     /* check for valid channel is selected */
        cs = False;                                 /* setting chip select of channel to low  - enabling */
        if(MoveDownFlag == True){
            updwn_ctrl = False;                          /* setting UP/DWN control signal as down */
        } else if(MoveUpFlag == True){
            updwn_ctrl = True;                          /* setting UP/DWN control signal as down */
        } else{
        }
        incr_ctrl = True;                           /* setting increment control signal to high */
        if(True == chAflag){
            PinWrite(PinCSA, cs);
            PinWrite(PinUDA, updwn_ctrl);
            PinWrite(PinINCA, incr_ctrl);
        }else if(True == chBflag){
            PinWrite(PinCSB, cs);
            PinWrite(PinUDB, updwn_ctrl);
            PinWrite(PinINCB, incr_ctrl);
        }

    }else{

    }

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
* FUNCTION   : void ISR_INCEDGE_Handler(void)
* PURPOSE    : ISR
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
void ISR_Timer25us_Handler(void){
    //PeriodicStop();

    if(((True == chAflag) || (True == chBflag)) && (curr_Tap != tapVal)){

        /* UP/Down logic*/
        if(updwn50usFlag == True){

            updwn50usFlag = False;
            if(MoveDownFlag == True){
                updwn_ctrl = False;                     /* Clear UP/DWN control signal */
            }else if(MoveUpFlag == True){
                updwn_ctrl = True;                     /* Set UP/DWN control signal */
            } else{
            }

            if(True == chAflag){
                PinWrite(PinUDA, updwn_ctrl);
            }else if(True == chBflag){
                PinWrite(PinUDB, updwn_ctrl);
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
        if(True == chAflag){
            PinWrite(PinUDA, incr_ctrl);
        }else if(True == chBflag){
            PinWrite(PinUDB, incr_ctrl);
        }
    }

    PeriodicIruptFlagClear();                       /* Clear interrupt flag */
}


