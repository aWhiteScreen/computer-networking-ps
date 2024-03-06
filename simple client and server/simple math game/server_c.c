#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>

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
    //char ip[] = "::1";
    //char *ip = "0.0.0.0";
    char port[] = "10001";

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((status = getaddrinfo(NULL, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    int server_socket;
    server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int opt = 0;
    setsockopt(server_socket, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&opt, sizeof(opt));


    status = bind(server_socket, res->ai_addr, res->ai_addrlen); // binds server to specific port
    if(status != 0)
        printf("Error binding!\n");


    status = listen(server_socket, 5); // Listens for sockets trying to connect to it
    if(status != 0)
        printf("Error listening!\n");

    int client_socket; // used for accepting connections

    client_socket = accept(server_socket, NULL, NULL);
    if(client_socket == -1)
        printf("Error accepting connection!\n");

    char server_message[256] = "Connection established! Prepare to solve some math equations!\n";
    send(client_socket, server_message, sizeof(server_message), res->ai_flags);

    int number = 10;
    int num_network_order = htonl(number); // Convert integer to network byte order
    send(client_socket, &num_network_order, sizeof(num_network_order), res->ai_flags);

    int received_num_network_order;
    recv(client_socket, &received_num_network_order, sizeof(received_num_network_order), 0);
    int received_num = ntohl(received_num_network_order); // Convert network byte order to host byte order
    printf("Received number from server: %d\n", received_num);


    time_t t;
    srand(time(&t));
    int grade = 0;
    char good_answer[] = "Correct";
    char bad_answer[] = "Incorrect";
    int sent_questions[10] = {0}; // Array to keep track of sent questions
    for (int i = 0; i < 10; i++) {
        int x, y;
        do {
            x = rand() % 10 + 1; // Generate random number between 1 and 10
            y = rand() % 10 + 1;
        } while (sent_questions[x * y] == 1); // Check if this question has been sent before
        sent_questions[x * y] = 1; // Mark question as sent
        int expected_answer = x*y;
        printf("expected answer %d", expected_answer);
        // Send question to client
        int send_x = htonl(x);
        send(client_socket, &send_x, sizeof(send_x), res->ai_flags);
        int send_y = htonl(y);
        send(client_socket, &send_y, sizeof(send_y), res->ai_flags);

        int received_answer_byte_stream;
        recv(client_socket, &received_answer_byte_stream, sizeof(received_answer_byte_stream), 0);
        int received_answer = ntohl(received_answer_byte_stream); // Convert network byte order to host byte order

        printf("Received answer for equation: %d\n", received_answer);
        if(received_answer == expected_answer)
        {
            send(client_socket, good_answer, sizeof(good_answer), res->ai_flags);
            grade++;
        }
        else if(received_answer != expected_answer)
            send(client_socket, bad_answer, sizeof(bad_answer), res->ai_flags);

        char ack[256];
        recv(client_socket, ack, sizeof(ack), 0);

    }

    int send_grade = htonl(grade);
    send(client_socket, &send_grade, sizeof(send_grade), res->ai_flags);

    freeaddrinfo(res);
    close(server_socket);

    return 0;
}
