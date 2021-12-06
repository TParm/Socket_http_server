/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 57-7 */

/* ud_ucase_cl.c

   A UNIX domain client that communicates with the server in ud_ucase_sv.c.
   This client sends each command-line argument as a datagram to the server,
   and then displays the contents of the server's response datagram.
*/
#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>
#include "tlpi_hdr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h> // for getnameinfo()

// Usual socket headers
#include <sys/types.h>
#include <netinet/in.h>

#include <arpa/inet.h>

// GPIO Includes
#include "ud_ucase.h"
#include "PJ_RPI.h"

#define SV_SOCK_PATH "/tmp/ud_ucase"
#define BACKLOG 10

int main(void)
{
    struct sockaddr_in serverAddress, claddr;
    int serverSocket, j;
    size_t msgLen;
    ssize_t numBytes;
    char resp[BACKLOG];

    if (map_peripheral(&gpio) == -1)
    {
        printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        return -1;
    }

    /* Create client socket; bind to unique pathname (based on PID) */

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
        errExit("socket");

    memset(&claddr, 0, sizeof(struct sockaddr_in));
    claddr.sin_family = AF_INET;
    sprintf(claddr.sin_port, sizeof(claddr.sin_port),
             "/tmp/ud_ucase_cl.%d", (long)getpid());

    if (bind(serverSocket, (struct sockaddr *)&claddr, sizeof(struct sockaddr_in)) == -1)
        errExit("bind");

    /* Construct address of server */

    memset(&serverAddress, 0, sizeof(struct sockaddr_in));
    serverAddress.sin_family = AF_INET;
    strncpy(serverAddress.sin_port, SV_SOCK_PATH, sizeof(serverAddress.sin_port) - 1);

    /* Send messages to server; echo responses on stdout */

    t_data data;

    printf("Enter a GPIO pin: ");
    scanf("%d", &data.IO);
    printf("Gpio = %d\n", data.IO);

    printf("Enter the period to toggle the led (seconds): ");
    scanf("%d", &data.period);
    printf("Period = %d\n", data.period);

    printf("Enter number of toggles: ");
    scanf("%d", &data.toggle);
    printf("Toggle = %d\n", data.toggle);

    for (j = 1; j < 2; j++)
    {
        printf("Server received: \n GPIO_pin= %d\n period = %d \n toggle = %d\n\n respone: %p %p", data.IO, data.period, data.toggle, sizeof(data), &data, &(data.IO));

        if (sendto(serverSocket, &data, sizeof(data), 0, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) != sizeof(data))
            fatal("sendto");

        t_data response;
        numBytes = recvfrom(serverSocket, &response, sizeof(response), 0, NULL, NULL);
        /* Or equivalently: numBytes = recv(sfd, resp, BUF_SIZE, 0);
                        or: numBytes = read(sfd, resp, BUF_SIZE); */
        if (numBytes == -1)
            errExit("recvfrom");
        printf("Response %d: %d: %d\n", data.IO, data.period, data.toggle);
    }
    exit(EXIT_SUCCESS); /* Closes our socket; server sees EOF */
}
