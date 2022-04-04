#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "master.h"
#include <pthread.h>


static const char *mainmenu[] = {
    "Possible Commands                                      ",
	"/login <client ID> <password> <server-IP> <server-port>",
	"/logout                                                ",
	"/joinsession <session ID>                              ",
	"/leavesession                                          ",
	"/createsession <session ID>                            ",
	"/list                                                  ",
	"/quit                                                  ",
    "/getIP                                                 ",
	"<text>                                          ",
    
	NULL
};

pthread_t thread;
void *returnVal;
char userName[MAX_NAME];

void list(int *sockfd) {
    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

    char buf[MAXBUFLEN];

    struct message *message = malloc(sizeof(struct message));
    message->type = QUERY;
    message->size = 0;
    strncpy(message->source, userName, MAX_NAME);

    messageToString(buf, message);

    free(message);

    int numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);
    if(numbytes == -1){
        printf("Error, couldn't send the list to the server\n");
        return;
    }
}

void login(char *msg, int *sockfd){
    char clientID[MAX_NAME], password[100], serverIP[15], serverPort[10];
    int numbytes;
    char buf[MAXBUFLEN];

    msg = strtok(NULL, " ");
    strcpy(clientID, msg);
    strcpy(userName, clientID);

    msg = strtok(NULL, " ");
    strcpy(password, msg);

    msg = strtok(NULL, " ");
    strcpy(serverIP, msg);

    msg = strtok(NULL, " ");
    strcpy(serverPort, msg);

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
            *sockfd = -1;
            return;
        }

        *sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
        if(*sockfd == -1){
            printf("ERROR GETTING SOCKET\n");
            *sockfd = -1;
            return;
        }

        if(connect(*sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
            close(*sockfd);
            printf("ERROR CONNECTING WITH SOCKET\n");
            *sockfd = -1;
            return;
        } else {
        	int ret = pthread_create(&thread, NULL, &textsession, (void*) sockfd);
        }

        struct message *message = malloc(sizeof(struct message));
        message->type = LOGIN;

        strncpy(message->source, userName, MAX_NAME);
        strncpy(message->data, password, MAX_DATA);
        message->size = strlen(message->data);
        messageToString(buf, message);
        
        free(message);

        numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);
        if(numbytes == -1){
            printf("COULDN'T SEND TO SERVER\n");
            close(*sockfd);
            *sockfd = -1;
            return;
        }
        if(numbytes == -1){
            printf("ERROR RECEIVING\n");
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
    strncpy(message->source, userName, MAX_NAME);
    messageToString(buf, message);

    numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);

    if(numbytes == -1){
        printf("ERROR SENDING EXIT SIGNAL\n");
        return;
    }
    int ret = pthread_cancel(thread);
    assert(ret == 0);
    close(*sockfd);
    *sockfd = -1;
}

void joinsess(int *sockfd, char *sessID){
    char buf[MAXBUFLEN];
    int numbytes;
    sessID = strtok(NULL, " ");
    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

   
    struct message *message;
    message->type = JOIN;
    strncpy(message->data, sessID, MAX_DATA);
    message->size = strlen(message->data);
    strncpy(message->source, userName, MAX_NAME);
    messageToString(buf, message);

    numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);
    if(numbytes == -1){
        printf("SEND ERROR\n");
        return;
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
    printf("Leaving Session...\n");

    struct message *message;
    message->type = LEAVE_SESS;
    message->size = 0;
    strncpy(message->source, userName, MAX_NAME);

    messageToString(buf, message);

    numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);

    if(numbytes == -1){
        printf("ERROR WITH SENDING CREATE\n");
        return;
    }

}

void createsess(int *sockfd, char *sessID){
    char *sessionID;
    char buf[MAXBUFLEN];
    int numbytes;
    sessID = strtok(NULL, " ");
    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

    struct message *message;
    message->type = NEW_SESS;
    message->size = 0;
    strcpy(message->data, sessID);
    strncpy(message->source, userName, MAX_NAME);

    messageToString(buf, message);

    numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);

    if(numbytes == -1){
        printf("ERROR WITH SENDING CREATE\n");
        return;
    }

}
void message(int *sockfd, char *msg){
    char buf[MAXBUFLEN];
    int numbytes;
    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

    struct message *message;
    message->type = MESSAGE;

    strncpy(message->data, msg, MAX_DATA);
    message->size = strlen(message->data);
    strncpy(message->source, userName, MAX_NAME);

    messageToString(buf, message);

    numbytes = send(*sockfd, buf, MAXBUFLEN - 1, 0);

    if(numbytes == -1){
        printf("ERROR WITH SEND\n");
        return;
    }
}


void printmenu(){
    int ct;
    for(int i = 0; mainmenu[i]; i++){
        printf("%s\n", mainmenu[i]);
    }
    printf("\n");
}

void *textsession(void *socketfd) {
    int *sockfd = (int *)socketfd;
	char msg[1000];
    struct message *message = malloc(sizeof(struct message));
	while (1) {
		int numbytes = recv(*sockfd, msg, 999, 0);
		if (numbytes == -1) {
			continue;
		}

        stringToMessage(msg, message);
        if (message->type == MESSAGE) {
            printf("\nMessage received:\n%s\n", message->data);
            printf("client[? for commands]: ");
            fflush(stdout);
		} 
        else if(message->type == QU_ACK){
            printf(message->data);
        }
        else if(message->type == LO_ACK){
            printf("User now logged in\n");
        }
        else if (message->type == LO_NAK) {
            printf("login failed: %s\n", message->data);
            close(*sockfd);
            *sockfd = -1;
            return returnVal;
        }
        else if(message->type == JN_ACK){
            printf("Successfully joined session\n");
        }
        else if (message->type == JN_NAK) {
            printf("Join session failed: %s\n", message->data);
        }
        else if(message->type == NS_ACK){
            printf("New session Created\n");
        }
        else{
            printf("UNKNOWN PACKET RECEIVED\n");
            close(*sockfd);
            *sockfd = -1;
            return returnVal;
        }
        
	}
}
