// SPDX-License-Identifier: GPL-2.0-or-later
/* IBM POWER Barrier Synchronization Register Driver
 *
 * Copyright IBM Corporation 2008
 *
 * Author: Sonny Rao <sonnyrao@us.ibm.com>*/
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <sqlite3.h>
 #include <string.h>
 #include <getopt.h>
 #include <unistd.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include "email.h"
 
 #define REQUEST_MSG_SIZE 1024
 #define REPLY_MSG_SIZE 500
 #define SERVER_PORT_NUM 25


    //1632442@campus.euss.org
void print_usage(void)
{
    printf("Usage: --servidor 172.20.0.21 --origen dr_clotet@euss.org --desti 1523276@campus.euss.cat --tema “Informe” --fitxer /home/pi/t/home/1523276@AD.EUSS.ES/Helena/Sprint4.0/src/SQLiteext_email.txt");
}

static int callback(void *data, int argc, char **argv, char **azColName)
{
    int i;
		
    fprintf(stderr, "Callback function called: ");
    

    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        strcpy(data, argv[i]);
        
    }
	
    printf("\n");
    return 0;
}

int main(int argc, char *argv[])
{
    sqlite3 *db;
    const unsigned int MAX_LENGTH=1000;
    char *zErrMsg = 0;
    int rc;
    char *sql;

    char IP[MAX_LENGTH];
    char Emisor[MAX_LENGTH];
    char Destinatario[MAX_LENGTH];
    char Temita[MAX_LENGTH];
    char cuerpo[MAX_LENGTH];
    int long_index = 0;

    int opt = 0;

    char Max_valor[MAX_LENGTH];
    char Min_valor[MAX_LENGTH];
    char Avg_valor[MAX_LENGTH];
    char Max_temps[MAX_LENGTH];
    char Min_temps[MAX_LENGTH];
    char filename[] = "resum.txt";
    
    if(argc < 2) {
        print_usage();
        return -1;
    }

       static struct option long_options[] = {
        {"servidor", required_argument, 0, 's'},
        {"origen", required_argument, 0, 'o'},
        {"desti", required_argument, 0, 'd'},
        {"tema", required_argument, 0, 't'},
        {"fitxer", required_argument, 0, 'x'}};

    while ((opt = getopt_long(argc, argv, "t:s:o:d:x:", long_options, &long_index)) != -1)
    {
        switch (opt)
        {
        case 't':
            strcpy(Temita, optarg);
            break;
        case 's':
            strcpy(IP, optarg);
            break;
        case 'o':
            strcpy(Emisor, optarg);
            break;
        case 'd':
            strcpy(Destinatario, optarg);
            break;
        case 'x':
            strcpy(filename, optarg);
            break;
        default:
            print_usage();
            exit(EXIT_FAILURE);
        }
    }
    
    /* Open database */
    rc = sqlite3_open("basedades_iiot.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (0);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n");
    }

	/* Obrim fitxer on guardarem el resum*/
	FILE *fp = fopen(filename, "w");
	
	if (fp == NULL)
	{
       printf("Error: could not open file %s", filename);
       return 1;
	}
		

        /* Create SQL statement valor maxim*/
        sql = "SELECT MAX(valor) from mesures";
        rc = sqlite3_exec(db, sql, callback, Max_valor, &zErrMsg);
        fprintf(fp, "Maxim valor: %s\n", Max_valor);
        
	    /* Create SQL statement valor minim*/
        sql = "SELECT MIN(valor) from mesures";
        rc = sqlite3_exec(db, sql, callback, Min_valor, &zErrMsg);
		fprintf(fp, "Minim valor: %s\n", Min_valor);
		
        /* Create SQL statement valor mitj*/
        sql = "SELECT AVG(valor) from mesures";
        rc = sqlite3_exec(db, sql, callback, Avg_valor, &zErrMsg);
		fprintf(fp, "Average valor: %s\n", Avg_valor);
		
        /* Create SQL statement temps maxim*/
        sql = "SELECT MAX(temps) from mesures";
        rc = sqlite3_exec(db, sql, callback, Max_temps, &zErrMsg);
		fprintf(fp, "Maxim temps: %s\n", Max_temps);
		
        /* Create SQL statement temps minim*/
        sql = "SELECT MIN(temps) from mesures";
        rc = sqlite3_exec(db, sql, callback, Min_temps, &zErrMsg);
		fprintf(fp, "Minim temps: %s\n", Min_temps);

 

    memset(cuerpo, 0, 256);

    //FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        printf("Error: could not open file %s", filename);
        return 1;
    }

    // reading line by line, max 256 bytes

    char Leido[MAX_LENGTH];

    while (fgets(Leido, MAX_LENGTH, fp))
    {
        strcat(cuerpo, Leido);
    }

    // close the file
    fclose(fp);

    email(IP, Destinatario, Emisor, Temita, cuerpo);


    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Operation done successfully\n");
    }
    sqlite3_close(db);
    fclose(fp);
    return 0;
}
