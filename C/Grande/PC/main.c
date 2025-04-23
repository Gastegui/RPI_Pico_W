#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>

#define BAUDRATE B115200
#define SERIAL_PORT "/dev/ttyACM0"

int main() {
    int fd, res;
    struct termios oldtio, newtio;
    char buf[8];

    fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror(SERIAL_PORT);
        exit(1);
    }

    // Guardar la configuración actual del puerto serie
    tcgetattr(fd, &oldtio);

    // Configurar el puerto serie
    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Configurar tiempos de espera
    newtio.c_cc[VTIME] = 0;   // Tiempo de espera en décimas de segundo
    newtio.c_cc[VMIN] = 1;    // Número mínimo de caracteres a leer

    // Aplicar la configuración al puerto serie
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    printf("Iniciando lectura desde el puerto serie...\n");

    // Bucle de lectura desde el puerto serie
    while (1) {
        res = read(fd, buf, sizeof(buf));
        if (res < 0) {
            perror("Error al leer desde el puerto serie");
            break;
        } else if (res > 0) {
            printf("Leídos %d bytes. Recibido: ", res);
            for (int i = 0; i < res; i++) {
                printf("%02X ", buf[i]);
            }
            printf("\n");
            memset(buf, 0, sizeof(buf));
        }
    }

    // Restaurar la configuración original del puerto serie
    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);

    return 0;
}
