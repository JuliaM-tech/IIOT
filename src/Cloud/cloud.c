/***************************************************************************
                          main.c  -  client
    copyright            : (C) 2024 by A.Fontquerni & S.Bernadas
    email                : afontquerni@euss.cat / sbernadas@euss.cat
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
#include "cloud.h"


#define REQUEST_MSG_SIZE	1024*256
#define SERVER_PORT_NUM		80



/************************
*
*
* tcpClient
*
*
*/
void cloud(char *id_sensor, char *valor){
	
	struct sockaddr_in	serverAddr;
	char	    serverName[] = "192.168.11.249"; //Adreça IP on està el client
	int			sockAddrSize;
	int			sFd;
	int 		result;
	char		buffer[REQUEST_MSG_SIZE];
	char		missatge[] ="GET http://iotlab.euss.cat/cloud/guardar_dades.php?id_sensor=%s&valor=%s&temps= HTTP/1.0\r\n\r\n";
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
	//printf("\nConnexió establerta amb el servidor: adreça %s, port %d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

	/*Enviar*/
	memset(buffer, 0, REQUEST_MSG_SIZE);
	sprintf(buffer,missatge, id_sensor, valor); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	//printf("Missatge enviat a servidor(bytes %d): \n{%s}\n\n",	result, buffer);

	/*Rebre*/
	memset(buffer, 0, REQUEST_MSG_SIZE);
	result = read(sFd, buffer, REQUEST_MSG_SIZE);
	//printf("Missatge rebut del servidor(bytes %d): \n{%s}\n",	result, buffer);

	/*Tancar el socket*/
	close(sFd);

	}

