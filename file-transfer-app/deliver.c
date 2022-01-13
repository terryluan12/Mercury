#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define MAXBUFLEN 100

int main(int argc, char *argv[]){

    struct addrinfo hints, *servinfo;
    struct in_addr servaddr;
    char input[MAXBUFLEN], output[MAXBUFLEN];
    char ftppt[3] = "ftp";
    int numbytes;


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
        printf("Please enter ftp <file> or -1 to exit: ");
        scanf("%s", input);
        if(strcmp(input, ftppt) != 0){
            printf("first command must be ftp. Cannot be %st\n", input);
            return -1;
        }else if(input == "-1"){
            break;
        }
        scanf("%s", input);
        if(access(input, F_OK) == 0){
            if((numbytes = sendto(sockfd, ftppt, 3, 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1){
                perror("COULDN'T SEND\n");
                return -1;
            }
        }else{
            printf("NO FILE FOUND\n");
            return -1;
        }
        printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
        printf("Waiting on response...\n");

        numbytes = recvfrom(sockfd, &output, MAXBUFLEN-1, 0, servinfo->ai_addr, (socklen_t *) &servinfo->ai_addrlen);

        output[numbytes] = '\0';

        printf("listener: got packet, %d bytes long\n", numbytes);
        printf("Got a %s\n", output);

        if(strcmp(output, "yes") == 0){
            printf("A file transfer can start.\n");
        }else{
            printf("File transfer cannot occur\n");
            return -1;
        }

    }
    printf("closing\n");
    close(sockfd);
    return 0;

}