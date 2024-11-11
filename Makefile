# Nombre del compilador
CC = gcc

# Opciones de compilación
CFLAGS = -Wall -fPIC

# Regla por defecto (el objetivo principal)
all: sensor client_smtp

build:
	mkdir build

sensor: build src/sensor/Sensor_LM35.c
	$(CC) src/sensor/Sensor_LM35.c -c -o build/Sensor_LM35.o 
	$(CC) build/Sensor_LM35.o -o build/Sensor_LM35 

# Compilar main.o y enlazar con la biblioteca
client_smtp: build/main.o build/libemail.so
	$(CC) src/client_smtp/main.c -Isrc/lib_email/ -Lbuild/ -lemail -o build/client_smtp

# Crear el archivo objeto main.o
build/main.o: src/client_smtp/main.c src/lib_email/email.h
	$(CC) $(CFLAGS) -c src/client_smtp/main.c -Isrc/lib_email/ -o build/main.o

# Crear la biblioteca compartida libemail.so a partir de email.c
libemail: build/libemail.so

build/libemail.so: build src/lib_email/email.c src/lib_email/email.h
	$(CC) -Wall -fPIC -shared src/lib_email/email.c -o build/libemail.so 

# Limpiar archivos generados
clean:
	rm -rf build

test:
	LD_LIBRARY_PATH=build/ ./build/client_smtp --servidor 172.20.0.21 --origen 1632442@campus.euss.org --desti 1523276@campus.euss.org --tema "tema del mail" --fitxer build/text_email.txt
