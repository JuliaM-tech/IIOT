# Nombre del compilador
CC = gcc

# Opciones de compilación
CFLAGS = -Wall -fPIC

# Objetivos
TARGET = main       # El programa ejecutable que queremos crear
LIBNAME = libemail.so   # La biblioteca compartida que queremos crear

# Regla por defecto (el objetivo principal)
all: $(TARGET)

# Compilar main.o y enlazar con la biblioteca
$(TARGET): main.o $(LIBNAME)
	$(CC) -o $(TARGET) main.o -L. -lemail

# Crear el archivo objeto main.o
main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

# Crear la biblioteca compartida libemail.so a partir de email.c
$(LIBNAME): email.c
	$(CC) $(CFLAGS) -shared email.c -o $(LIBNAME)

# Limpiar archivos generados
clean:
	rm -f main.o $(TARGET) $(LIBNAME)

