#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/types.h>
#include<sys/socket.h>

#include<netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int main()
{
    int status;
    struct addrinfo hints, *res, *p;
    //char *ip = "127.0.0.1";
    char *ip = "::1";
    //char *ip = "0.0.0.0";
    char *port = "20000";

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((status = getaddrinfo(ip, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        void *addr;
        char ipstr[INET6_ADDRSTRLEN];

        if(p->ai_family = AF_INET)
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        }
        if(p->ai_family = AF_INET6)
        {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }

        inet_ntop(p->ai_family,addr, ipstr, sizeof ipstr);

        printf("Family: %d, Socket Type: %d, Protocol: %d, Canon Name: %s, IP: %s, Port: %d \n", p->ai_family, p->ai_socktype, p->ai_protocol, p->ai_canonname, ipstr, ntohs(((struct sockaddr_in *)p->ai_addr)->sin_port));
    }

    int server_socket;
    server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    status = bind(server_socket, res->ai_addr, res->ai_addrlen);


    listen(server_socket, 1); // Listens for sockets trying to connect to it

    int client_socket; // used for accepting connections

    client_socket = accept(server_socket, NULL, NULL);

    char server_message[256] = "Welcome to the server socket! Hello, world\n";
    send(client_socket, server_message, sizeof(server_message), NULL);

	char incoming[256];
    recv(client_socket, &incoming, sizeof(incoming), 0);
    printf("Client socket says: %s\n", incoming);
    send(client_socket, strupr(incoming), sizeof(incoming), NULL);

    freeaddrinfo(res);
    close(server_socket);

    return 0;
}
