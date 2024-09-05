#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#

#define SIZE 1024  // buffer size
#define PORT 2728  // port number
#define BACKLOG 10 // number of pending connections queue will hold
#define MAX_CLIENTS 4

int mastersockfd;
int activeconnections = 0;
struct sockaddr_in clientIPs[MAX_CLIENTS];
int connfds[MAX_CLIENTS];
struct sockaddr_in serverAddr;
int addrlen = sizeof(serverAddr);
char inBuffer[MAX_CLIENTS][1024] = {0};
char outBuffer[MAX_CLIENTS][1024] = {0};
char names[][];

void runServer()
{
    fd_set readfds;
    int max_fd, readyfds;

    printf("run server\n");
    while (1)
    {
        FD_ZERO(&readfds);

        // add mastersockfd
        FD_SET(mastersockfd, &readfds);
        max_fd = mastersockfd;

        for (int i = 0; i < activeconnections; i++)
        {
            if (connfds[i] != 0)
            {
                FD_SET(connfds[i], &readfds);
            }
            if (connfds[i] > max_fd)
            {
                max_fd = connfds[i];
            }
        }

        readyfds = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if ((readyfds < 0) && (errno != EINTR))
        {
            printf("select error\n");
        }

        if (FD_ISSET(mastersockfd, &readfds))
        {
            if ((connfds[activeconnections] = accept(mastersockfd, (struct sockaddr *)&clientIPs[activeconnections], (socklen_t *)&addrlen)) < 0)
            {
                perror("accept error...");
                exit(1);
            }

            fprintf(stdout, "New connection from %s\n", inet_ntoa(clientIPs[activeconnections].sin_addr));
            activeconnections++;
        }

        for (int i = 0; i < activeconnections; i++)
        {
            // check if connection is active and it is ready to read
            if (connfds[i] != 0 && FD_ISSET(connfds[i], &readfds))
            {
                // clear buffer
                memset(inBuffer[i], 0, 1024);
                memset(outBuffer[i], 0, 1024);
            }

            // TODO: make this mutlithreaded
            // read returns 0 if connection closed normally
            // and - 1 if error
            if (read(connfds[i], inBuffer[i], 1024) <= 0)
            {
                fprintf(stderr, "%s (code: %d)\n", strerror(errno), errno);
                strncpy(outBuffer[i], inet_ntoa(clientIPs[i].sin_addr), INET_ADDRSTRLEN);
                fprintf(stderr, "Host %s disconnected\n", outBuffer[i]);
                close(connfds[i]);
                connfds[i] = 0;
                continue;
            }

            // get client ip
            strncpy(outBuffer[i], inet_ntoa(clientIPs[i].sin_addr), INET_ADDRSTRLEN);

            printf("test");
            fprintf(stdout, "%s: %s", outBuffer[i], inBuffer[i]);

            strcat(outBuffer[i], " : ");
            strcat(outBuffer[i], inBuffer[i]);

            for (int j = 0; j < activeconnections; j++)
            {
                if (connfds[j] != 0 && i != j)
                {
                    write(connfds[j], outBuffer[i], strlen(outBuffer[i]));
                }
            }
        }
    }
}

int main()
{

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if ((mastersockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    setsockopt(mastersockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if (bind(mastersockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(mastersockfd, 3) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    runServer();

    return 0;
}
