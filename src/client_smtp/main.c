
// SPDX-License-Identifier: GPL-2.0-or-later
/* IBM POWER Barrier Synchronization Register Driver
 *
 * Copyright IBM Corporation 2008
 *
 * Author: Sonny Rao <sonnyrao@us.ibm.com>*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "email.h"
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



void print_usage(void)
{
    printf("Usage: --servidor correuss.euss.cat --origen alumne@euss.cat --desti professor@euss.cat --tema “tema del mail” --fitxer /home/pi/text_email.txt");
}

void main(int argc, char *argv[])
{
    const unsigned int MAX_LENGTH = 256;
    // char texto [MAX_LENGTH][MAX_LENGTH];
    char IP[MAX_LENGTH];
    char Emisor[MAX_LENGTH];
    char Destinatario[MAX_LENGTH];
    char Temita[MAX_LENGTH];
    char cuerpo[MAX_LENGTH];
    char filename[MAX_LENGTH];
    int long_index = 0;

    int opt = 0;

    static struct option long_options[] = {
        {"servidor", required_argument, 0, 's'},
        {"origen", required_argument, 0, 'o'},
        {"desti", required_argument, 0, 'd'},
        {"tema", required_argument, 0, 't'},
        {"texte", required_argument, 0, 'x'}};

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
        strcat(cuerpo, Leido);
    }

    // close the file
    fclose(fp);

    email(IP, Destinatario, Emisor, Temita, cuerpo);

    return 0;
}
