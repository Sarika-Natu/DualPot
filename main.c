#include <stdio.h>
#include "DualPot_Drv.h"

int main() {
    bool result = False;
    printf("Hello, World!\n");
    DualPotDrv_Init();

    result = DualPotDrv_Main(2, 1000.25);

    DualPotDrv_DeInit();
    printf("The result is %d", result);
    return 0;
}
