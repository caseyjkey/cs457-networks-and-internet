/*
** server.c -- a stream socket server demo
*/

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
    printf("%s", question, MAXDATASIZE - 1);
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



int client(char *hostname) {
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0) {
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
        char *IPbuffer;
        struct hostent *host_entry;

        gethostname(hostbuffer, sizeof(hostbuffer));
        host_entry = gethostbyname(hostbuffer);
        IPbuffer = inet_ntoa(*((struct in_addr*)
                         host_entry->h_addr_list[0]));

        printf("Waiting for connection on %s port %d\n", IPbuffer, ((struct sockaddr_in *)our_addr)->sin_port);
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
        continue;
    }

    bool receiving = true;
    while (true) {
        if (receiving) {
            printf("Found a friend! You receive first.");

            receiving = false;
        }
        else {
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
        }

    }
    return 0;

}

int main()
{

}
