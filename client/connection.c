#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int connection_init(const char *port, const char *address) {
    struct sockaddr_in sa;
    int client_socket;
    if ((client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);

    sa.sin_addr.s_addr = inet_addr(address);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(strtol(port, NULL, 10));

    if (connect(client_socket, (struct sockaddr *)&sa, sizeof sa) == -1) {
        perror("Connect failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connection accepted %d\n", client_socket);
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(client_socket, (struct sockaddr *)&sin, &len) == -1)
        perror("getsockname");
    else
        printf("port number %d\n", ntohs(sin.sin_port));

    return client_socket;
}
