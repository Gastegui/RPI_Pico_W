//
// Created by julen on 9/10/24.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../headers/core1.h"
#include "../headers/CONSTANTES.h"
#include "../headers/Buzzer.h"
#include "../headers/Led.h"

void core1()
{
    Buzzer buzzer{PIN_BUZZER};

    char str[10];

    Led verde{PIN_LED_VERDE, false};

    while(true)
    {
        int num = 0;
        memset(str,0,sizeof(str));

        fgets(str, 4, stdin);

        num = atoi(str);

        verde.alternar();
        //buzzer.sonido(1000, 500, num);
    }
}
