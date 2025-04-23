#include "includes/core0.h"
#include "includes/core1.h"

#include <pico/stdlib.h>
#include "classes/motor.h"
#include "pico/multicore.h"
#include "includes/core0.h"
#include "includes/core1.h"
#include <stdio.h>
#include <string.h>

int main()
{
    stdio_init_all();

    sleep_ms(5000);

    printf("INICIO CORE 0\n");

    multicore_launch_core1(&core1_main);

    Motor motorAI(18, 19);
    Motor motorAD(8, 9, true);
    Motor motorDI(20, 21, true);
    Motor motorDD(6, 7, true);

    volatile int lastMove = 0;

    while(true)
    {
        if(multicore_fifo_rvalid())
        {
            lastMove = multicore_fifo_pop_blocking();
            printf("MOVER DIRECCION: %d", lastMove);
            switch(lastMove)
            {
                case 1:
                    motorAI.alante(50);
                    motorAD.alante(50);
                    motorDI.alante(50);
                    motorDD.alante(50);
                    break;
                case 2:
                    motorAI.atras(50);
                    motorAD.atras(50);
                    motorDI.atras(50);
                    motorDD.atras(50);
                    break;
                case 3:
                    motorAD.alante(50);
                    motorDD.alante(50);
                    motorAI.parar();
                    motorDI.parar();
                    break;
                case 4:
                    motorAI.alante(50);
                    motorDI.alante(50);
                    motorAD.parar();
                    motorDD.parar();
                    break;
                case 0:
                default:
                    motorAI.parar();
                    motorAD.parar();
                    motorDI.parar();
                    motorDD.parar();
                    break;
            }
        }
    }

    return 0;
}
