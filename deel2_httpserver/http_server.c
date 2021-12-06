#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h> // for getnameinfo()

// Usual socket headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>

// GPIO Includes
#include "ud_ucase.h"
#include "PJ_RPI.h"

#define SV_SOCK_PATH "/tmp/ud_ucase"
#define SIZE 1024
#define BACKLOG 10 // Passed to listen()

void report(struct sockaddr_in *serverAddress);

void setHttpHeader(char httpHeader[])
{
    // File object to return
    FILE *htmlData = fopen("../index.html", "r");

    char line[100];
    char responseData[8000];
    while (fgets(line, 100, htmlData) != 0)
    {
        strcat(responseData, line);
    }
    // char httpHeader[8000] = "HTTP/1.1 200 OK\r\n\n";
    strcat(httpHeader, responseData);
}

int main(void)
{
    char httpHeader[8000] = "HTTP/1.1 200 OK\r\n\n";

    // NEW Variables gpio
    int j;
    ssize_t numBytes;
    socklen_t len;
    char buf[BUF_SIZE];

    // Socket setup: creates an endpoint for communication, returns a descriptor
    // -----------------------------------------------------------------------------------------------------------------
    int serverSocket = socket(
        AF_INET,     // Domain: specifies protocol family
        SOCK_STREAM, // Type: specifies communication semantics
        0            // Protocol: 0 because there is a single protocol for the specified family
    );

    // Construct local address structure
    // -----------------------------------------------------------------------------------------------------------------
    struct sockaddr_in serverAddress, claddr;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8001);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // inet_addr("127.0.0.1");

    // Bind socket to local address
    // -----------------------------------------------------------------------------------------------------------------
    // bind() assigns the address specified by serverAddress to the socket
    // referred to by the file descriptor serverSocket.
    bind(
        serverSocket,                      // file descriptor referring to a socket
        (struct sockaddr *)&serverAddress, // Address to be assigned to the socket
        sizeof(serverAddress)              // Size (bytes) of the address structure
    );

    // Mark socket to listen for incoming connections
    // -----------------------------------------------------------------------------------------------------------------
    int listening = listen(serverSocket, BACKLOG);
    if (listening < 0)
    {
        printf("Error: The server is not listening.\n");
        return 1;
    }
    printf("test");
    report(&serverAddress);    // Custom report function
    setHttpHeader(httpHeader); // Custom function to set header
    int clientSocket;

    // Wait for a connection, create a connected socket if a connection is pending
    // -----------------------------------------------------------------------------------------------------------------
    while (1)
    {
        clientSocket = accept(serverSocket, NULL, NULL);
        send(clientSocket, httpHeader, sizeof(httpHeader), 0);
        printf("client");
        // for (;;)
        // {
        //     t_data received_data;
        //     len = sizeof(struct sockaddr_un);
        //     numBytes = recvfrom(serverSocket, &received_data, sizeof(received_data), 0,
        //                         (struct sockaddr *)&clientSocket, &len);
        //     if (numBytes == -1)
        //         errExit("recvfrom");

        //     printf("Server received %zd bytes from %s\n", numBytes, claddr.sin_port);
        //     /*FIXME: above: should use %zd here, and remove (long) cast */

        //     for (j = 0; j < numBytes; j++)
        //         buf[j] = toupper((unsigned char)buf[j]);

        //     for (size_t i = 0; i < received_data.toggle; i++)
        //     {
        //         // Toggle (blink a led!)
        //         GPIO_SET = 1 << received_data.IO;
        //         printf("gpio: %d is aan\n", received_data.IO);
        //         sleep(received_data.period);

        //         GPIO_CLR = 1 << received_data.IO;
        //         printf("gpio: %d is uit\n", received_data.IO);
        //         sleep(received_data.period);
        //     }
        //     printf("Waiting for new data from client.\n");

        //     if (sendto(serverSocket, &received_data, sizeof(received_data), 0, (struct sockaddr *)&clientSocket, len) !=
        //         numBytes)
        //         fatal("Client side is not available! Not able to sendto\n\n");
        // }
        close(clientSocket);
    }
    return 0;
}

void report(struct sockaddr_in *serverAddress)
{
    char hostBuffer[INET6_ADDRSTRLEN];
    char serviceBuffer[NI_MAXSERV]; // defined in <netdb.h>
    socklen_t addr_len = sizeof(*serverAddress);
    int err = getnameinfo(
        (struct sockaddr *)serverAddress,
        addr_len,
        hostBuffer,
        sizeof(hostBuffer),
        serviceBuffer,
        sizeof(serviceBuffer),
        NI_NUMERICHOST);
    if (err != 0)
    {
        printf("It's not working!!\n");
    }
    printf("\n\n\tServer listening on http://%s:%s\n", hostBuffer, serviceBuffer);
}
