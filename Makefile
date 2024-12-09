# Nombre del compilador
CC = gcc

# Opciones de compilación
CFLAGS = -Wall -fPIC

# Regla por defecto (el objetivo principal)
all: sensor

build:
	mkdir build
	
#SENSOR
sensor: build/Sensor
	./build/Sensor
	
build/Sensor: build src/sensor/Sensor.c build/libcloud.a src/Cloud/cloud.h
	$(CC) -c src/sensor/Sensor.c -o build/Sensor.o -I/home/pi/Desktop/IIOT/src/Cloud  
	$(CC) -Wall build/Sensor.o -o build/Sensor -L/home/pi/Desktop/IIOT/build -lcloud -lsqlite3 -l gpiod -li2c -lm 

build/libcloud.a: build src/Cloud/cloud.c src/Cloud/cloud.h
	$(CC) -c -o build/cloud.o src/Cloud/cloud.c
	$(AR) rcs build/libcloud.a build/cloud.o

#Client_smtp+email
# Compilar main.o y enlazar con la biblioteca
client_smtp: build/client_smtp

build/client_smtp: build/libemail.so src/client_smtp/main.c src/lib_email/email.h
	$(CC) src/client_smtp/main.c -Isrc/lib_email/ -Lbuild/ -lemail -o build/client_smtp
    
#SQLite
informe: build/informe
	./build/informe

build/informe: src/SQLite/informe.c build
	$(CC) src/SQLite/informe.c -o build/informe -lsqlite3 -lm
	


# Crear la biblioteca compartida libemail.so a partir de email.c
libemail: build/libemail.so

build/libemail.so: build src/lib_email/email.c src/lib_email/email.h
	$(CC) -Wall -fPIC -c -o build/libemail.o src/lib_email/email.c  
	$(CC) -Wall -shared -fPIC -o build/libemail.so build/libemail.o

# Limpiar archivos generados
clean:
	rm -rf build

test: build/client_smtp
	LD_LIBRARY_PATH=build/ ./build/client_smtp --servidor 172.20.0.21 --origen 1632442@campus.euss.org --desti 1523276@campus.euss.org --tema "tema del mail" --fitxer src/SQLite/resum.txt

