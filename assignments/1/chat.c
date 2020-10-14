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

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold

#define MAXDATASIZE 140 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void getInput(char *question, char *inputBuffer)
{
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
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    printf("Connecting to server...\n");
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("Connected! %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
    
    // Read and send message
    char msg[MAXDATASIZE];
    printf("Connected to a friend! You send first.\n");
    getInput("You: ", msg);
        

    // Receive from server
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);

    return 0;
}

int server() {
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        printf("Welcome to Chat!\n"); 
        

        struct sockaddr *our_addr = p->ai_addr; 
        //inet_ntop(our_addr->sa_family,
        //    get_in_addr((struct sockaddr *)&our_addr),
        //    ip, sizeof ip);

        

        char hostbuffer[256];
        char IPbuffer[INET_ADDRSTRLEN];
        struct hostent *host_entry;
        struct addrinfo *host_res;

        gethostname(hostbuffer, sizeof(hostbuffer));
        getaddrinfo(hostbuffer, NULL, &hints, &host_res);
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)host_res->ai_addr;
        void *addr = &(ipv4->sin_addr); 
        inet_ntop(host_res->ai_family, addr, IPbuffer, sizeof IPbuffer);
 

        //host_entry = gethostbyname(hostbuffer);
        //IPbuffer = inet_ntoa(*((struct in_addr*)
        //                 host_entry->h_addr_list[0]));

        printf("Waiting for connection on %s port %d\n", IPbuffer, ((struct sockaddr_in *)our_addr)->sin_port);
        freeaddrinfo(host_res);
        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }


    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
        perror("accept");
    }

    bool receiving = true;
    while (true) {
        if (receiving) {
            printf("Found a friend! You receive first.");
            int numbytes;
            char buf[MAXDATASIZE]; 
            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("server recv");
                exit(1);
            }
            buf[numbytes] = '\0';
            printf("Friend: %s\n", buf);
            receiving = false;
        }
        else {
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
        }

    }
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
    printf("Usage: ./chat [flags] [arguments]\n");
    printf("Flags:\n-h displays this message\n-p specifies port -s IP address of server\n");
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
                printf("Client connections require both -p PORT and -s SERVER_IP arguments");
            
            else if (!isPort(argv[2]))
                printf("Port number must be between 0 and 65535.");
            
            else if (!isIPAddress(argv[4]))
                printf("Invalid IPv4 address.");
            
            else {
                char *port = argv[2];
                char *ip_addr = argv[4];
                client(port, ip_addr);
            }
            
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
