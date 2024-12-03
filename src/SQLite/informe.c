#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

static int callback(void *data, int argc, char **argv, char **azColName)
{
    char (*result)[16] = data; // Usamos un array para almacenar un solo resultado.
    if (argc > 0 && argv[0] != NULL)
    {
        sprintf((*result), "%s", argv[0]); // Guardar el resultado del cálculo.
    }
    return 0;
}

int main(int argc, char *argv[])
{
    sqlite3 *db;
    const unsigned int MAX_LENGTH = 1000;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    char result[2][16]; // Para MAX y MIN
    char avgResult[16]; // Para AVG
    char filename[] = "/home/pi/Desktop/IIOT/src/SQLite/resum.txt";
    
    remove(filename);
    
    /* Abrir base de datos */
    rc = sqlite3_open("/home/pi/Desktop/IIOT/baseDeDatos.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (0);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Abrir archivo para el resumen */
    FILE *fp = fopen(filename, "w");

    if (fp == NULL)
    {
        printf("Error: could not open file %s", filename);
        return 1;
    }

    /* Consultas para Temperatura (ID = 101) */

    fprintf(fp, "Resumen de Temperatura (ID = 101):\n");

    // Máximo
    sql = "SELECT MAX(Valor) FROM Sensor WHERE id = 101;";
    rc = sqlite3_exec(db, sql, callback, &result[0], &zErrMsg);
    fprintf(fp, "MAX -- Temperatura: %s\n", result[0]);

    // Mínimo
    sql = "SELECT MIN(Valor) FROM Sensor WHERE id = 101;";
    rc = sqlite3_exec(db, sql, callback, &result[1], &zErrMsg);
    fprintf(fp, "MIN -- Temperatura: %s\n", result[1]);

    // Promedio
    sql = "SELECT AVG(Valor) FROM Sensor WHERE id = 101;";
    rc = sqlite3_exec(db, sql, callback, &avgResult, &zErrMsg);
    fprintf(fp, "AVG -- Temperatura promedio: %s\n", avgResult);

    /* Consultas para Humedad (ID = 102) */

    fprintf(fp, "\nResumen de Humedad (ID = 102):\n");

    // Máximo
    sql = "SELECT MAX(Valor) FROM Sensor WHERE id = 102;";
    rc = sqlite3_exec(db, sql, callback, &result[0], &zErrMsg);
    fprintf(fp, "MAX -- Humedad: %s\n", result[0]);

    // Mínimo
    sql = "SELECT MIN(Valor) FROM Sensor WHERE id = 102;";
    rc = sqlite3_exec(db, sql, callback, &result[1], &zErrMsg);
    fprintf(fp, "MIN -- Humedad: %s\n", result[1]);

    // Promedio
    sql = "SELECT AVG(Valor) FROM Sensor WHERE id = 102;";
    rc = sqlite3_exec(db, sql, callback, &avgResult, &zErrMsg);
    fprintf(fp, "AVG -- Humedad promedio: %s\n", avgResult);

    /* Manejo de errores */
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Operation done successfully\n");
    }

    /* Cerrar base de datos y archivo */
    sqlite3_close(db);
    fclose(fp);

    return 0;
}
