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

#define REQUEST_MSG_SIZE 1024
#define REPLY_MSG_SIZE 500
#define SERVER_PORT_NUM 25

int email(char *server_address, char *email_destinatari, char *email_remitent, char *tema_email, char *text_email);

/************************
 *
 *
 * tcpClient
 *
 *
 */

int main(int argc, char *argv[])
{

    char IP[] = "172.20.0.21";
    char Emisor[] = "1632442@campus.euss.org";
    char Destinatario[] = "1598023@campus.euss.org";
    char Temita[] = "Lo logramos";
    char Texto[] = "La Concha de la lora , te recuerdo que me has de enviar un Bizum de 5 euros";

    email(IP, Destinatario, Emisor, Temita, Texto);

    return 0;
}

int email(char *server_address, char *email_destinatari, char *email_remitent, char *tema_email, char *text_email)
{

    struct sockaddr_in serverAddr;
    int sockAddrSize;
    int sFd;
    int mlen;
    int result;
    char buffer[256];
    char missatge[] = "#1";
    char missatge1[] = " HELO host\n "; // No em convenç aquesta solució, es podria fer per estructura
    char missatge6[] = " DATA \n";
    // char        missatge9 [] = " Hola Salvador\n.\n";

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

    /*Enviar*/

    strcpy(buffer, missatge); // Copiar missatge a buffer
    printf("%s\n",buffer);
    result = write(sFd, buffer, strlen(buffer));
    printf("Missatge enviat a servidor(bytes %d): %s\n", result, missatge);

    /*Rebre*/
    result = read(sFd, buffer, 256);
    printf(" %s\n", buffer);
    memset(buffer, 0, 256);

    //////////////////////////////////////////////////////////////////////

    /*Enviar*/
    strcpy(buffer, missatge1); // Copiar missatge a buffer
    printf("%s\n",buffer);
    result = write(sFd, buffer, strlen(buffer));
    memset(buffer, 0, 256);

    /*Rebre*/
    result = read(sFd, buffer, 256);
    printf(" %s\n", buffer);
    memset(buffer, 0, 256);

    //////////////////////////////////////////////////////////////////////

    /*Enviar*/

    sprintf(buffer, " MAIL FROM: %s\n ", email_remitent);
    printf("%s\n",buffer);
    result = write(sFd, buffer, strlen(buffer));
    memset(buffer, 0, 256);

    /*Rebre*/
    result = read(sFd, buffer, 256); // 250 Ok
    printf(" %s\n", buffer);
    memset(buffer, 0, 256);

    //////////////////////////////////////////////////////////////////////

    /*Enviar*/

    sprintf(buffer, " RCPT TO:  %s\n ", email_destinatari);
    printf("%s\n",buffer);
    result = write(sFd, buffer, strlen(buffer));
    memset(buffer, 0, 256);

    /*Rebre*/
    result = read(sFd, buffer, 256); // 250 Ok
    printf(" %s\n", buffer);
    memset(buffer, 0, 256);

    //////////////////////////////////////////////////////////////////////

    /*Enviar*/
    strcpy(buffer, missatge6); // Copiar missatge a buffer
    printf("%s\n",buffer);
    result = write(sFd, buffer, strlen(buffer));
    memset(buffer, 0, 256);


     /*Rebre*/
    result = read(sFd, buffer, 256); // 250 Ok
    printf(" %s\n", buffer);
    memset(buffer, 0, 256);

    /*Enviar*/

    sprintf(buffer, " Subject: %s\n ", tema_email);
    printf("%s\n",buffer);
    result = write(sFd, buffer, strlen(buffer));
    memset(buffer, 0, 256);

    //////////////////////////////////////////////////////////////////////

    /*Enviar*/

    sprintf(buffer, " From: %s\n ", email_remitent);
    printf("%s\n",buffer);
    result = write(sFd, buffer, strlen(buffer));
    memset(buffer, 0, 256);

    //////////////////////////////////////////////////////////////////////

    /*Enviar*/

    sprintf(buffer, " To: %s\n", email_destinatari);
    printf("%s\n",buffer);
    result = write(sFd, buffer, strlen(buffer));
    memset(buffer, 0, 256);


    //////////////////////////////////////////////////////////////////////

    sprintf(buffer,"%s\n.\n", text_email);
    printf("%s\n",buffer);
    result = write(sFd, buffer, strlen(buffer));
    memset(buffer, 0, 256);

    /*Rebre*/
    result = read(sFd, buffer, 256); // 250 Ok
    printf(" %s\n", buffer);

    /*Tancar el socket*/
    close(sFd);
}
