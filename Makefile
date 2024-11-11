# Nombre del compilador
CC = gcc

# Opciones de compilación
CFLAGS = -Wall -fPIC

# Regla por defecto (el objetivo principal)
all: main

# Compilar main.o y enlazar con la biblioteca
$(TARGET): main.o libemail.so 
	$(CC) -o main main.o -L. -lemail

# Crear el archivo objeto main.o
main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

# Crear la biblioteca compartida libemail.so a partir de email.c
libemail.so: email.c email.h
	$(CC) -Wall -fPIC -shared email.c -o libemail.so 

# Limpiar archivos generados
clean:
	rm -f main.o main libemail.so 

