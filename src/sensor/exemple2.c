#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEV_ID 0x38
#define DEV_PATH "/dev/i2c-1"

void delay(int);

int main(void)
{
    int fd;
    uint8_t cmd_init[] = {0xBE, 0x08, 0x00}; // Comando de inicialización
    uint8_t cmd_measure[] = {0xAC, 0x33, 0x00}; // Comando de medición
    uint8_t data[6];
    double humidity = 0.0, temperature = 0.0;

    /* Abrir dispositivo I2C */
    if ((fd = open(DEV_PATH, O_RDWR)) < 0) {
        perror("No se puede abrir el dispositivo I2C");
        exit(1);
    }

    /* Configurar el dispositivo esclavo */
    if (ioctl(fd, I2C_SLAVE, DEV_ID) < 0) {
        perror("No se puede configurar el dispositivo esclavo I2C");
        close(fd);
        exit(1);
    }

    /* Inicializar el sensor */
    if (write(fd, cmd_init, 3) != 3) {
        perror("Error al inicializar el sensor AHT20");
        close(fd);
        exit(1);
    }
    usleep(50000); // Esperar 50 ms

    /* Iniciar una medición */
    if (write(fd, cmd_measure, 3) != 3) {
        perror("Error al enviar comando de medición");
        close(fd);
        exit(1);
    }
    usleep(80000); // Esperar 80 ms para completar la medición

    /* Leer datos del sensor */
    if (read(fd, data, 6) != 6) {
        perror("Error al leer los datos del sensor");
        close(fd);
        exit(1);
    }

    /* Procesar los datos */
    uint32_t raw_humidity = ((data[1] << 16) | (data[2] << 8) | data[3]) >> 4;
    uint32_t raw_temperature = ((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5];

    humidity = (raw_humidity / 1048576.0) * 100.0; // Calcular humedad relativa
    temperature = (raw_temperature / 1048576.0) * 200.0 - 50.0; // Calcular temperatura

    printf("Humedad relativa: %.2f%%\n", humidity);
    printf("Temperatura: %.2f°C\n", temperature);

    close(fd);

    return 0;
}

