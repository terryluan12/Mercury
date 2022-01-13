#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define MAXBUFLEN 100

int main(int argc, char *argv[]){
    char buf[MAXBUFLEN];
    struct addrinfo hints, *servinfo;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    int port, rv;

    if(argc != 2){
        printf("MUST BE ONE ARG");
        return -1;
    }


    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    
    if((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
        close(sockfd);
        printf("ERROR IN BIND");
        return -1;
    }

    printf("listener: waiting to recvfrom...\n");

    int numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
                            (struct sockaddr *)&their_addr, &addr_len);

    if(numbytes == -1){
        printf("NOTHING RECEIVED");
        close(sockfd);
        return -1;
    }
    close(sockfd);
    printf("everything good. Closing");

    return 0;


    
}