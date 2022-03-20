#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "master.h"

void list(int *sockfd) {
    if(*sockfd == -1){
        printf("Not currently connected to any server.");
        return;
    }

    char *buf;

    struct message *m;
    m->type = QUERY;
    m->size = 0;

    messageToString(buf, m);

    int numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);
    if(numbytes == -1){
        printf("Error, couldn't send the list to the server");
        return;
    }
}

void login(char *message, int *sockfd){
    char clientID[20], password[100], serverIP[15], serverPort[10];
    char buf[MAXBUFLEN];
    int numbytes;

    char * str1 = strchr(message, ' ');

	printf("message: %s\n", message);
	printf("str1: %s\n", str1 + 1);
	char * str2 = strchr(str1+1, ' ');
	int pos2 = str2 - (str1 + 1);
    strncpy(clientID, str1 + 1, pos2);

    char *str3 = strchr(str2 + 1, ' ');
    int pos3 = str3 - str2 - 1;
    strncpy(password, str2 + 1, pos3);

    str1 = strchr(str3 + 1, ' ');
    pos2 = str1 - str3 - 1;
    strncpy(serverIP, str3 + 1, pos2);

    str2 = strchr(str1 + 1, ' ');
    pos3 = str2 - str1 - 1;
    strncpy(serverPort, str1 + 1, pos3);

    if(clientID == NULL || password == NULL || serverIP == NULL || serverPort == NULL){
        printf("ERROR OCCURED WITH LOGIN.\n");
        return;
    }else if(*sockfd != -1){
        printf("ALREADY LOGGED INTO SERVER\n");
        return;
    }
    else{
        
        struct addrinfo hints, *servinfo;
        struct sockaddr_storage their_addr;
        struct sockaddr_in *return_addr;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int success = getaddrinfo(serverIP, serverPort, &hints, &servinfo);
        if(success != 0){
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(success));
            return;
        }

        *sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
        if(*sockfd == -1)
            printf("ERROR GETTING SOCKET");

        if(connect(*sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
            close(*sockfd);
            printf("ERROR CONNECTING WITH SOCKET");
            return;
        }

        struct message *message = malloc(sizeof(struct message));
        message->type = LOGIN;

        strncpy(message->source, clientID, MAX_NAME);
        strncpy(message->data, password, MAX_DATA);
        message->size = strlen(message->data);
        messageToString(buf, message);

        numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);

        if(numbytes == -1){
            printf("COULDN'T SEND TO SERVER\n");
            close(*sockfd);
            *sockfd = -1;
            return;
        }

        numbytes = recv(*sockfd, buf, MAXBUFLEN-1, 0);
        if(numbytes == -1){
            printf("ERROR RECEIVING\n");
            close(*sockfd);
            *sockfd = -1;
            return;
        }

        buf[numbytes] = '\0';
        stringToMessage(buf, message);

        if(message->type == LO_ACK){
            printf("User now logged in\n");
        }
        else if (message->type == LO_NAK) {
            printf("login failed: %s\n", message->data);
            close(*sockfd);
            *sockfd = -1;
            return;
        }
        else{
            printf("UNKNOWN PACKET RECEIVED");
            close(*sockfd);
            *sockfd = -1;
            return;
        }
    }

}

void logout(int *sockfd){
    int numbytes;
    char buf[MAXBUFLEN];
    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

    struct message *message;

    message->type = EXIT;
    message->size = 0;
    messageToString(buf, message);

    numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);

    if(numbytes == -1){
        printf("ERROR SENDING EXIT SIGNAL");
        return;
    }
    close(*sockfd);
    *sockfd = -1;
}

void joinsess(int *sockfd, char *sessID){
    char *sessionID;
    char buf[MAXBUFLEN];
    int numbytes;
    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

    sessionID = strtok(sessID, " ");

    if(sessionID == NULL){
        printf("Invalid command. %s is not a valid session\n", sessionID);
    }else{
        struct message *message;
        message->type = JOIN;
        strncpy(message->data, sessionID, MAX_DATA);
        message->size = strlen(message->data);
        messageToString(buf, message);

        numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);
        if(numbytes == -1){
            printf("SEND ERROR");
            return;
        }
    }
}

void leavesess(int *sockfd){
    char *sessionID;
    char buf[MAXBUFLEN];
    int numbytes;
    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

    struct message *message;
    message->type = LEAVE_SESS;
    message->size = 0;

    messageToString(buf, message);

    numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);

    if(numbytes == -1){
        printf("ERROR WITH SENDING CREATE");
        return;
    }

}

void createsess(int *sockfd){
    char *sessionID;
    char buf[MAXBUFLEN];
    int numbytes;
    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

    struct message *message;
    message->type = NEW_SESS;
    message->size = 0;

    messageToString(buf, message);

    numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);

    if(numbytes == -1){
        printf("ERROR WITH SENDING CREATE");
        return;
    }

}
