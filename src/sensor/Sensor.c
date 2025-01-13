#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include "cloud.h"
#include <sqlite3.h>
#include <gpiod.h>
#include "email.h"
#include <mosquitto.h>
#include <time.h>         // Para la función time(), localtime(), etc.
#include <sys/stat.h>     // A veces se requiere (depende de la distro)
#include <sys/types.h>    // A veces se requiere (depende de la distro)
#include <mosquitto.h>

// Definiciones y variables globales
int verbose = 1;
int fd2;          // Descriptor de archivo para I2C
char data[5] = "";
uint8_t trig[3] = { 0xAC, 0X33, 0x00 };
uint8_t state;
char global_bbdd[] = "/home/pi/Desktop/IIOT/baseDeDatos.db";

static char *cntdevice = "/dev/spidev0.0";

// ADC configurations (MCP3008)
#define SINGLE_ENDED_CH0 8
#define SINGLE_ENDED_CH1 9
#define SINGLE_ENDED_CH2 10
#define SINGLE_ENDED_CH3 11
#define SINGLE_ENDED_CH4 12
#define SINGLE_ENDED_CH5 13
#define SINGLE_ENDED_CH6 14
#define SINGLE_ENDED_CH7 15

// Canales diferenciales (por si los usas)
#define DIFERENTIAL_CH0_CH1 0
#define DIFERENTIAL_CH1_CH0 1
#define DIFERENTIAL_CH2_CH3 2
#define DIFERENTIAL_CH3_CH2 3
#define DIFERENTIAL_CH4_CH5 4
#define DIFERENTIAL_CH5_CH4 5
#define DIFERENTIAL_CH6_CH7 6
#define DIFERENTIAL_CH7_CH6 7

// AHT20
#define DEV_ID 0x38
#define DEV_PATH "/dev/i2c-1"
#define TRIGGER 0xAC3300
#define STATUS 0x71
#define RESET 0xBA

// VZ89TE
#define VZ89TE_DEV_ID 0x70  // Dirección del dispositivo VZ89TE

struct mosquitto *mosq = NULL; // Si lo usas para MQTT, inicialízalo en main()

/* =====================================================================
   Función para calcular el CRC (según el datasheet del VZ89TE)
   ===================================================================== */
uint8_t calculate_crc(uint8_t *buffer, size_t size) {
    uint16_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += buffer[i];
    }
    uint8_t crc = sum + (sum >> 8); // Suma con carry
    return (uint8_t)(0xFF - crc);   // Complemento
}

/* =====================================================================
   Función para leer datos del VZ89TE
   ===================================================================== */
void VZ89TE_read(float *voc, float *co2) {
    i2cINI();
    
    // Comando para obtener el estado del sensor
    uint8_t command = 0x0C;
    uint8_t buffer[7] = {0}; 
    uint8_t crc;

    // Configurar comunicación I2C con el VZ89TE (0x70)
    if (ioctl(fd2, I2C_SLAVE, VZ89TE_DEV_ID) < 0) {
        perror("Error al configurar el esclavo I2C (VZ89TE)");
        return;
    }

    // Calcular CRC para el comando
    crc = calculate_crc(&command, 1);

    // Enviar comando al sensor
    uint8_t write_buffer[6] = {command, 0x00, 0x00, 0x00, 0x00, crc};
    if (write(fd2, write_buffer, sizeof(write_buffer)) != sizeof(write_buffer)) {
        perror("Error al enviar el comando al sensor VZ89TE");
        return;
    }

    // Esperar 100 ms (requisito del sensor VZ89TE)
    usleep(100000);

    // Leer los datos del sensor
    if (read(fd2, buffer, 7) != 7) {
        perror("Error al leer datos del sensor VZ89TE");
        return;
    }

    // Verificar el CRC de los datos recibidos
    crc = calculate_crc(buffer, 6);
    if (crc != buffer[6]) {
        fprintf(stderr, "Error: CRC no coincide en VZ89TE\n");
        return;
    }

    // Procesar los datos
    // VOC en ppb
    *voc = (buffer[0] - 13) * (1000.0 / 229.0);
    // CO₂ en ppm
    *co2 = (buffer[1] - 13) * (1600.0 / 229.0) + 400.0;

    printf("VOC: %.2f ppb, CO2: %.2f ppm\n", *voc, *co2);
}

/* =====================================================================
   Wrappers de libgpiod para set/get
   ===================================================================== */
int gpiod_ctxless_get_value(const char *device, unsigned int offset,
                            bool active_low, const char *consumer);

int gpiod_ctxless_set_value(const char *device, unsigned int offset, int value,
                            bool active_low, const char *consumer,
                            gpiod_ctxless_set_value_cb cb,
                            void *data);

/* =====================================================================
   Funciones MQTT 
   ===================================================================== */

int mosquitto_send (sensor,valor){
	int rc;
	struct mosquitto *mosq; 
	
	mosquitto_lib_init();
	
	mosq = mosquitto_new("publisher-test", true, NULL);

	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	
	if (rc!= 0){
		printf("El client no s'ha pogut connectar! Codi d'error %d\n", rc);
		mosquitto_destroy(mosq);
		return-1;
	}
	
	printf ("We are now connected to the broker\n");
	mosquitto_publish(mosq,NULL, sensor,6, valor, 0, false);
	mosquitto_disconnect (mosq);
	mosquitto_destroy(mosq);

	mosquitto_lib_cleanup();
	return 0;
}

/* =====================================================================
   Función para abortar con mensaje de error
   ===================================================================== */
static void pabort(const char *s) {
    perror(s);
    abort();
}

/* =====================================================================
   Configuración del buffer TX para MCP3008
   ===================================================================== */
static void spiadc_config_tx(int conf, uint8_t tx[3]) {
    int i;
    uint8_t tx_dac[3] = { 0x00, 0x00, 0x00 };
    uint8_t n_tx_dac = 3;

    for (i = 0; i < n_tx_dac; i++) {
        tx[i] = tx_dac[i];
    }

    // Establece el modo de comunicacion en la parte alta del 2o byte
    tx[1] = conf << 4;

    if (verbose) {
        // Si deseas imprimir, descomenta:
        // for (i = 0; i < n_tx_dac; i++) {
        //     printf("spi tx dac byte:(%02d)=0x%02x\n", i, tx[i]);
        // }
    }
}

/* =====================================================================
   Transferencia SPI al MCP3008
   ===================================================================== */
static int spiadc_transfer(int fd, uint8_t bits, uint32_t speed, uint16_t delay,
                           uint8_t tx[3], uint8_t *rx, int len) {
    int ret;
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len * sizeof(uint8_t),
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    return ret;
}

/* =====================================================================
   Configuración y lectura de ADC a través de MCP3008
   ===================================================================== */
static int spiadc_config_transfer(int conf, int *value) {
    int ret = 0;
    int fd;
    uint8_t rx[2];
    char buffer[255];

    // Parámetros SPI
    char *device = cntdevice;
    // El modo preferido: SPI_CPHA (depende de tu cableado)
    uint8_t mode = SPI_CPHA;
    uint8_t bits = 8;
    uint32_t speed = 500000; // máximo ~1.5MHz
    uint16_t delay = 0;

    // Buffer de transmisión
    uint8_t tx[3];

    // Abrir el dispositivo SPI
    fd = open(device, O_RDWR);
    if (fd < 0) {
        sprintf(buffer, "can't open device (%s)", device);
        pabort(buffer);
    }

    // Configurar modo SPI
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("can't set spi mode");

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("can't get spi mode");

    // Bits por palabra
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't set bits per word");

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't get bits per word");

    // Velocidad máxima
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't set max speed hz");

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't get max speed hz");

    // Construir datos a transferir
    spiadc_config_tx(conf, tx);

    // Transferir por SPI
    ret = spiadc_transfer(fd, bits, speed, delay, tx, rx, 3);
    if (ret == 1) {
        pabort("can't send spi message");
    }

    close(fd);

    // El MCP3008 regresa 10 bits repartidos en rx[0] y rx[1]
    *value = (rx[0] << 6) | (rx[1] >> 2);
    return ret;
}

/* =====================================================================
   Creación de timer
   ===================================================================== */
typedef void (timer_callback) (union sigval);

int set_timer(timer_t *timer_id, float delay, float interval,
              timer_callback *func, void *data) {
    int status = 0;
    struct itimerspec ts;
    struct sigevent se;

    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = data;
    se.sigev_notify_function = func;
    se.sigev_notify_attributes = NULL;

    status = timer_create(CLOCK_REALTIME, &se, timer_id);

    ts.it_value.tv_sec = (time_t)abs(delay);
    ts.it_value.tv_nsec = (long)((delay - abs(delay)) * 1e9);
    ts.it_interval.tv_sec = (time_t)abs(interval);
    ts.it_interval.tv_nsec = (long)((interval - abs(interval)) * 1e9);

    status = timer_settime(*timer_id, 0, &ts, 0);
    return status; // O retorna 0, dependiendo de si quieres controlar el error
}

/* =====================================================================
   Lectura del LM35
   ===================================================================== */
void lm35(float *valor) {
    int value_int;
    float value_volts;

    spiadc_config_transfer(SINGLE_ENDED_CH2, &value_int);
    value_volts = 3.3f * value_int / 1023.0f;

    // LM35 -> 10mV/°C -> 1V = 100°C
    // => factor de multiplicación ~100
    *valor = value_volts * 100.0f; 
}

/* =====================================================================
   Inicialización de la comunicación I2C (AHT20, VZ89TE, etc.)
   ===================================================================== */
void i2cINI(void) {
   fd2 = open(DEV_PATH, O_RDWR);
    if (fd2 < 0) {
        perror("Error al abrir I2C");
        exit(1);
    }
    printf("I2C inicializado correctamente.\n");
}

/* =====================================================================
   Lectura de AHT20 - Humedad
   ===================================================================== */
void AHT20(long double *valor) {
     i2cINI();
     
     if (fd2 < 0) {
        perror("Error: I2C no inicializado");
        return;
    }

    // Configurar AHT20 (0x38) antes de acceder
    if (ioctl(fd2, I2C_SLAVE, DEV_ID) < 0) {
        perror("Error al configurar I2C para AHT20");
        return;
    }

    // Enviar comando de medición
    uint8_t cmd[3] = {0xAC, 0x0033, 0x00};
    if (write(fd2, cmd, 3) != 3) {
        perror("Error al iniciar medición en AHT20");
        return;
    }

    usleep(80000);  // Esperar 80ms

    // Verificar estado del sensor
    uint8_t status;
    uint8_t status_cmd = STATUS;
    write(fd2, &status_cmd, 1);
    read(fd2, &status, 1);

    if (status & 0x80) {  // Bit 7 indica si está ocupado
        fprintf(stderr, "Error: El AHT20 aún está midiendo.\n");
        return;
    }

    // Leer 6 bytes de datos
    uint8_t data[6];
    if (read(fd2, data, 6) != 6) {
        perror("Error al leer datos de AHT20");
        return;
    }


    // Cálculo de la humedad
    int hum = (data[1] << 12) | (data[2] << 4) | ((data[3] & 0xF0) >> 4);
    long double calcH = (hum * 100.0) / 1048576.0; // 2^20 = 1048576

    *valor = calcH;
    close(fd2);

}

/* =====================================================================
   Insertar datos en la base de datos
   ===================================================================== */
void SQLite(char *base_dades, char *id_sensor, double valor, char *Temps) {
    sqlite3 *db;
    char *errMessage = 0;
    int rc;

    // Abrir base de datos
    rc = sqlite3_open(base_dades, &db);
    if (rc) {
        fprintf(stderr, "No se pudo abrir la base de datos: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Construir SQL
    char insertSQL[300];
    sprintf(insertSQL, "INSERT INTO Sensor (id, Valor, Temps) VALUES ('%s', %f, '%s');",
            id_sensor, valor, Temps);

    rc = sqlite3_exec(db, insertSQL, 0, 0, &errMessage);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error al insertar datos: %s\n", errMessage);
        sqlite3_free(errMessage);
    }

    // Cerrar base de datos
    sqlite3_close(db);
}

/* =====================================================================
   Ejemplo de encender LED rojo (GPIO) usando libgpiod
   ===================================================================== */
void led_vermell() {
    int ret, value;
    // Leer el pin 17 (SW1) de gpiochip0, negando el valor
    value = gpiod_ctxless_get_value("gpiochip0", 17, true, "sw1");
    // printf("sw1 = %d\n",value);

    // Encender LED en pin 18
    ret = gpiod_ctxless_set_value("gpiochip0", 18, 1, false, "LED RED", NULL, NULL);
    // printf("ok led red = %d\n",ret);
    sleep(0.5);

    // Apagar LED
    ret = gpiod_ctxless_set_value("gpiochip0", 18, 0, false, "LED RED", NULL, NULL);
    // printf("ok led red = %d\n",ret);

    // Si quieres retornar algo, cambia la firma de la función
    // return ret;
}

/* =====================================================================
   Función callback del timer
   ===================================================================== */
void callback(union sigval si) {
    float temp, voc, co2;
    long double hum;
    char id_sensorT[]   = "101";
    char id_sensorH[]   = "102";
    char id_sensorVOC[] = "105";
    char id_sensorCO2[] = "104";
    char valorT[256], valorH[256], valorVOC[256], valorCO2[256];

    time_t now = time(NULL);
    
    struct tm *local_time = localtime(&now);

    printf("Hora de medida: %s", asctime(local_time));

    // 1) Lectura de temperatura (LM35)
    lm35(&temp);
    printf("Lectura temperatura: %.1fC\n", temp);
    sprintf(valorT, "%f", temp);
    cloud(id_sensorT, valorT);
    SQLite(global_bbdd, id_sensorT, temp, asctime(local_time));

    // 2) Lectura de humedad (AHT20)
    //    Asegúrate de que el fd2 esté apuntando a 0x38
    AHT20(&hum);
    printf("Lectura humedad: %.2Lf\n", hum);
    sprintf(valorH, "%Lf", hum);
    cloud(id_sensorH, valorH);
    SQLite(global_bbdd, id_sensorH, hum, asctime(local_time));
    
    // 3) Lectura de VOC y CO2 (VZ89TE)
    //    La función VZ89TE_read() hace el ioctl() para 0x70 internamente.
    
    VZ89TE_read(&voc, &co2);
    sprintf(valorCO2, "%f", co2);
    cloud(id_sensorCO2, valorCO2);
    sprintf(valorVOC, "%f", voc);
    cloud(id_sensorVOC, valorVOC);
    

    
    SQLite(global_bbdd, id_sensorVOC, voc, asctime(local_time));

   
    SQLite(global_bbdd, id_sensorCO2, co2, asctime(local_time));

	
	int intochar;  
	
    if (temp > 20) {
	mosquitto_send("temperatura","1");
    
    }
    
    if (temp < 15) {
	mosquitto_send("temperatura","0");	
    }

//email alarma VOC
    if (voc >  1000 ){
        mosquitto_send("voc","1");	
        }

//email alarma CO2
    if (co2 > 800 ){
        mosquitto_send("co2","1");	
        }
        
    if (co2 < 400 ){
        mosquitto_send("co2", "0");	
        }
    
	/* sprintf(intochar, "%.1f", temp);
	mosquitto_send("temperatura",intochar);
	
	sprintf(intochar, "%.3LF", hum);
	mosquitto_send("humitat",intochar);
    
    sprintf(intochar, "%f", voc);
	mosquitto_send("voc",intochar);
    
    sprintf(intochar, "%f", co2);
	mosquitto_send("co2",intochar);*/
	 
	led_vermell();

    // 4) Activar LED rojo como indicador
    led_vermell();
}

/* =====================================================================
   Inicialización de la tabla "Sensor" en SQLite (si no existe)
   ===================================================================== */
int init_base_dades(char *base_dades) {
    sqlite3 *db;
    char *errMessage = 0;
    int rc;

    // Abrir la base de datos
    rc = sqlite3_open(base_dades, &db);
    if (rc) {
        fprintf(stderr, "No se pudo abrir la base de datos: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Crear la tabla si no existe
    const char *createTableSQL = 
        "CREATE TABLE IF NOT EXISTS Sensor ("
        "id INTEGER, "
        "Valor REAL, "
        "Temps TEXT);";

    rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMessage);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error al crear la tabla: %s\n", errMessage);
        sqlite3_free(errMessage);
        sqlite3_close(db);
        return -1;
    }

    // Cerrar la base de datos
    sqlite3_close(db);
    return 0;
}

/* =====================================================================
   Función principal
   ===================================================================== */
int main(int argc, char *argv[]) {
  

    // Configurar la base de datos
    init_base_dades(global_bbdd);

    // Crear un timer para lectura periódica de sensores
    timer_t mesures;
    // Ejemplo: delay inicial = 1s, intervalo = 5s
    set_timer(&mesures, 1, 5, callback, NULL);

    
    getchar();
    close(fd2);

    

    return 0;
}
