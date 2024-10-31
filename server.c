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

struct user *handshake(SOCKET *socket_client) {
    struct user *usr = (struct user*)malloc(sizeof(struct user));
    usr->next = NULL;
    usr->socket_user = socket_client;

    //char *buffer = (char*)malloc(1025);
    char buffer[1025];
    memset(&buffer, 0, 1025);
    int rec = 0;
    char *p = buffer;

    while (1) {
        int bytes_received = recv(*socket_client, buffer, 1024, 0);
        if (bytes_received < 1) {
            close(*socket_client);
            free(usr);
            return NULL;
        }
        p += bytes_received;
        rec += bytes_received;
        buffer[rec] = '\0';

        char *q = strstr(buffer, "\n");
        if(q == NULL) {
            return NULL;
        }
        else {
            break;
        }
    }

    if(strncmp(buffer, "HELLO ", 6) != 0) {
        free(usr);
        return NULL;
    }

    char *name = buffer + 6;
    char *endName = strstr(name, "\n");
    if(endName == NULL) {
        free(usr);
        return NULL;
    }
    *endName = '\0';
    if(endName - name < 100) {     
        strcpy(usr->name, name);
    }
    else {
        free(usr);
        return NULL;
    }

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
    char buffer[1126];
    strcpy(buffer, name);
    strcat(buffer, ": ");
    strcat(buffer, message);


    struct user *p = head;
    while(p != NULL) {
        printf("name: %s\n", p->name);
        if(p->socket_user == socket_client) {
            p = p->next;
            continue;
        }

        int bytes_sent = send(*(p->socket_user), buffer, 1126, 0);
        printf("bytes_sent: %d\n", bytes_sent);
        p = p->next;
    }
}

void *handleClient(void *vargp) {
    SOCKET *socket_client = (SOCKET *)vargp;

    struct user *usr = handshake(socket_client);
    if(usr == NULL) {
        free(socket_client);
        return NULL;
    }

    printf("Name: %s\n", usr->name);

    while(1) {
        char buffer[1025];
        memset(&buffer, 0, 1025);
        int rec = 0;
        char *p = buffer, *q;
        int bytes_received = recv(*socket_client, p, 1024 - rec, 0);
        if(bytes_received < 1) {
            serveToRoom(socket_client, usr->name, "left the Room!");
            printf("Connection was closed by peer or error occured\n");
            goto cleanup;
        }

        p += bytes_received;
        rec += bytes_received;
        buffer[rec] = '\0';

        printf("buffer: %s\n", buffer);

        q = strstr(buffer, "\n");
        if(q == NULL) {
            break;
        }

        serveToRoom(socket_client, usr->name, buffer);
    }


cleanup:
    close(*socket_client);
    deleteUser(usr);
    free(socket_client);
}

int main() {
    // initialize user list
    head = NULL;
    end = NULL;

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
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    if(!ISVALIDSOCKET(socket)) {
        fprintf(stderr, "socket() failed. (%s)\n", strerror(errno));
        return 1;
    }

    int yes = 1;
    if(setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes)) <0) {
        fprintf(stderr, "setsockopt() failed. (%d)\n", errno);
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