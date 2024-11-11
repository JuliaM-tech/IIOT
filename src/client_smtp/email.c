

// SPDX-License-Identifier: GPL-2.0-or-later
/* IBM POWER Barrier Synchronization Register Driver
 *
 * Copyright IBM Corporation 2008
 *
 * Author: Sonny Rao <sonnyrao@us.ibm.com>*/

 #ifdef HAVE_CONFIG_H
 #include <config.h>
 #endif

 #include "email.h"

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

void email(char *server_address, char *email_destinatari, char *email_remitent, char *tema_email, char *text_email)
{

    struct sockaddr_in serverAddr;
    int sockAddrSize;
    int sFd;
    // int mlen;
    int result;
    char buffer[256];
    char obertura[256] = "HELO host\n";
    char pas2[256] = "MAIL FROM: ";
    char pas3[256] = "RCPT TO: ";
    char DATA[256] = "DATA\n";
    char Subject[256] = "Subject: ";
    char From[256] = "From: ";
    char To[256] = "To: ";
    char Intro[] = "\n";
    char IntroF[] = "\n.\n";
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

    memset(buffer, 0, 256);
    //////////////////////////////////////////////

    /*Enviar*/

    strcpy(buffer, obertura); // Copiar obertura a buffer
    result = write(sFd, buffer, strlen(buffer));
    printf("Client(bytes %d): %s\n", result, obertura);

    memset(buffer, 0, 256);

    /*Rebre*/
    result = read(sFd, buffer, 256);
    printf("Servidor(bytes %d): %s\n", result, buffer);

    memset(buffer, 0, 256);

    /*Enviar*/
    strcat(pas2, email_remitent);
    strcat(pas2, Intro);
    strcpy(buffer, pas2); // Copiar missatge a buffer
    result = write(sFd, buffer, strlen(buffer));
    printf("Client(bytes %d): %s\n", result, pas2);

    memset(buffer, 0, 256);

    /*Rebre*/
    result = read(sFd, buffer, 256);
    printf("Servidor(bytes %d): %s\n", result, buffer);

    memset(buffer, 0, 256);

    /*Enviar*/
    strcat(pas3, email_destinatari);
    strcat(pas3, Intro);
    strcpy(buffer, pas3); // Copiar missatge a buffer
    result = write(sFd, buffer, strlen(buffer));
    printf("Client(bytes %d): %s\n", result, pas3);

    memset(buffer, 0, 256);

    /*Rebre*/
    result = read(sFd, buffer, 256);
    printf("Servidor(bytes %d): %s\n", result, buffer);

    memset(buffer, 0, 256);

    /*Enviar*/
    strcpy(buffer, DATA); // Copiar missatge a buffer
    result = write(sFd, buffer, strlen(buffer));
    printf("Client(bytes %d): %s\n", result, DATA);

    memset(buffer, 0, 256);

    /*Rebre*/
    result = read(sFd, buffer, 256);
    printf("Servidor(bytes %d): %s\n", result, buffer);

    memset(buffer, 0, 256);

    /*Enviar*/
    strcat(Subject, tema_email);
    strcat(Subject, Intro);
    strcpy(buffer, Subject); // Copiar missatge a buffer
    result = write(sFd, buffer, strlen(buffer));
    printf("Client(bytes %d): %s\n", result, Subject);

    memset(buffer, 0, 256);

    /*Enviar*/
    strcat(From, email_remitent);
    strcat(From, Intro);
    strcpy(buffer, From); // Copiar missatge a buffer
    result = write(sFd, buffer, strlen(buffer));
    printf("Client(bytes %d): %s\n", result, From);

    memset(buffer, 0, 256);

    /*Enviar*/
    strcat(To, email_destinatari);
    strcat(To, Intro);
    strcat(To, Intro);
    strcpy(buffer, To); // Copiar missatge a buffer
    result = write(sFd, buffer, strlen(buffer));
    printf("Client(bytes %d): %s\n", result, To);

    memset(buffer, 0, 256);

    /*Enviar*/
    strcat(text_email, IntroF);
    strcpy(buffer, text_email); // Copiar missatge a buffer
    result = write(sFd, buffer, strlen(buffer));
    printf("Client(bytes %d): %s\n", result, text_email);

    memset(buffer, 0, 256);

    /*Tancar el socket*/
    close(sFd);

    return 0;

    /*Tancar el socket*/
    close(sFd);
}
