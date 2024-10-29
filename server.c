#include "includes.h"

struct user;

struct user {
    char name[100];
    SOCKET *socket_user;
    struct user *next;
};

static struct user *head;
static struct user *end;

struct user* getUser(SOCKET *socket_client) {

}

void deleteUser(struct user *usr) {
    
}

char *handshake(SOCKET *socket_client) {
    struct user *usr = (struct user*)malloc(sizeof(struct user));
    usr->next = NULL;
    usr->socket_user = socket_client;

    char buffer[1025];
    int rec = 0;
    char *p = buffer;
    char *q;

    while (1) {
        int bytes_received = recv(*socket_client, p, 1024 - rec, 0);
        if (bytes_received < 1) {
            close(*socket_client);
            free(usr);
            return NULL;
        }
        p += bytes_received;
        rec += bytes_received;
        buffer[rec + 1] = '\0';

        if((q = strstr("\r\n\r\n"))) {
            break;
        }
    }

    if(!strncmp(buffer, "HELLO ", 6))) {
        free(usr);
        return NULL;
    }

    char *name = buffer + 6;
    char *endName = strstr(name, "\r\n");
    *endName = '\0';
    strcpy(usr->name, name);

    // TODO: make it thread safe
    if(head == NULL) {
        head = usr;
        end = usr;
    }
    else {
        end->next = usr;
        end = usr;
    }

    return usr;
}

void serveToRoom(SOCKET *socket_client, char *name, char *message) {

}

void *handleClient(void *vargp) {
    SOCKET *socket_client = (SOCKET *)vargp;

    struct user *usr = handshake(socket_client);
    if(usr == NULL) {
        free(socket_client);
        return;
    }

    char buffer[1025];
    int rec = 0;
    char *p = buffer, *q;

    while(1) {
        int bytes_received = recv(*socket_client, p, 1024 - rec, 0);
        if(bytes_received < 1) {
            printf("Connection was closed by peer or error occured\n");
            goto cleanup
        }

        p += bytes_received;
        rec += bytes_received;
        buffer[rec + 1] = '\0';

        if((q = strstr("\r\n"))) {
            break;
        }
    }

    serveToRoom(socket_client, usr->name, buffer);


cleanup:
    close(socket_client);
    deleteUser(usr);
    free(socket_client);
}

int main() {
    // initialize user list
    head = NULL;
    end = NULL

    // Create local address
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, "8080", &hints, &bind_address);

    // Create listening socket
    SOCKET socket_listen;
    socket_Listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    if(!ISVALIDSOCKET(socket)) {
        fprintf(stderr, "socket() failed. (%s)\n", strerror(errno));
        return 1;
    }

    // Bind socket
    if(bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%s)\n", strerror(errno));
        return 1;
    }

    freeaddrinfo(bind_address);

    // Listening
    if(listen(socket_listen, 20) < 0) {
        fprintf(stderr, "listen() failed. (%s)\n", strerror(errno));
        return 1;
    }

    // Waiting for connections
    while(1) {
        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);
        SOCKET *socket_client = (SOCKET *)malloc(sizeof(SOCKET));
        *socket_client = accept(socket_listen, (struct sockaddr *)&client_address, &client_len);
        if(!ISVALIDSOCKET(*socket_client)) {
            fprintf(stderr, "socket() failed. (%s)\n", strerror(errno));
            continue;
        }

        char address_buffer[100];
        getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
        printf("New connection from %s\n", address_buffer);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handleClient, (void *)socket_client);
        pthread_detach(thread_id);
    }

}