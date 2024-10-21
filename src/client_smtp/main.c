/***************************************************************************
                          main.c  -  client
                             -------------------
    begin                : lun feb  4 16:00:04 CET 2002
    copyright            : (C) 2002 by A. Moreno
    email                : amoreno@euss.es
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#define REQUEST_MSG_SIZE 1024
#define REPLY_MSG_SIZE 500
#define SERVER_PORT_NUM 25

int email(char *server_address, char *email_destinatari, char *email_remitent, char *tema_email, char *text_email);

void print_usage() {
	printf("Usage: --servidor correuss.euss.cat --origen alumne@euss.cat --desti professor@euss.cat --tema “tema del mail” --fitxer /home/pi/text_email.txt");
}


int main(int argc, char *argv[])
{
    const unsigned MAX_LENGTH = 256;
    //char texto [MAX_LENGTH][MAX_LENGTH];
    char IP[MAX_LENGTH];
    char Emisor[MAX_LENGTH];
    char Destinatario[MAX_LENGTH];
    char Temita[MAX_LENGTH];
    char cuerpo[MAX_LENGTH];
	char filename[MAX_LENGTH];
	int long_index =0;
	
	int opt=0;
	
    
    static struct option long_options[]= {
		{"servidor", required_argument, 0, 's'},
		{"origen", required_argument, 0, 'o'},
		{"desti", required_argument, 0, 'd'},
		{"tema", required_argument, 0, 't'},
		{"texte", required_argument, 0, 'x'}	
	};
	
	
	while ((opt = getopt_long(argc, argv,"t:s:o:d:x:",long_options, &long_index )) != -1) {
		switch (opt) {
			case 't' : 
			strcpy(Temita, optarg);
			break;
			case 's' :
			strcpy(IP, optarg);
			break;
			case 'o' :
			strcpy(Emisor, optarg);
			break;
			case 'd' :
			strcpy(Destinatario, optarg);
			break;
			case 'x' :
			strcpy(filename, optarg);
			break;
			default: print_usage();
			exit(EXIT_FAILURE);
		}
	}

    memset(cuerpo, 0, 256);

    
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        printf("Error: could not open file %s", filename);
        return 1;
    }

    // reading line by line, max 256 bytes
 
    char Leido[MAX_LENGTH];

   while (fgets(Leido, MAX_LENGTH, fp))
    {
     strcat(cuerpo,Leido);
    }


    // close the file
    fclose(fp);

    email(IP, Destinatario, Emisor, Temita, cuerpo);

    return 0;
}

int email(char *server_address, char *email_destinatari, char *email_remitent, char *tema_email, char *text_email) 
{

    struct sockaddr_in serverAddr;
    int sockAddrSize;
    int sFd;
    //int mlen;
    int result;
    char buffer[256];
	char		obertura[256] = "HELO host\n";
	char		pas2[256]="MAIL FROM: ";
	char		pas3[256]="RCPT TO: ";
	char		DATA[256]="DATA\n";
	char		Subject[256] = "Subject: "; 
	char		From[256] = "From: "; 
	char		To[256] = "To: "; 
	char        Intro[]="\n";
	char        IntroF[]="\n.\n";
    /*Crear el socket*/
    sFd = socket(AF_INET, SOCK_STREAM, 0);

    /*Construir l'adreça*/
    sockAddrSize = sizeof(struct sockaddr_in);
    bzero((char *)&serverAddr, sockAddrSize); // Posar l'estructura a zero
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT_NUM);
    serverAddr.sin_addr.s_addr = inet_addr(server_address);

    /*Conexió*/
    result = connect(sFd, (struct sockaddr *)&serverAddr, sockAddrSize);
    if (result < 0)
    {
        printf("Error en establir la connexió\n");
        exit(-1);
    }
    printf("\nConnexió establerta amb el servidor: adreça %s, port %d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

  
   /*Rebre*/
	result = read(sFd, buffer, 256);
	printf("Servidor(bytes %d): %s\n", result, buffer);
	
	memset(buffer,0,256);
	//////////////////////////////////////////////
	
	/*Enviar*/
	
	strcpy(buffer,obertura); //Copiar obertura a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n", result, obertura);

	memset(buffer,0,256);
	
	/*Rebre*/
	result = read(sFd, buffer, 256);
	printf("Servidor(bytes %d): %s\n",	result, buffer);
	
	memset(buffer,0,256);
	
	/*Enviar*/
	strcat(pas2, email_remitent);
	strcat(pas2, Intro);
	strcpy(buffer,pas2); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, pas2);

	memset(buffer,0,256);
	
	/*Rebre*/
	result = read(sFd, buffer, 256);
	printf("Servidor(bytes %d): %s\n",	result, buffer);
	
	memset(buffer,0,256);
	
	/*Enviar*/
	strcat(pas3, email_destinatari);
	strcat(pas3, Intro);
	strcpy(buffer,pas3); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, pas3);

	memset(buffer,0,256);
	
	/*Rebre*/
	result = read(sFd, buffer, 256);
	printf("Servidor(bytes %d): %s\n",	result, buffer);
	
	memset(buffer,0,256);
	
	/*Enviar*/
	strcpy(buffer,DATA); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, DATA);

	memset(buffer,0,256);
	
	/*Rebre*/
	result = read(sFd, buffer, 256);
	printf("Servidor(bytes %d): %s\n",	result, buffer);
	
	memset(buffer,0,256);
	
	/*Enviar*/
	strcat(Subject, tema_email);
	strcat(Subject, Intro);
	strcpy(buffer,Subject); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, Subject);

	memset(buffer,0,256);
	
	/*Enviar*/
	strcat(From, email_remitent);
	strcat(From, Intro);
	strcpy(buffer,From); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, From);

	memset(buffer,0,256);
	
	/*Enviar*/
	strcat(To, email_destinatari);
	strcat(To, Intro);
	strcat(To, Intro);	
	strcpy(buffer,To); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, To);

	memset(buffer,0,256);
	
	/*Enviar*/
	strcat(text_email, IntroF);
	strcpy(buffer,text_email); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, text_email);

	memset(buffer,0,256);

	/*Tancar el socket*/
	close(sFd);

	return 0;

    /*Tancar el socket*/
    close(sFd);
    
}
