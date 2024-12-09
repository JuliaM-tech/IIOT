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


int verbose = 1;
int fd2;
char data[5]="";
uint8_t trig[3] = { 0xAC, 0X33, 0x00 };
uint8_t state;
char global_bbdd[] = "/home/pi/Desktop/IIOT/baseDeDatos.db";

static char *cntdevice = "/dev/spidev0.0";

typedef void (timer_callback) (union sigval);

//ADC configurations segons manual MCP3008
#define SINGLE_ENDED_CH0 8
#define SINGLE_ENDED_CH1 9
#define SINGLE_ENDED_CH2 10
#define SINGLE_ENDED_CH3 11
#define SINGLE_ENDED_CH4 12
#define SINGLE_ENDED_CH5 13
#define SINGLE_ENDED_CH6 14
#define SINGLE_ENDED_CH7 15
#define DIFERENTIAL_CH0_CH1 0 //Chanel CH0 = IN+ CH1 = IN-
#define DIFERENTIAL_CH1_CH0 1 //Chanel CH0 = IN- CH1 = IN+
#define DIFERENTIAL_CH2_CH3 2 //Chanel CH2 = IN+ CH3 = IN-
#define DIFERENTIAL_CH3_CH2 3 //Chanel CH2 = IN- CH3 = IN+
#define DIFERENTIAL_CH4_CH5 4 //Chanel CH4 = IN+ CH5 = IN-
#define DIFERENTIAL_CH5_CH4 5 //Chanel CH4 = IN- CH5 = IN+
#define DIFERENTIAL_CH6_CH7 6 //Chanel CH6 = IN+ CH7 = IN-
#define DIFERENTIAL_CH7_CH6 7 //Chanel CH6 = IN- CH7 = IN+


#define DEV_ID 0x38
#define DEV_PATH "/dev/i2c-1"
#define TRIGGER 0xAC3300
#define STATUS 0x71
#define RESET 0xBA




int gpiod_ctxless_get_value(const char *device, unsigned int offset,
bool active_low, const char *consumer);

int gpiod_ctxless_set_value(const char *device, unsigned int offset, int value,
bool active_low, const char *consumer,
gpiod_ctxless_set_value_cb cb,
void *data);


// -----------------------------------------------------------------------------------------------

static void pabort(const char *s)
{
	perror(s);
	abort();
}

// -----------------------------------------------------------------------------------------------

static void spiadc_config_tx(int conf, uint8_t tx[3])
{
	int i;

	uint8_t tx_dac[3] = { 0x00, 0x00, 0x00 };
	uint8_t n_tx_dac = 3;

	for (i = 0; i < n_tx_dac; i++) {
		tx[i] = tx_dac[i];
	}

// Estableix el mode de comunicació en la parta alta del 2n byte
	tx[1] = conf<<4;

	if (verbose) {
		for (i = 0; i < n_tx_dac; i++) {
			//printf("spi tx dac byte:(%02d)=0x%02x\n",i,tx[i] );
		}
	}

}

// -----------------------------------------------------------------------------------------------


static int spiadc_transfer(int fd, uint8_t bits, uint32_t speed, uint16_t delay, uint8_t tx[3], uint8_t *rx, int len)
{
    int ret;

    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)tx,
	.rx_buf = (unsigned long)rx,
	.len = len * sizeof(uint8_t),
	.delay_usecs = delay,
	.speed_hz = speed,
	.bits_per_word = bits,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);


      return ret;
}



// -----------------------------------------------------------------------------------------------

static int spiadc_config_transfer(int conf, int *value)
{
	int ret = 0;
	int fd;
	uint8_t rx[2];
	char buffer[255];

	/* SPI parameters */
	char *device = cntdevice;
	//uint8_t mode = SPI_CPOL; //No va bé amb aquesta configuació, ha de ser CPHA
	uint8_t mode = SPI_CPHA;
	uint8_t bits = 8;
	uint32_t speed = 500000; //max 1500KHz
	uint16_t delay = 0;

	/* Transmission buffer */
	uint8_t tx[3];

	/* open device */
	fd = open(device, O_RDWR);
	if (fd < 0) {
		sprintf(buffer, "can't open device (%s)", device);
		pabort(buffer);
	}

	/* spi mode	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/* bits per word	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/* max speed hz  */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	/* build data to transfer */
	spiadc_config_tx(conf, tx);

	/* spi adc transfer */
	ret = spiadc_transfer(fd, bits, speed, delay, tx, rx, 3);
	if (ret == 1)
		pabort("can't send spi message");

	close(fd);

	//printf("%b %b\n",rx[0], rx[1]);
	*value = rx[0] << 6 | rx[1] >> 2;
	//printf("%x\n", *value);

	return ret;
}
//-------------/*timer*/----------------------//

int set_timer(timer_t *timer_id, float delay, float interval, timer_callback *func, void *data)
{
    int status = 0;
    struct itimerspec ts;
    struct sigevent se;

    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = data;
    se.sigev_notify_function = func;
    se.sigev_notify_attributes = NULL;

    status = timer_create(CLOCK_REALTIME, &se, timer_id);

    ts.it_value.tv_sec = abs(delay);
    ts.it_value.tv_nsec = (delay-abs(delay)) * 1e09;
    ts.it_interval.tv_sec = abs(interval);
    ts.it_interval.tv_nsec = (interval-abs(interval)) * 1e09;

    status = timer_settime(*timer_id, 0, &ts, 0);
    return 0;
}
void lm35(float *valor)
{
    int value_int;
    float value_volts;

    spiadc_config_transfer(SINGLE_ENDED_CH2, &value_int);
    //printf("Valor llegit (0-255) %d\n", value_int);
    value_volts = 3.3 * value_int / 1023;
    //printf("Voltatge %.3f V\n", value_volts);
    *valor = value_volts*(1/(10*pow(10, -3)));
    //printf("voltatge %.3f V\n", value_volts);
    //printf("Temperatura %.1f C\n", *valor);
    //printf("\n");
}


void i2cINI(void)
{
    //delay(50);

    /* open i2c comms */
    fd2 = open(DEV_PATH, O_RDWR);
    if (fd2 < 0) {
   	 perror("Unable to open i2c device");
   	 exit(1);
    }

    /* configure i2c slave */
    if (ioctl(fd2, I2C_SLAVE, DEV_ID) < 0) {
   	 perror("Unable to configure i2c slave device");
   	 close(fd2);
   	 exit(1);
    }

    i2c_smbus_write_byte(fd2, RESET);
    //delay(30);
    i2c_smbus_write_byte(fd2, STATUS);
    state = i2c_smbus_read_byte(fd2);
    printf("state: %b\n", state);

    if (!(state & 8)) {
   	 perror("Estado invalido");
   	 close(fd2);
   	 exit(1);
    } else {
   	 printf("Estado I2C OK\n");
    }
}


void AHT20(long double *valor)
{
    /* open i2c comms */
    fd2 = open(DEV_PATH, O_RDWR);
    if (fd2 < 0) {
   	 perror("Unable to open i2c device");
   	 
    }

    /* configure i2c slave */
    if (ioctl(fd2, I2C_SLAVE, DEV_ID) < 0) {
   	 perror("Unable to configure i2c slave device");
   	 close(fd2);
	
    }
    
    i2c_smbus_write_word_data(fd2, 0xAC, 0x0033);
    //delay(80);

    i2c_smbus_read_i2c_block_data(fd2, 0x00, 6, data);

    int hum = 0;
    //printf("hum: %x,%x,%x,%x,%x,%x\n", data[0], data[1],data[2],data[3],data[4], data[5]);
    hum = data[1] << 12;
    //printf("hum: %d\n", hum);
    hum = hum | (data[2] << 4);
    hum = hum | ((240 & data[3]) >> 4);
    //printf("hum: %d\n", hum);
    long double calcH = (hum * 100) / (long double)1048576;
    
    printf("humitat: %lf\n", calcH);

    *valor = calcH;
    
}


void SQLite(char *base_dades, char *id_sensor, double valor, char *Temps)
{
// Conectar a la base de datos SQLite
    sqlite3 *db;
    char *errMessage = 0;
    int rc;

    rc = sqlite3_open(base_dades, &db);
    if (rc) {
        fprintf(stderr, "No se pudo abrir la base de datos: %s\n", sqlite3_errmsg(db));
        return;  // Sale de la función si no se pudo abrir la base de datos
    } else {
        //printf("Conexión exitosa a la base de datos\n");
    }

    // Insertar los datos en la tabla usando parámetros preparados
    char *insertSQL[200];
    sprintf( insertSQL, "INSERT INTO Sensor (id, Valor, Temps) VALUES (%s, %f, '%s');", id_sensor, valor, Temps); ;

    rc = sqlite3_exec(db, insertSQL, 0, 0, &errMessage);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error al crear la tabla: %s\n", errMessage);
        sqlite3_free(errMessage);  // Liberar el mensaje de error
        sqlite3_close(db);         // Cerrar la base de datos
        return;  // Sale de la función si hubo un error creando la tabla
    } else {
       // printf("Tabla 'Sensor' creada o ya existe\n");
    }


    // Cerrar la base de datos
    sqlite3_close(db);
}



void led_vermell() {
	int ret, value;
// llegeix el pin 17 (SW1), del gpiochip0 i ho fa negant el valor rebut.
// assigna el nom a la lina "sw1"
	value = gpiod_ctxless_get_value("gpiochip0",17,true,"sw1");
	//printf("sw1 = %d\n",value);
// assigna un 1 al pin 18 (Led vermell), del gpiochip0, no nega la lògica
// assigna el nom a la lina "LED RED", no s'enllaça cap funció de callback
	ret = gpiod_ctxless_set_value("gpiochip0",18,1,false,"LED RED",NULL,NULL);
	//printf("ok led red = %d\n",ret);
	sleep(1);
// el mateix d'abans, passant la sortida a 0
	ret = gpiod_ctxless_set_value("gpiochip0",18,0,false,"LED RED",NULL,NULL);
	//printf("ok led red = %d\n",ret);
return 0;
}


void callback(union sigval si)
{

	float temp;
	double hum;
	char id_sensorT[]="101";
	char id_sensorH[]="102";
	char valorT[256];
	char valorH[256];
	//char temps[256];
	
	time_t now = time(NULL);
	// Convert to local time
	struct tm *local_time = localtime(&now);

	printf("Hora de medida: %s", asctime(local_time));
	
	lm35(&temp);
	printf("Lectura temperatura: %.1fC\n", temp);
	sprintf(valorT, "%f", temp);
	cloud(id_sensorT, valorT);
	SQLite( global_bbdd, id_sensorT, temp, asctime(local_time));
	
	AHT20(&hum);
	printf("Lectura humitat: %.3LF \n", hum);
	sprintf(valorH, "%LF", hum);
	cloud(id_sensorH, valorH);
	SQLite( global_bbdd, id_sensorH, hum, asctime(local_time));
	
	led_vermell();
}


int init_base_dades(char *base_dades) 
{
    	
    sqlite3 *db;
    char *errMessage = 0;
    int rc;

    rc = sqlite3_open(base_dades, &db);
    if (rc) {
        fprintf(stderr, "No se pudo abrir la base de datos: %s\n", sqlite3_errmsg(db));
        return;  // Sale de la función si no se pudo abrir la base de datos
    } else {
        //printf("Conexión exitosa a la base de datos\n");
    }

    // Crear la tabla si no existe
    const char *createTableSQL = "CREATE TABLE IF NOT EXISTS Sensor ("
                                 "id INTEGER, "
                                 "Valor REAL, "
				 "Temps REAL);";
    rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMessage);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error al crear la tabla: %s\n", errMessage);
        sqlite3_free(errMessage);  // Liberar el mensaje de error
        sqlite3_close(db);         // Cerrar la base de datos
        return;  // Sale de la función si hubo un error creando la tabla
    } else {
       // printf("Tabla 'Sensor' creada o ya existe\n");
    }
    // Cerrar la base de datos
    sqlite3_close(db);
} 

// -----------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	init_base_dades(global_bbdd);
	//i2cINI();
			
	timer_t mesures;

	set_timer(&mesures, 1, 300, callback, NULL);

	while(1) {
	    sleep(3600);
	}
	close(fd2);
	

}

