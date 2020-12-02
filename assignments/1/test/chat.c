/*
** server.c -- a stream socket server demo
*/

#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ifaddrs.h>
#include <net/if.h>

#define PORT 3490  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold

#define MAXDATASIZE 141 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void getInput(char *question, char *inputBuffer) {
    printf("%s", question);
    fgets(inputBuffer, MAXDATASIZE, stdin);

    if (inputBuffer[strlen(inputBuffer) -1] != '\n')
    {
        int dropped = 0;
        while (fgetc(stdin) != '\n')
            dropped++;

        if (dropped > 0) // if they input exactly (bufferLength - 1) 
                         // characters, there's only the \n to chop off
        {
            printf("Error: Input too long.\n");
            getInput(question, inputBuffer);
        }
    }
    else
    {
        inputBuffer[strlen(inputBuffer) -1] = '\0';      
    }
}



int client(char* port, char *hostname) {
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];


    printf("Connecting to server... ");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("client: socket");
        exit(1);
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(hostname);
    servaddr.sin_port = htons(atoi(port));

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        close(sockfd);
        perror("client: connect");
        exit(1);
    }

    printf("Connected!\n");
    
    // Read and send message
    char msg[MAXDATASIZE];
    printf("Connected to a friend! You send first.\n");
    

    while (true) {
        bzero(msg, sizeof(msg));
        getInput("You: ", msg);
        write(sockfd, msg, sizeof(msg));
        bzero(msg, sizeof(msg));
        read(sockfd, msg, sizeof(msg));
        printf("Friend: %s\n", msg);
    } 

    return 0;
}

const char* getLocalIP() {
    struct ifaddrs *myaddrs, *ifa;
    void *in_addr;
    static char buf[64];

    if(getifaddrs(&myaddrs) != 0) {
        perror("getifaddrs");
        exit(1);
    }

    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;

        switch (ifa->ifa_addr->sa_family) {
            case AF_INET:
            {
                struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                in_addr = &s4->sin_addr;
                break;
            }

            default:
                continue;
        }

        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf))) {
            printf("%s: inet_ntop failed!\n", ifa->ifa_name);
        }
    }

    freeifaddrs(myaddrs);
    
    return buf;
}

int server() {
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in servaddr, theiraddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("server: socket");
        exit(1);
    }

    const char* IPbuffer = getLocalIP();
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IPbuffer);
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        close(sockfd);
        perror("server: bind");
        exit(1);
    }

    printf("Welcome to Chat!\n"); 
    printf("Waiting for connection on %s port %d\n", IPbuffer, PORT);

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }


    int sin_size = sizeof theiraddr;
    new_fd = accept(sockfd, (struct sockaddr *)&theiraddr, &sin_size);
    if (new_fd == -1) {
        perror("accept");
    }


    printf("Found a friend! You receive first.\n");

    while (true) {
        char buf[MAXDATASIZE];

        bzero(buf, sizeof(buf));
        int numbytes;
        if ((numbytes = read(new_fd, buf, sizeof(buf))) == -1) {
            perror("server recv");
            exit(1);
        }

        printf("Friend: %s\n", buf);

        bzero(buf, sizeof(buf));
        getInput("You: ", buf);
        if (write(new_fd, buf, sizeof(buf)) == -1)
            perror("send");
    }
    
    close(sockfd);
    return 0;

}

bool isPort(char number[]) {
    int i = 0;

    //checking for negative numbers
    if (number[0] == '-')
        return false;
    for (; number[i] != 0; i++)
    {
        //if (number[i] > '9' || number[i] < '0')
        if (!isdigit(number[i]))
            return false;
    }


    int port = atoi(number);
    if (port < 0 || port > 65535)
        return false;
    
    return true;
}

bool isIPAddress(char* ip) {
    // Check IP address
    unsigned char buf[sizeof(struct in_addr)];
    if (inet_pton(AF_INET, ip, buf) <= 0) {
        return false;
    }
    return true;
}

void help() {
    printf("Usage: ./chat [-h] [-s SERVER_IP] [-p PORT]\n");
    printf("Flags:\n-h displays this message\n-p specifies port for connection\n-s IPv4 address of server\n");
    printf("No flags or arguments will run the chat program as a server.\n");
    return;
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0) {
            help();
        }
        // Validate client configuration
        else if (strcmp(argv[1], "-p") == 0) {
            if (argc < 4 || strcmp(argv[3], "-s") != 0) 
                printf("\nClient connections require both -p PORT and -s SERVER_IP arguments.\n\n");
            
            else if (!isPort(argv[2]))
                printf("\nPORT must be a number between 0 and 65535.\n\n");
            
            else if (!isIPAddress(argv[4]))
                printf("Value supplied for SERVER_IP must be a valid IPv4 address.\n\n");
            
            else {
                char *port = argv[2];
                char *ip_addr = argv[4];
                client(port, ip_addr);
            }

            help();
            
            return 1;
        }
        
        // Validate client configuration
        else if (strcmp(argv[1], "-s") == 0) {
            if (argc < 4 || strcmp(argv[3], "-p") != 0) 
                printf("Client connections require both -p PORT and -s SERVER_IP arguments");
            
            else if (!isPort(argv[4]))
                printf("Port number must be between 0 and 65535.");
            
            else if (!isIPAddress(argv[2]))
                printf("Invalid IPv4 address.");
            
            else {
                char *port = argv[4];
                char *ip_addr = argv[2];
                client(port, ip_addr);
            }
            
            // Invalid arguments 
            return 1;
        }
        
        else {
            help();
        }
    }    
    
    // No arguments
    else {
        server();
    }
}
