#include <stdio.h>
#include "DualPot_Drv.h"

int main() {
    bool result = False;
    bool result1 = False;
    int counter = 0;

    printf("\nDual digi driver Init is called!\n");
    DualPotDrv_Init();


    for (counter = 0; counter < 150; counter++)
    {
        printf("\nDual digi driver main function is called %d!\n", counter);
        ISR_Timer25us_Handler();

        result1 = DualPotDrv_Main(2, 7000);
        result = DualPotDrv_Main(1, 3000);

        if ((result == True) && (result1 == True)) {
            counter = 150;
        }
    }

    printf("\nDual digi driver De-Init is called!\n");
    DualPotDrv_DeInit();

    printf("\n****The result is %d****\n\n", result);
    return 0;
}
