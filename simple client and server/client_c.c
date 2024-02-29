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
    struct addrinfo hints, *res, *p; // structs for getaddrinfo, p is for iterating through the results list
    int status; // used to check connection status
    //char ip[] = "127.0.0.1";
    char ip[] = "::1";
    //char ip[] = "0.0.0.0";
    char port[] = "20000";
    char *input; // Input that will be sent to the server

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // Allows both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM; // Defaults to TCP

    if(status = getaddrinfo(ip, port, &hints, &res) != 0) // Returns a linked list with results about ip and port
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status)); // returns error if ip or port is unsupported
        return 2;
    }

    for (p = res; p != NULL; p = p->ai_next) { // prints out some gettaddrinfo results
        void *addr;
        char ipstr[INET6_ADDRSTRLEN];

        if (p->ai_family == AF_INET) { // checks if ip is IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else if(p->ai_family == AF_INET6){ // checks if ip is IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr); // Converts ip into string

        printf("Family: %d, Socket Type: %d, Protocol %d, Canon name: %s,  IP address: %s, Port: %d\n", p->ai_family, p->ai_socktype, p->ai_protocol, p->ai_canonname, ipstr, ntohs(((struct sockaddr_in *)p->ai_addr)->sin_port));
    }

    int client_socket; // used for socket descriptor
    client_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // socket() returns the socket descriptor
    status = connect(client_socket, res->ai_addr, res->ai_addrlen); // connects the client socket to a server, -1 if connection fails
    if(status == -1)
    {
        printf("Connection failed!\n");
        return -1;
    }

    printf("Send a message to the server: ");
    scanf("%s", &input);
    send(client_socket, input, strlen(input), res->ai_flags); // sends data from a connected socket

    freeaddrinfo(res); // frees up memory used by the getaddrinfo generated linked list
    close(client_socket); // closes socket


    return 0;
}
