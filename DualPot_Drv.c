/*H**********************************************************************
* FILENAME : DualPot_Drv.c
* DESCRIPTION : Dual channel digital potentiometer driver
* PUBLIC FUNCTIONS :
*           void DualPotDrv_Init(void)
*           bool DualPotDrv_Main(u8 channel ,f32 resistance)
*           void DualPotDrv_DeInit(void)
* NOTES : This application driver means to control
*         a dual-channel digital potentiometer (MAX5389, 10 kΩ model)
* AUTHOR : Sarika Natu         DATE : 22 Jun 2020
* CHANGES :
* VERSION   DATE      WHO     DETAIL
* 0.1.0   19Jun2020   SN      Dual pot driver implementation
* 0.2.0   21Jun2020   SN      Multichannel support
*H***********************************************************************/

/******************************************************************************/
//	includes
/******************************************************************************/
#include "DualPot_Drv.h"

/******************************************************************************
 *	variables
 ******************************************************************************/
typedef enum {
    Initial = 0,            /* Initial state */
    Setup1,                 /* Setup1 State */
    Setup2,                 /* Setup2 State */
    Running,                /* Running State */
    Stop                    /* Done State */
} Sig_states;

struct DigiPot{
    u8 channel:8;               /* Channel indication */
    u8 curr_Tap:8;              /* store current Wiper tap value */
    u8 tapVal:8;                /* store required output Wiper tap value*/
    bool cs:1;                  /* chip select input */
    bool updwn_ctrl:1;          /* up/down control input */
    bool MoveDownFlag:1;        /* move down notification */
    bool MoveUpFlag:1;          /* move up notification */
    Sig_states STATE;           /* state indication */
}channelA,channelB;

bool prevIncr;            /* variable to store previous value increment control input*/
bool incr_ctrl;           /* Wiper increment control input*/
bool updwn50usFlag;       /* Control 50us timer elapse*/

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

    if(TIMER_FREQ <= PeriodicFreqHzMax){    /* check if required rolling frequency is in range */
        /* Setting rolling frequency and assign interrupt handler*/
        PeriodicConfig(TIMER_FREQ, ISR_Timer25us_Handler);
    }

    /* Initialize tap value to 128 at power-up for Channel A and Channel B */
    channelA.curr_Tap = MID_TAP;
    channelB.curr_Tap = MID_TAP;

    /* Initialize move up and move down flags for Channel A and Channel B */
    channelA.MoveUpFlag = False;
    channelA.MoveDownFlag = False;
    channelB.MoveUpFlag = False;
    channelB.MoveDownFlag = False;

    /* Initialize signal state for Channel A and Channel B */
    channelA.STATE = Initial;
    channelB.STATE = Initial;

    /* Initialize 50us timer flag */
    updwn50usFlag = False;

    /* Initialize chip select for Channel A and Channel B; write to the respective registers */
    channelA.cs = True;
    channelB.cs = True;
    PinWrite(PinCSA, channelA.cs);
    PinWrite(PinCSB, channelB.cs);

    /* Initialize Up/Down control signal for Channel A and Channel B  */
    channelA.updwn_ctrl = False;
    channelB.updwn_ctrl = False;

    /* Initialize increment control signal */
    prevIncr = True;                     /* Initialize previous increment value */
    incr_ctrl = True;                    /* Initialize increment control variable */

    PeriodicIruptEnable();                /* Enabling interrupts */

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
    if((resistance >= MIN_RESISTANCE) && (resistance <= MAX_RESISTANCE) && ((channel == chA) || (channel == chB))){
        if(channel == chA){
            channelA.channel = chA;
            channelA.tapVal = getTap(resistance);   /* Get tap value for the provided resistance for channel A */
        }
        if (channel == chB){
            channelB.channel = chB;
            channelB.tapVal = getTap(resistance);   /* Get tap value for the provided resistance for channel B */
        }

        setWiper();                           /* Move towards desired wiper location if conditions are met */

        /* Check digital resistive value is nearer to LA or HA for channel A.
         * set move up or move down flag accordingly if channel A is requested */
        if (chA == channelA.channel){
            if(channelA.tapVal <= MID_TAP){
                channelA.MoveDownFlag = True;
            }else {
                if(channelA.tapVal > MID_TAP){
                    channelA.MoveUpFlag = True;
                }/*ELSE: Do nothing*/
            }
        }

        /* Check digital resistive value is nearer to LB or HB for channel B
         * set move up or move down flag accordingly if channel B is requested */
        if (chB == channelB.channel){
            if(channelB.tapVal <= MID_TAP){
                channelB.MoveDownFlag = True;
            }else {
                if(channelB.tapVal > MID_TAP){
                    channelB.MoveUpFlag = True;
                }/*ELSE: Do nothing*/
            }
        }


        generateSig();                            /* setting initial inputs to control wiper terminal(WA/WB) */

        /* if desired tap value is achieved return success else fail */
        if(channelA.curr_Tap == channelA.tapVal) {
            channelA.MoveDownFlag = False;          /* reset move up or move down flag for channel A */
            channelA.MoveUpFlag = False;
            channelA.STATE = Stop;                  /* change signal state to Stop for channel A */
        }
        if(channelB.curr_Tap == channelB.tapVal){

            channelB.MoveDownFlag = False;          /* reset move up or move down flag for channel B */
            channelB.MoveUpFlag = False;
            channelB.STATE = Stop;                  /* change signal state to Stop for channel B */
        }

        /* If no channel is running, stop the timer and return successful */
        if(((channelA.STATE == Stop) && (channelB.STATE == Initial)) ||
                    ((channelA.STATE == Initial) && (channelB.STATE == Stop)) ||
                    ((channelA.STATE == Stop) && (channelB.STATE == Stop))){
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

    //printf("Requested tap value for %d channel is %d!\n", channelA.channel, channelA.tapVal); /* for testing purpose */
    //printf("Requested tap value for %d channel is %d!\n", channelB.channel, channelB.tapVal); /* for testing purpose */

    /* Reset chip select for Channel A and Channel B by writing to the respective registers */
    PinWrite(PinCSA, True);
    PinWrite(PinCSB, True);

    /* Reset Up/Down control signal for Channel A and Channel B by writing to the respective registers */
    PinWrite(PinUDA, False);
    PinWrite(PinUDB, False);

    /* Reset Increment control signal for Channel A and Channel B by writing to the respective registers */
    PinWrite(PinINCA, True);
    PinWrite(PinINCB, True);
}

/********************************************************************
* FUNCTION   : u8 getTap(f32 resistance)
* PURPOSE    : Calculate tap value for desired resistance
* PARAMETERS : f32 resistance       //desired value of the resistance
* RETURN     : u8
**********************************************************************/
u8 getTap(f32 resistance) {

    /* return tap value for the provided  input resistance */
    return (u8)(resistance/MAX_RESISTANCE * FULL_TAP);
}

/********************************************************************
* FUNCTION   : static void setWiper(void)
* PURPOSE    : Calculate tap value for desired resistance
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
static void setWiper(void){

    /* Check if the signal state is Running for Channel A */
    if(Running == channelA.STATE){

        /* Check for falling edge on increment control signal for Channel A */
        if((prevIncr == True) && (incr_ctrl == False)){

            //printf("Current tap value for %d channel is %d!\n", channelA.channel, channelA.curr_Tap); /* for testing purpose */

            /* Check if the move down flag is true and if wiper terminal has not reached its minimum value*/
            if((channelA.MoveDownFlag == True) && (channelA.curr_Tap != MIN_TAP)){

                /* decrementing the tap value by one position
                * as wiper terminal should be moved one tap location towards low terminal */
                channelA.curr_Tap--;

            }else {
                /* Check if the move up flag is true and if wiper terminal has not reached its maximum value*/
                if((channelA.MoveUpFlag == True) && (channelA.curr_Tap != FULL_TAP)){

                    /* incrementing the tap value by one position
                    * as wiper terminal should be moved one tap location towards high terminal */
                    channelA.curr_Tap++;
                }/*ELSE: Do nothing*/
            }
        }
    }

    /* Check if the signal state is Running for Channel A */
    if(Running == channelB.STATE){

        /* Check for falling edge on increment control signal for Channel B */
        if((prevIncr == True) && (incr_ctrl == False)){

            //printf("Current tap value for %d channel is %d!\n", channelB.channel, channelB.curr_Tap); /* for testing purpose */

            /* Check if the move down flag is true and if wiper terminal has not reached its minimum value*/
            if((channelB.MoveDownFlag == True) && (channelB.curr_Tap != MIN_TAP)){

                /* decrementing the tap value by one position
                * as wiper terminal should be moved one tap location towards low terminal */
                channelB.curr_Tap--;

            }else {
                /* Check if the move up flag is true and if wiper terminal has not reached its maximum value*/
                if((channelB.MoveUpFlag == True)&& (channelB.curr_Tap != FULL_TAP)){

                    /* incrementing the tap value by one position
                    * as wiper terminal should be moved one tap location towards high terminal */
                    channelB.curr_Tap++;
                }/*ELSE: Do nothing*/
            }
        }
    }
    prevIncr = incr_ctrl;               /* storing current increment control value */
}

/********************************************************************
* FUNCTION   : void generateSig(void)
* PURPOSE    : Calculate tap value for desired resistance
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
static void generateSig(void) {

    /* timer start flag to avoid PeriodicStart() function call more than once */
    static bool timer_start = False;

    /* Check if channel A is in initial signal state */
    if((chA == channelA.channel) && (Initial == channelA.STATE)){

        /* Update Up/Down flag according to Move up and Move down flags */
        if(channelA.MoveDownFlag == True){
            channelA.updwn_ctrl = False;                    /* setting Up/Down control signal as down */
        } else {
            if(channelA.MoveUpFlag == True){
                channelA.updwn_ctrl = True;                 /* setting Up/Down control signal as Up */
            }/*ELSE: Do nothing*/
        }

        incr_ctrl = True;                                   /* setting increment control signal to high for channel A */
        PinWrite(PinUDA, channelA.updwn_ctrl);
        PinWrite(PinINCA, incr_ctrl);
        channelA.STATE = Setup1;                            /* change signal state to Setup1 for channel A */

    }

    /* Check if channel B is in initial signal state */
    if((chB == channelB.channel) && (Initial == channelB.STATE)){

        /* Update Up/Down control signal according to Move up and Move down flags */
        if(channelB.MoveDownFlag == True){
            channelB.updwn_ctrl = False;                     /* setting Up/Down control signal as down */
        } else {
            if(channelB.MoveUpFlag == True){
                channelB.updwn_ctrl = True;                  /* setting Up/Down control signal as Up */
            }/*ELSE: Do nothing*/
        }

        incr_ctrl = True;                                   /* setting increment control signal to high for channel B */
        PinWrite(PinUDB, channelB.updwn_ctrl);
        PinWrite(PinINCB, incr_ctrl);
        channelB.STATE = Setup1;                            /* change signal state to Setup1 for channel B */
    }

    /* Start the timer if timer is not already running and if signal state of either channel is Setup1 */
    if((channelA.STATE == Setup1) && (timer_start == False)){
        PeriodicStart();
        timer_start = True;
    }else{
        if((channelB.STATE == Setup1) && (timer_start == False)){
            PeriodicStart();
            timer_start = True;
        }
    }

}

/********************************************************************
* FUNCTION   : void ISR_Timer25us_Handler(void)
* PURPOSE    : ISR
* PARAMETERS : void
* RETURN     : void
**********************************************************************/
void ISR_Timer25us_Handler(void){

    /* Check if either of channel A or channel B is active */
    if((chA == channelA.channel) || (chB == channelB.channel)){

        /* If ISR is hit for the first time UpDown control signal will not be set as it needs 50us time*/
        if(updwn50usFlag == True){

            updwn50usFlag = False;                      /* reset 50us timer flag for 50us completion*/

            /* Check if channel A is in Setup1 signal state */
            if((chA == channelA.channel) && (Setup1 == channelA.STATE)){

                /* Update Up/Down control signal according to Move up and Move down flags */
                if(channelA.MoveDownFlag == True){
                    channelA.updwn_ctrl = False;            /* Clear Up/Down control signal */
                }else{
                    if(channelA.MoveUpFlag == True) {
                        channelA.updwn_ctrl = True;         /* Set Up/Down control signal */
                    }/*ELSE: Do nothing*/
                }
                PinWrite(PinUDA, channelA.updwn_ctrl);
                channelA.STATE = Setup2;                    /* change signal state to Setup2 for channel A */

            }

            /* Check if channel B is in Setup1 signal state */
            if((chB == channelB.channel) && (Setup1 == channelB.STATE)){

                /* Update Up/Down control signal according to Move up and Move down flags */
                if(channelB.MoveDownFlag == True){
                    channelB.updwn_ctrl = False;             /* Clear Up/Down control signal */
                }else {
                    if(channelB.MoveUpFlag == True){
                        channelB.updwn_ctrl = True;          /* Set Up/Down control signal */
                    }/*ELSE: Do nothing*/
                }
                PinWrite(PinUDB, channelB.updwn_ctrl);
                channelB.STATE = Setup2;                    /* change signal state to Setup2 for channel B */
            }
        }else{

            /* setting chip select of respective channel to low if signal state is Setup1 */
            if((chA == channelA.channel)&& (Setup1 == channelA.STATE)){
                channelA.cs = False;
                PinWrite(PinCSA, channelA.cs);
            }
            if((chB == channelB.channel)&& (Setup1 == channelB.STATE)){
                channelB.cs = False;
                PinWrite(PinCSB, channelB.cs);
            }

            updwn50usFlag = True;                           /* setting 50us flag for 25us completion*/
        }

        /* Inverting the Increment control signal every 25us if channels are in Setup2/Running signal state */
        if((channelA.STATE == Setup2) || (channelB.STATE == Setup2) ||
                    (channelA.STATE == Running) || (channelB.STATE == Running)){
            incr_ctrl ^= True;

            /* Store increment control value if its TRUE for detecting falling edge */
            if(True == incr_ctrl){
                prevIncr = True;
            }

            /* Writing increment control signal values to respective pins*/
            if((chA == channelA.channel) && (channelA.STATE != Stop)){
                PinWrite(PinINCA, incr_ctrl);
                channelA.STATE = Running;
            }
            if((chB == channelB.channel) && (channelB.STATE != Stop)){
                PinWrite(PinINCB, incr_ctrl);
                channelB.STATE = Running;
            }/*ELSE: Do nothing*/
        }
    }

    PeriodicIruptFlagClear();                       /* Clear interrupt flag */
}


