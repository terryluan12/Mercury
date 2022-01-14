#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define MAXBUFLEN 100

/*
 * Function: deliver <server address> <server port number>
 * -------------------------------------------------------
 * Given the port and IPv4 address, allows a user to 
 * do various actions. Only supports ftp at the moment
 */

int main(int argc, char *argv[]){

    struct addrinfo hints, *servinfo;
    struct in_addr servaddr;
    char buf[MAXBUFLEN];
    char ftppt[3] = "ftp";
    int numbytes;

    // only works if the user gives 2 arguments
    if (argc != 3){
        printf("NOT ENOUGH ARGUMENTS\n");
        return -1;
    }

    // set the hints struct to use IPv4, UDP, and 
    // accept a numeric port
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICSERV;

    // try to convert the server address from "printable"
    // form to "network" form. Store it in servaddr
    int success = inet_pton(AF_INET, argv[1], &servaddr);

    // if it's successful, notify hint that the IP address
    // is numeric
    if(success == 1){
        hints.ai_flags |= AI_NUMERICHOST;
    }

    // get the info of the server and store in servinfo
    success = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
    if(success != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(success));
        printf("COULDN'T FIND ADDRESS: %s AND PORT %s\n", argv[1], argv[2]);
        return -1;
    }

    // try to get the socket info based on info of server
    int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sockfd == -1){
        printf("COULDN'T FIND SOCKET");
        return -1;
    }

    // go into a continuous loop as commands can be input
    while(1){
        printf("Please enter ftp <file> or -1 to exit: ");
        scanf("%s", buf);

        // check if first word input is "ftp". If not, exit.
        if(strcmp(buf, ftppt) == 0){
            printf("first command must be ftp. Cannot be %st\n", buf);
            return -1;
        }else if(buf == "-1"){
            break;
        }

        scanf("%s", buf);

        // if the file exists, then send "ftp" to the server. Otherwise quit
        if(access(buf, F_OK) == 0){
            numbytes = sendto(sockfd, ftppt, 3, 0, servinfo->ai_addr, servinfo->ai_addrlen);
            if(numbytes == -1){
                perror("COULDN'T SEND\n");
                return -1;
            }
        }else{
            printf("NO FILE FOUND\n");
            return -1;
        }
        printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
        printf("Waiting on response...\n");

        // wait for a response
        numbytes = recvfrom(sockfd, &buf, MAXBUFLEN-1, 0, servinfo->ai_addr, (socklen_t *) &servinfo->ai_addrlen);
        buf[numbytes] = '\0';

        printf("listener: got packet, %d bytes long\n", numbytes);
        printf("Got a %s\n", buf);

        // check response
        if(strcmp(buf, "yes") == 0){
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