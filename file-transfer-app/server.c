#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define MAXBUFLEN 256

int main(int argc, char *argv[]){
    char buf[MAXBUFLEN];
    struct addrinfo hints, *servinfo;
    struct sockaddr_storage their_addr;
    socklen_t their_addr_len;
    int port, rv;

    if(argc != 2){
        printf("MUST BE ONE ARG");
        return -1;
    }

    memset(&hints, 0, sizeof hints);
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
        printf("ERROR IN BIND\n");
        return -1;
    }

    while(1){
        printf("listener: waiting to recvfrom...\n");

        int numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
                                (struct sockaddr *)&their_addr, &their_addr_len);

        if(numbytes == -1){
            printf("NOTHING RECEIVED");
            close(sockfd);
            return -1;
        }


        buf[numbytes] = '\0';
        printf("listener: got packet, %d bytes long\n", numbytes);
        printf("Got a %s\n", buf);


        if(strcmp(buf, "ftp") == 0){
            numbytes = sendto(sockfd, "yes", 3, 0, (struct sockaddr *)&their_addr, their_addr_len);
        }else{
            numbytes = sendto(sockfd, "no", 2, 0, (struct sockaddr *)&their_addr, their_addr_len);
        }
        printf("talker: sent %d bytes back\n", numbytes);
    }
    close(sockfd);
    printf("everything good. Closing");

    return 0;


    
}