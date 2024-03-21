#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <poll.h>

#include<sys/types.h>
#include<sys/socket.h>

#include<netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


int clientCount = 0;
int passed_test = 0;
int can_connect = 1;
int clientSockets[256];
char clientNames[256][256];
int x[10];
int y[10];
pthread_barrier_t barrier; // will be used for waiting for all threads to reach a certain point before continuing
int countdown = 30;

void *handle_client(void *socketDesc); // handles connection between connections
void broadcast(char *msg, int len); // sends a message out to all clients
int name_checker(char *name); // checks if name is in use

int main()
{
    

    int problem[10] = {0};
    time_t t;
    srand(time(&t));

    for(int i = 0; i<10; i++)
    {
        do {
        x[i] = rand() % 10 + 1;
        y[i] = rand() % 10 + 1;
        }while (problem[x[i]*y[i]] == 1);
        problem[x[i]*y[i]] = 1;
        printf("%d * %d \n", x[i], y[i]);
    }

    int status;
    struct addrinfo hints, *res;
    //char *ip = "127.0.0.1";
    //char ip[] = "::1";
    //char *ip = "0.0.0.0";
    char port[] = "40000";

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
    else printf("Binding successful\n");

    status = listen(server_socket, 5); // Listens for sockets trying to connect to it
    if(status != 0)
        printf("Error listening!\n");
    else printf("Listening for Connection\n");


    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        if (clientCount < 256) {
            // Allocate a new socket descriptor for the thread to avoid race conditions
            int* newSockPtr = malloc(sizeof(int));
            *newSockPtr = client_socket;

            pthread_t threadID;
            pthread_create(&threadID, NULL, handle_client, (void *)newSockPtr);

        } else {
            printf("Max clients reached. Connection refused: ");
            close(client_socket);
        }
    }


    return 0;

}

void *handle_client(void *socketDesc) {

    int grade = 0;

    int sock = *(int *)socketDesc;
    free(socketDesc); 

    char name[256] = {0};
    char recvBuf[256];
    char sendBuf[512];
    bool isNameAccepted = false;

    // Prompt the client for a username
    send(sock, "ATSIUSKVARDA\n", strlen("ATSIUSKVARDA\n"), 0);

    // Initially request a name from the client until a unique one is provided
    while (!isNameAccepted) 
    {
        memset(name, 0, 256);
        int bytesReceived = recv(sock, name, 256 - 1, 0);
        if (bytesReceived <= 0) 
        {
            perror("Failed to get name from client");
            close(sock);
            return NULL;
        }
        name[bytesReceived] = '\0'; // Null-terminate the string
        name[strcspn(name, "\r\n")] = 0; // Remove any newline or carriage return

        if (!name_checker(name) && can_connect == 1) 
        { 
            strncpy(clientNames[clientCount], name, 256 - 1); 
            clientSockets[clientCount++] = sock; 
            pthread_barrier_init(&barrier, NULL, clientCount);
            isNameAccepted = true;
            send(sock, "VARDASOK\n", strlen("VARDASOK\n"), 0); 
        } 
        else 
        {
            send(sock, "NEGALIMAS\n", strlen("NEGALIMAS\n"), 0); 
            close(sock);
            return NULL; 
        }
    }


    snprintf(sendBuf, sizeof(sendBuf), "PRANESIMAS: Waiting 30 seconds for aditional users to connect! \n");
    send(sock, sendBuf, strlen(sendBuf), 0);

    while(countdown>0)
    {
        snprintf(sendBuf, sizeof(sendBuf), "PRANESIMAS: Test is starting in %d! \n", countdown);
        send(sock, sendBuf, strlen(sendBuf), 0);
        countdown--;
        sleep(1);   
    }

    can_connect = 0;

    pthread_barrier_wait(&barrier); // Wait for all clients to reach this point

    snprintf(sendBuf, sizeof(sendBuf), "PRANESIMAS: Test starting, good luck! \n");
    send(sock, sendBuf, strlen(sendBuf), 0);

    int problems[10];
    int problems_check[10] = {0};

    for(int i = 0; i<10; i++)
    {
        do {
        problems[i] = rand() % 10;
        }while (problems_check[problems[i]] == 1);
        problems_check[problems[i]] = 1;
    }

    printf("number of clients: %d\n", clientCount);


    fd_set readfds;
    struct timeval timeout;
    int maxfd = sock + 1;


    for(int i = 0; i < 10; i++) {
        int j = problems[i];
        snprintf(sendBuf, sizeof(sendBuf), "PRANESIMAS: %d * %d = ?\n", x[j], y[j]);
        send(sock, sendBuf, strlen(sendBuf), 0);

        // Clear recvBuf before receiving new data
        memset(recvBuf, 0, sizeof(recvBuf));

        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        timeout.tv_sec = 10; // 10 seconds timeout
        timeout.tv_usec = 0;

        // Use select to wait for data or timeout
        int selectResult = select(maxfd, &readfds, NULL, NULL, &timeout);
        if (selectResult < 0)
        {
            perror("Select error");
            close(sock);
            return NULL;
        }
        else if (selectResult == 0)
        {
            printf("Receive timeout. Moving on...\n");
        }
        else
        {
            // Receive data and ensure null-termination
            int bytesReceived = recv(sock, recvBuf, sizeof(recvBuf) - 1, 0);
            if (bytesReceived <= 0) {
                printf("Error\n");
            }

            // Null-terminate the received data
            recvBuf[bytesReceived] = '\0';

            int num = atoi(recvBuf);

            if (num == (x[j] * y[j]))
            {
                grade++;
                printf("The sent in answer was correct! \n");
            }
            else
                printf("sent in asnwer was incorrect\n");

            printf("received number is %d\n", num);
        }
    }

    snprintf(sendBuf, sizeof(sendBuf), "PRANESIMAS: Grade: %d/10! \n", grade);
    send(sock, sendBuf, strlen(sendBuf), 0);

    if(grade>=4)
    {
        snprintf(sendBuf, sizeof(sendBuf), "PRANESIMAS: You have passed the test!\n");
        send(sock, sendBuf, strlen(sendBuf), 0);
        passed_test++;
    } 
    else 
    {
        snprintf(sendBuf, sizeof(sendBuf), "PRANESIMAS: You have failed the test!\n");
        send(sock, sendBuf, strlen(sendBuf), 0);
    }

    snprintf(sendBuf, sizeof(sendBuf), "PRANESIMAS: Waiting for all users to finish! Number of clients: %d\n", clientCount);
    send(sock, sendBuf, strlen(sendBuf), 0);

    pthread_barrier_wait(&barrier);

    snprintf(sendBuf, sizeof(sendBuf), "PRANESIMAS: Number of clients: %d\n", clientCount);
    send(sock, sendBuf, strlen(sendBuf), 0);

    snprintf(sendBuf, sizeof(sendBuf), "PRANESIMAS: The test was passed by %d/ %d\n", passed_test, clientCount);
    send(sock, sendBuf, strlen(sendBuf), 0);

    close(sock);
    return NULL;
}

void broadcast(char *msg, int len) 
{
    for (int i = 0; i < clientCount; i++) 
    {
        if (send(clientSockets[i], msg, len, 0) < 0) {
            perror("Error sending message");
            continue;
        }
    }
}

int name_checker(char *name) 
{
    for (int i = 0; i < clientCount; i++) 
    {
        if (strcmp(clientNames[i], name) == 0) return 1;
    }
    return 0;
}

