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
    char ip[] = "127.0.0.1";
    //char ip[] = "::1";
    //char ip[] = "0.0.0.0";
    char port[] = "10001";

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // Allows both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM; // Defaults to TCP

    if((status = getaddrinfo(ip, port, &hints, &res)) != 0) // Returns a linked list with results about ip and port
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status)); // returns error if ip or port is unsupported
        return 2;
    }


    int client_socket; // used for socket descriptor
    client_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // socket() returns the socket descriptor
    status = connect(client_socket, res->ai_addr, res->ai_addrlen); // connects the client socket to a server, -1 if connection fails
    if(status == -1)
    {
        printf("Connection failed!\n");
        return -1;
    }

	char incoming[256];
    recv(client_socket, &incoming, sizeof(incoming), 0);
    printf("Server socket says: %s\n", incoming);

    //char input[256]; // Input that will be sent to the server
    //printf("Send a message to the server: ");
    //fgets(input, sizeof(input), stdin);
    //send(client_socket, input, strlen(input), res->ai_flags); // sends data from a connected socket

    int received_num_network_order;
    recv(client_socket, &received_num_network_order, sizeof(received_num_network_order), 0);
    int received_num = htonl(received_num_network_order); // Convert network byte order to host byte order
    printf("Received number from server: %d\n", received_num);

    int number = 10;
    int num_network_order = ntohl(number); // Convert integer to network byte order
    send(client_socket, &num_network_order, sizeof(num_network_order), res->ai_flags);

    int answer;

    for(int i = 0; i<10; i++)
    {
    int received_x_byte_stream;
    recv(client_socket, &received_x_byte_stream, sizeof(received_x_byte_stream), 0);
    int received_x = htonl(received_x_byte_stream); // Convert network byte order to host byte order

    int received_y_byte_stream;
    recv(client_socket, &received_y_byte_stream, sizeof(received_y_byte_stream), 0);
    int received_y = htonl(received_y_byte_stream); // Convert network byte order to host byte order

    printf("%d * %d = ? \n", received_x, received_y);

    scanf("%d", &answer);

    int answer_byte_stream = ntohl(answer);
    send(client_socket, &answer_byte_stream, sizeof(answer_byte_stream), res->ai_flags);

    char answer_response[256];
    recv(client_socket, &answer_response, sizeof(answer_response), 0);
    printf("%s\n", answer_response);

    send(client_socket, "ACK", sizeof("ACK"), res->ai_flags);


    }

    int received_grade_byte_stream;
    recv(client_socket, &received_grade_byte_stream, sizeof(received_grade_byte_stream), 0);
    int received_grade = htonl(received_grade_byte_stream); // Convert network byte order to host byte order

    printf("Your grade is %d/10\n", received_grade);

    freeaddrinfo(res); // frees up memory used by the getaddrinfo generated linked list
    close(client_socket); // closes socket


    return 0;
}
