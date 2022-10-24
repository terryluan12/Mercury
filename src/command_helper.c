#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../include/master.h"
#include <pthread.h>


static const char *mainmenu[] = {
    "Possible Commands(admin commands in \033[0;31mred\033[0m) ",
    "/register <username> <password>                           ",
	"/login <client ID> <password> <server-IP> <server-port>   ",
	"/logout                                                   ",
	"/joinsession <session ID>                                 ",
	"/leavesession                                             ",
	"/createsession <session ID>                               ",
    "\033[0;31m/kick <username>\033[0m                         ",
    "\033[0;31m/addadmin <username>\033[0m                     ",
	"/list                                                     ",
	"/quit                                                     ",
    "/getIP                                                    ",
	"<text>                                                    ",
    
	NULL
};

pthread_t thread;
void *returnVal;
char userName[MAX_NAME];
char isAdmin;
int inSession = 0;

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

void login(char *msg, int *sockfd, int login){
    char *clientID, *password, *serverIP, *serverPort;
    int numbytes;
    char buf[MAXBUFLEN];

    clientID = strtok(NULL, " ");

    password = strtok(NULL, " ");

    serverIP = strtok(NULL, " ");

    serverPort = strtok(NULL, " ");

    if(!userName || !password || !serverIP || !serverPort){
        printf("Incorrect Usage.\nMust be in the form /login <client ID> <password> <server-IP> <server-port>\n");
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
        if (login)
        	message->type = LOGIN;
        else message -> type = REGISTER;

        strncpy(message->source, clientID, MAX_NAME);
        strncpy(message->data, password, MAX_DATA);
        message->size = strlen(message->data);

        messageToString(buf, message);
        free(message);

        if (login)
        	printf("Logging in...\n");
        else
        	printf("Registering...\n");
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

    struct message *message = malloc(sizeof(struct message));

    message->type = EXIT;
    message->size = 0;
    strncpy(message->source, userName, MAX_NAME);
    messageToString(buf, message);
    free(message);

    printf("Logging out...\n");
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
    
    if(!sessID){
        printf("Incorrect Usage.\nMust be in the form /joinsession <session ID>\n");
        return;
    }
    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

   
    struct message *message = malloc(sizeof(struct message));
    message->type = JOIN;
    strncpy(message->data, sessID, MAX_DATA);
    message->size = strlen(message->data);
    strncpy(message->source, userName, MAX_NAME);

    messageToString(buf, message);
    free(message);

    printf("Joining Session...\n");
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
    if(!inSession){
        printf("Must be in a session to leave session\n");
        return;
    }

    struct message *message = malloc(sizeof(struct message));
    message->type = LEAVE_SESS;
    message->size = 0;
    strncpy(message->source, userName, MAX_NAME);

    messageToString(buf, message);
    free(message);

    printf("Leaving Session...\n");
    numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);
    if(numbytes == -1){
        printf("ERROR WITH SENDING CREATE\n");
        return;
    }

    inSession = 0;
}

void createsess(int *sockfd, char *sessID){
    char *sessionID;
    char buf[MAXBUFLEN];
    int numbytes;
    sessID = strtok(NULL, " ");

    if(!sessID){
        printf("Incorrect Usage.\nMust be in the form /createsession <session ID>\n");
        return;
    }

    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

    struct message *message = malloc(sizeof(struct message));
    message->type = NEW_SESS;
    message->size = 0;
    strcpy(message->data, sessID);
    strncpy(message->source, userName, MAX_NAME);

    messageToString(buf, message);
    free(message);

    printf("Creating Session...\n");

    numbytes = send(*sockfd, buf, MAXBUFLEN-1, 0);

    if(numbytes == -1){
        printf("ERROR WITH SENDING CREATE\n");
        return;
    }

}


void kick(int *sockfd){
    char buf[MAXBUFLEN];
    int numbytes;
    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

    if(!inSession){
        printf("Must be in a session to kick someone\n");
        return;
    }

    char *kickedName = strtok(NULL, " ");
    if(!kickedName){
        printf("Incorrect Usage.\nMust be in the form /kick <username>\n");
        return;
    }

    struct message *message = malloc(sizeof(struct message));
    message->type = KICK;
    strcpy(message->data, kickedName);
    message->size = strlen(kickedName);
    strncpy(message->source, userName, MAX_NAME);

    messageToString(buf, message);
    free(message);


    numbytes = send(*sockfd, buf, MAXBUFLEN - 1, 0);

    if(numbytes == -1){
        printf("ERROR WITH SEND\n");
        return;
    }
    
}

void addmin(int *sockfd){
    char buf[MAXBUFLEN];
    int numbytes;

    if(*sockfd == -1){
        printf("Not currently logged in.\n");
        return;
    }

    if(!inSession){
        printf("Must be in a session to kick someone\n");
        return;
    }

    char *newAdmin = strtok(NULL, " ");
    if(!newAdmin){
        printf("Incorrect Usage.\nMust be in the form /addadmin <username>\n");
        return;
    }

    struct message *message = malloc(sizeof(struct message));
    message->type = ADDMIN;
    strcpy(message->data, newAdmin);
    message->size = strlen(newAdmin);
    strncpy(message->source, userName, MAX_NAME);

    messageToString(buf, message);
    free(message);


    numbytes = send(*sockfd, buf, MAXBUFLEN - 1, 0);

    if(numbytes == -1){
        printf("ERROR WITH SEND\n");
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

    if(!inSession){
        printf("Must be in a session to send a message\n");
        return;
    }

    struct message *message = malloc(sizeof(struct message));
    message->type = MESSAGE;

    strncpy(message->data, msg, MAX_DATA);
    message->size = strlen(message->data);
    strncpy(message->source, userName, MAX_NAME);

    messageToString(buf, message);
    free(message);

    printf("Sending Message to Session");

    numbytes = send(*sockfd, buf, MAXBUFLEN - 1, 0);

    if(numbytes == -1){
        printf("ERROR WITH SEND\n");
        return;
    }
}

void reg(int *socketfd, char *regInfo){
    char buf[MAXBUFLEN];
    int numbytes;

    char *newUserName = strtok(NULL, " ");
    char *newPassword = strtok(NULL, "\0");

    if(!newUserName || !newPassword){
        printf("Incorrect Usage.\nMust be in the form /register <username> <password>\n");
        return;
    }
    struct message *message = malloc(sizeof(struct message));

    message->type = REGISTER;
    message->size = strlen(newUserName) + 1 + strlen(newPassword);
    sprintf(message->data, "%s : %s", newUserName, newPassword);
    strncpy(message->source, userName, MAX_NAME);
    
    messageToString(buf, message);
    free(message);

    printf("Registering new User...\n");

    numbytes = send(*socketfd, buf, MAXBUFLEN - 1, 0);
    
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
            strcpy(userName, message->source);
        }
        else if (message->type == LO_NAK) {
            printf("login failed: %s\n", message->data);
            close(*sockfd);
            *sockfd = -1;
            return returnVal;
        }
        else if(message->type == JN_ACK){
            printf("Successfully joined session\n");
            inSession = 1;
        }
        else if (message->type == JN_NAK) {
            printf("Join session failed: %s\n", message->data);
            inSession = 0;
        }
        else if(message->type == NS_ACK){
            printf("New session Created\n");
            inSession = 1;
        }
        else if(message->type == KICK_ACK){
            printf("User Kicked\n");
        }
        else if(message->type == KICK_NAK){
            printf("%s\n", message->data);
        }
        else if(message->type == ADDMIN_ACK){
            printf("User successfully added as admin\n");
        }
        else if(message->type == ADDMIN_NAK){
            printf("%s\n", message->data);
        }
		else if(message->type == USERKICK) {
			printf("You have been kicked from the session\n");
		}
		else if(message->type == REG_ACK) {
			printf("Registration successful\n");
		}
		else if(message->type == REG_NACK) {
			printf("Registration failed! Please pick another username.\n");
			close(*sockfd);
            *sockfd = -1;
            return returnVal;
		}
        else if(message->type == -1){
            printf("UNEXPECTED ERROR: %s", message->data);
        }
        else{
            printf("UNKNOWN PACKET RECEIVED\n");
            close(*sockfd);
            *sockfd = -1;
            return returnVal;
        }
        
	}
    free(message);
}
