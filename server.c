#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <signal.h>
#include <pthread.h>

#include "hashTable.h"

#define SIZE 1024  // buffer size
#define PORT 2728  // port number
#define BACKLOG 10 // number of pending connections queue will hold

int serverSocket;
struct sockaddr_in serverAddr;

ht *clientTable;

void handleRequest(void *arg)
{
    int *clientFd = (int *)arg;

    char *request = (char *)malloc(SIZE * sizeof(char));

    read(*clientFd, request, SIZE);

    char method[10], info[100];

    sscanf(request, "%s %s", method, info);

    if (method == "Login")
    {
        login(request, clientFd);
    }
}

void runServer()
{
    while (1)
    {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int *clientFd = malloc(sizeof(int));

        if ((*clientFd = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0)
        {
            perror("accept failed");
            continue;
        }

        printf("%d\n", *clientFd);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handleRequest, (void *)clientFd);
        pthread_detach(thread_id);
    }
}

int main()
{
    ht *clientTable = ht_create();

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 10) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    char hostBuffer[10000], serviceBuffer[100000];
    int error = getnameinfo((struct sockaddr *)&serverAddr, sizeof(serverAddr), hostBuffer,
                            sizeof(hostBuffer), serviceBuffer, sizeof(serviceBuffer), 0);

    printf("\nServer is listening on http://%s:%s/\n\n", hostBuffer, serviceBuffer);

    runServer();

    ht_destroy(clientTable);

    return 0;
}
