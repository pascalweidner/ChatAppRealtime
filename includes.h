#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define SOCKET int

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
