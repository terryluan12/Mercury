#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){

    struct addrinfo hints, *servinfo;
    struct in_addr servaddr;
    char input[256];


    if (argc != 3){
        printf("NOT ENOUGH ARGUMENTS\n");
        return -1;
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICSERV;

    int success = inet_pton(AF_INET, argv[1], &servaddr);

    if(success == 1){
        printf("ITS AN IP\n");
        hints.ai_flags |= AI_NUMERICHOST;
    }

    success = getaddrinfo(argv[1], argv[2], &hints, &servinfo);

    if(success != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(success));
        printf("COULDN'T FIND ADDRESS: %s AND PORT %s\n", argv[1], argv[2]);
        return -1;
    }

    int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sockfd == -1){
        printf("COULDN'T FIND SOCKET");
        return -1;
    }

    while(1){
        printf("Please enter a command or -1 to exit: ");
        scanf("%s", input);
        printf("IT is %s\n", input);
    }

}