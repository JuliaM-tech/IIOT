#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <sqlite3.h>
#include <math.h>

int verbose = 1;

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

// -----------------------------------------------------------------------------------------------

static void pabort(const char *s)
{
	perror(s);
	abort();
}

// -----------------------------------------------------------------------------------------------

static void spiadc_config_tx( int conf, uint8_t tx[3] )
{
	int i;

	uint8_t tx_dac[3] = { 0x00, 0x00, 0x00 };
	uint8_t n_tx_dac = 3;
	
	for (i=0; i < n_tx_dac; i++) {
		tx[i] = tx_dac[i];
	}
	
// Estableix el mode de comunicació en la parta alta del 2n byte
	tx[1]=conf<<4;
	
	if( verbose ) {
		for (i=0; i < n_tx_dac; i++) {
			//printf("spi tx dac byte:(%02d)=0x%02x\n",i,tx[i] );
		}
	}
		
}

// -----------------------------------------------------------------------------------------------


static int spiadc_transfer(int fd, uint8_t bits, uint32_t speed, uint16_t delay, uint8_t tx[3], uint8_t *rx, int len )
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

static int spiadc_config_transfer( int conf, int *value )
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
		sprintf( buffer, "can't open device (%s)", device );
		pabort( buffer );
	}

	/* spi mode 	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/* bits per word 	 */
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
	spiadc_config_tx( conf, tx );
		
	/* spi adc transfer */
	ret = spiadc_transfer( fd, bits, speed, delay, tx, rx, 3 );
	if (ret == 1)
		pabort("can't send spi message");

	close(fd);
	
	//printf("%b %b\n",rx[0], rx[1]);
	*value = rx[0] << 6 | rx[1] >> 2;
	//printf("%b\n",*value);

	return ret;
}
//-------------/*timer*/----------------------//

int set_timer(timer_t * timer_id, float delay, float interval, timer_callback * func, void * data) 
{
    int status =0;
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


void callback(union sigval si)
{
    int value_int;
    float value_volts, temp;
    int ret = 0;  // Declaración de ret

    // Configuración de lectura de ADC
    ret = spiadc_config_transfer(SINGLE_ENDED_CH2, &value_int);
    /*if (ret != 0) {
        //fprintf(stderr, "Error al leer el valor del ADC\n");
        //return;  // Sale de la función si hay un error
    }*/

    // Convertir el valor ADC a voltaje
    value_volts = 3.3 * value_int / 1023;
    // Convertir el voltaje a temperatura (suponiendo una fórmula de sensor)
    temp = value_volts * (1 / (10 * pow(10, -3)));

    // Imprimir los valores en consola
    printf("Voltage: %.3f V\n", value_volts);
    printf("Temperatura: %.1f °C\n", temp);
    printf("\n");

    // Conectar a la base de datos SQLite
    sqlite3 *db;
    char *errMessage = 0;
    int rc;

    rc = sqlite3_open("valor_sensor.db", &db);
    if (rc) {
        fprintf(stderr, "No se pudo abrir la base de datos: %s\n", sqlite3_errmsg(db));
        return;  // Sale de la función si no se pudo abrir la base de datos
    } else {
        //printf("Conexión exitosa a la base de datos\n");
    }

    // Crear la tabla si no existe
    const char *createTableSQL = "CREATE TABLE IF NOT EXISTS Sensor ("
                                 "id INTEGER , "
                                 "Temperatura REAL, "
                                 "VOLTAGE REAL);";
    rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMessage);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error al crear la tabla: %s\n", errMessage);
        sqlite3_free(errMessage);  // Liberar el mensaje de error
        sqlite3_close(db);         // Cerrar la base de datos
        return;  // Sale de la función si hubo un error creando la tabla
    } else {
       // printf("Tabla 'Sensor' creada o ya existe\n");
    }

    // Insertar los datos en la tabla usando parámetros preparados
    const char *insertSQL = "INSERT INTO Sensor (id, Temperatura, VOLTAGE) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    // Preparar la consulta SQL
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);  // Cerrar la base de datos en caso de error
        return;  // Sale de la función si no se pudo preparar la consulta
    }

    // Vincular las variables a la consulta
    sqlite3_bind_double(stmt, 1, 101);		// Vincula el valor de 'id' a la primera posición del placeholder '?'
    sqlite3_bind_double(stmt, 2, temp);         // Vincula el valor de 'temp' a la segunda posición del placeholder '?'
    sqlite3_bind_double(stmt, 3, value_volts);  // Vincula el valor de 'value_volts' a la tercera posición del placeholder '?'

    // Ejecutar la consulta
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error al insertar los datos: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);  // Finalizar la declaración preparada
        sqlite3_close(db);       // Cerrar la base de datos en caso de error
        return;  // Sale de la función si la inserción falla
    } else {
        //printf("Datos insertados correctamente en la tabla 'Sensor'\n");
    }

    // Finalizar la declaración preparada
    sqlite3_finalize(stmt);

    // Cerrar la base de datos
    sqlite3_close(db);
}

// -----------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{

	timer_t Temperatura;
	
	set_timer(&Temperatura, 1, 1, callback, NULL);
		
	
	getchar();
	
	
}

