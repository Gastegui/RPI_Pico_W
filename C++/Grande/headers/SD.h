//
// Created by julen on 9/10/24.
//

#ifndef SD_H
#define SD_H

#include <iostream>
#include <ff.h>
#include <pico/stdlib.h>

#include "CONSTANTES.h"

extern void abortar(const char* msg);

class SD
{
    bool montado{false};
    bool abierto{false};
    char linea[255]{""};
    FIL fil{nullptr};
    FRESULT fr{FR_DENIED};
    BYTE modo{0};
    FATFS fs{0};

public:

    SD()
    {
        montar();
    }

    ~SD()
    {
        desmontar();
    }

    [[nodiscard]] bool getMontado() const { return montado; }
    [[nodiscard]] bool getAbierto() const { return abierto; }
    [[nodiscard]] char* getLinea() { return linea; }

    void montar()
    {
        fr = f_mount(&fs, "", 1);

        if(fr == FR_OK)
            montado = true;
        else
            abortar("ERROR: no se ha podido montar la SD");

        gpio_put(PIN_SD_LED, true);
    }

    void desmontar()
    {
        if(!montado)
            return;

        if(abierto)
            cerrar();

        fr = f_unmount("");

        if(fr != FR_OK)
            abortar("ERROR: no se ha conseguido desmontar la SD\n");

        montado = false;
        gpio_put(PIN_SD_LED, false);
    }

    void abrir(const char* path, const BYTE modo_)
    {
        if(!montado)
            abortar("ERROR: Se ha intentado abrir un archivo sin haber montado una SD antes\n");

        if(abierto)
            abortar("ERROR: Se ha intentado abrir un archivo habiendo otro habierto ya\n");

        modo = modo_;
        fr = f_open(&fil, path, modo);

        if(fr != FR_OK)
        {
            printf("Codigo: %d\n", fr);
            abortar("ERROR: No se ha podido abrir el archivo.");
        }

        abierto = true;
    }

    void cerrar()
    {

        abierto = false;
    }

    bool leerLinea()
    {
        if(!montado || !abierto)
            abortar("ERROR: Se ha intentado leer de un archivo sin haberlo cargado antes o sin haber montado la tarjeta\n");

        if(modo & FA_READ == 0)
            abortar("ERROR: Se ha intentado leer de un archivo que no está abierto para lectura\n");

        if(f_eof(&fil) != 0)
            return false;

        f_gets(linea, sizeof(linea), &fil);

        return true;
    }
};

#endif //SD_H
