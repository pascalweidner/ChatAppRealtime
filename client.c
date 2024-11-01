#include "includes.h"

#if defined(_WIN32)
#include <conio.h>
#endif

void initiateChat(SOCKET socket_peer)
{
    while (1)
    {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);

#if !defined(_WIN32)
        FD_SET(fileno(stdin), &reads);
#endif

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        if (select(socket_peer + 1, &reads, 0, 0, &timeout) < 0)
        {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            exit(1);
        }

        if (FD_ISSET(socket_peer, &reads))
        {
            char read[4096];
            int bytes_received = recv(socket_peer, read, 4096, 0);
            if (bytes_received < 1)
            {
                printf("Connection closed by peer.\n");
                break;
            }

            printf("%.*s", bytes_received, read);
        }

#if defined(_WIN32)
        if (_kbhit())
        {
#else
        if (FD_ISSET(0, &reads))
        {
#endif
            char read[1025];
            if (!fgets(read, 1025, stdin))
                break;
            int bytes_sent = send(socket_peer, read, strlen(read), 0);
        }
    }
}

int main()
{
#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d))
    {
        fprintf(stderr, "Failed to initialize.\n");
        return 1;
    }
#endif

    // Configuring remote address
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *peer_address;

    if (getaddrinfo("127.0.0.1", "8080", &hints, &peer_address))
    {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", errno);
        return 1;
    }

    // Creating socket
    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer))
    {
        fprintf(stderr, "socket() failed. (%d)\n", errno);
        return 1;
    }

    // connecting
    if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen))
    {
        fprintf(stderr, "connect() failed. (%d)\n", errno);
        return 1;
    }

    freeaddrinfo(peer_address);

    printf("Please enter a name: ");
    char read[100];
    if (!fgets(read, 100, stdin))
        return 1;
    char message[106];
    strcpy(message, "HELLO ");
    strcat(message, read);
    int bytes_sent = send(socket_peer, message, strlen(message), 0);

    initiateChat(socket_peer);
}