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


#define REQUEST_MSG_SIZE	1024
#define REPLY_MSG_SIZE		500
#define SERVER_PORT_NUM		25



 /************************
*
*
* tcpClient
*
*
*/
int main(int argc, char *argv[]){
	struct sockaddr_in	serverAddr;
	char	    serverName[] = "172.20.0.21"; //Adreça IP on està el client
	int			sockAddrSize;
	int			sFd;
	//int			mlen;
	int 		result;
	char		buffer[256];
	char		obertura[] = "HELO juliaM\n";
	char		pas2[]="MAIL FROM:juliaM@gmail.com\n";
	char		pas3[]="RCPT TO:sbernadas@euss.cat\n";
	//char		remitent[] = "From: juliaM@gmail.com";
	//char		destinatari[] = "To: 1598023@campus.euss.org";
	char		DATA[]="DATA\n";
	char		Subject[] = "Subject: prova\n"; 
	char		From[] = "From: juliaM@gmail.com\n"; 
	char		To[] = "To: sbernadas@euss.cat\n"; 
	char		Dades[] = "Bon dia\n Es un exemple\n Júlia\n.\n";
	

	/*Crear el socket*/
	sFd=socket(AF_INET,SOCK_STREAM,0);

	/*Construir l'adreça*/
	sockAddrSize = sizeof(struct sockaddr_in);
	bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons (SERVER_PORT_NUM);
	serverAddr.sin_addr.s_addr = inet_addr(serverName);

	/*Conexió*/
	result = connect (sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
	if (result < 0)
	{
		printf("Error en establir la connexió\n");
		exit(-1);
	}
	printf("\nConnexió establerta amb el servidor: adreça %s, port %d\n",	inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

	/*Rebre*/
	result = read(sFd, buffer, 256);
	printf("Servidor(bytes %d): %s\n", result, buffer);
	
	memset(buffer,0,256);
	
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
	strcpy(buffer,pas2); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, pas2);

	memset(buffer,0,256);
	
	/*Rebre*/
	result = read(sFd, buffer, 256);
	printf("Servidor(bytes %d): %s\n",	result, buffer);
	
	memset(buffer,0,256);
	
	/*Enviar*/
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
	strcpy(buffer,Subject); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, Subject);

	memset(buffer,0,256);
	
	/*Enviar*/
	strcpy(buffer,From); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, From);

	memset(buffer,0,256);
	
	/*Enviar*/
	strcpy(buffer,To); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, To);

	memset(buffer,0,256);
	
	/*Enviar*/
	strcpy(buffer,Dades); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	printf("Client(bytes %d): %s\n",	result, Dades);

	memset(buffer,0,256);

	/*Tancar el socket*/
	close(sFd);

	return 0;
	}
