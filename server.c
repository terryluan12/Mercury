#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "master.h"


struct user *userList[MAXNUMUSERS+1], *loggedList[MAXLOGGEDINUSERS+1];
struct session *sessionList[MAXNUMSESSIONS+1];
pthread_mutex_t *numUsers_mutex, *loggedList_mutex, *sessionList_mutex;
int numUsers;
void *returnval;

void *mainLoop(void *arg){
    struct user *mainUser = (struct user *)arg;

    char buf[MAXBUFLEN];
    char id[MAXIDLEN];
    int type;
    int loggedIn = 0;
    int sessionIDLoc = -1;

    while(1){
        // all variables that need to be reset each packet
        struct message *messagerecv, *messagesend;

        memset(buf, 0, sizeof(char) * MAXBUFLEN);
        memset(messagerecv, 0, sizeof(struct message));
        memset(messagesend, 0, sizeof(struct message));

        // Get the next packet from the user
        // basically equal to recvfrom
        printf("listener: waiting to recvfrom...\n");
        int numbytes = read(mainUser->sockfd, buf, MAXBUFLEN-1);
        if(numbytes == -1){
            printf("NOTHING RECEIVED\n");
            return returnval ;
        }
        buf[numbytes] = '\0';

        // turn it into a string, input it into messagerecv and mainuser
        stringToMessage(buf, messagerecv);
        strcpy(mainUser->id, messagerecv->source);
        printf("listener: got packet, %d bytes long from %s\n", numbytes, mainUser->id);
        type = messagerecv->type;

        if(type == EXIT)
            break;
        
        // if not logged in yet
        if(!loggedIn){
            if(type == LOGIN){
                int isDuplicate = 0;
                int isValidUser = 0;

                printf("Attempting to log in\n");
                strcpy(mainUser->password, messagerecv->data);
                printf("Password is %s", mainUser->password);

                // iterate through the loggedList and check if
                // the user is logged in already
                int i = 0;
                while(loggedList[i] != NULL){
                    if(strcmp(loggedList[i]->id, mainUser->id) == 0){
                        isDuplicate = 1;
                        break;
                    }
                    i++;
                } 

                // iterate through userList and check if the login info is valid
                i = 0;
                while(userList[i] != NULL){
                    if(strcmp(loggedList[i]->id, mainUser->id) == 0 &&
                       strcmp(loggedList[i]->password, mainUser->password) == 0){
                        isValidUser = 1;
                        break;
                    }
                    i++;
                } 
                
                // Check if invalid
                if(!isValidUser){
                    messagesend->type = LO_NAK;
                    strcpy(messagesend->data, "Invalid login info");
                }else if(isDuplicate){
                    messagesend->type = LO_NAK;
                    strcpy(messagesend->data, "User already logged in");
                }else{
                    printf("Logging in...");

                    // create new copy of the user to insert into loggedList
                    struct user *tempUser = malloc(sizeof(struct user));
                    memcpy(tempUser, mainUser, sizeof(struct user));

                    // Insert into loggedList
                    pthread_mutex_lock(loggedList_mutex);
                        loggedList[numUsers-1] = tempUser;
                        loggedList[numUsers] = NULL;
                    pthread_mutex_unlock(loggedList_mutex);
                    loggedIn = 1;

                    // Update message to send
                    messagesend->type = LO_ACK;
                    printf("Successfully logged in!!");
                }

            }else{
                messagesend->type = LO_NAK;
                strcpy(messagesend->data, "Not logged in.");
            }

        }else{
            if(type == JOIN){
                printf("Attempting to join session");
                if(sessionIDLoc != -1){
                    messagesend->type = JN_NAK;
                    // TODO Sona can you find a less janky way to add the error dataaaaa
                    // strcpy(messagesend->data, (char*)atoi(messagerecv->data) + "Already in a Session.");

                }else{
                    int inSession = 0;
                    int isValidSession = 0;
                    int i;
                    int sessionID = atoi(messagerecv->data);
                    
                    // Check if is a valid session
                    i = 0;
                    while(sessionList[i] != NULL){
                        if(sessionID == sessionList[i]->sessionID){
                            isValidSession= i;
                            sessionID = i;
                            break;
                        }
                        i++;
                    }

                    // Check if in session already
                    if(isValidSession){
                        int j = 0;
                        while(sessionList[sessionID]->users[j] != NULL){
                            if(mainUser->id == sessionList[i]->users[j]->id){
                                inSession = 1;
                                break;
                            }
                            j++;
                        }
                    }

                    if(!isValidSession){
                        // TODO here too plzzz
                        messagesend->type = JN_NAK;
                        // strcpy(messagesend->data, (char*)atoi(messagerecv->data) + "Invalid Session.");
                    }else if(inSession){
                        // TODO here too plzzz
                        messagesend->type = JN_NAK;
                        // strcpy(messagesend->data, (char*)atoi(messagerecv->data) + "Session already joined.");
                    }else{

                        // Find the last user in the session
                        i = 0;

                        while(sessionList[sessionID]->users[i] != NULL)
                            i++;

                        
                        // Create new copy of the user to insert into session
                        struct user *tempUser = malloc(sizeof(struct user));
                        memcpy(tempUser, mainUser,sizeof(struct user));

                        // Add the user to the session
                        pthread_mutex_lock(sessionList_mutex);
                            sessionList[sessionID]->users[i] = tempUser;
                            sessionList[sessionID]->users[i+1] = NULL;
                        pthread_mutex_unlock(sessionList_mutex);
                        sessionIDLoc = sessionID;

                        // Update message to send
                        messagesend->type = JN_ACK;
                        strcpy(messagesend->data, messagerecv->data);
                        printf("User joined session successfully");
                    }
                } 
            }
            else if(type == LEAVE_SESS){
                // If not in a session
                if(sessionIDLoc == -1)
                    strcpy(messagesend->data, "NOT IN SESSION CURRENTLY. UNEXPECTED!");
                int i, j;

                // Lock the sessionList so we can access and edit it
                pthread_mutex_lock(sessionList_mutex);
                    i = 0;
                    // Get the location of the user, and free it
                    while(sessionList[sessionIDLoc]->users[i]->id != mainUser->id)
                        i++;
                    free(sessionList[sessionIDLoc]->users[i]);

                    // shift all the users left, so the "users" array is contiguous
                    while(sessionList[sessionIDLoc]->users[i+1] != NULL){
                        sessionList[sessionIDLoc]->users[i] = sessionList[sessionIDLoc]->users[i+1];
                        i++;
                    }
                    sessionList[sessionIDLoc]->users[i] = NULL;
                pthread_mutex_unlock(sessionList_mutex);
                sessionIDLoc = -1;

                printf("User left session successfully");
            }
            else if(type == NEW_SESS){
                if(sessionIDLoc == -1)
                    strcpy(messagesend->data, "NOT IN SESSION CURRENTLY. UNEXPECTED!");
                
                // create temp Session to insert into list
                struct session *tempSession = malloc(sizeof(struct session));
                tempSession->sessionID = atoi(messagerecv->data);
                tempSession->users[0] = NULL;

                pthread_mutex_lock(sessionList_mutex);
                    int i = 0;
                    while(sessionList[i] != NULL)
                        i++;
                    sessionList[i] = tempSession;
                    sessionList[i + 1] = NULL;
                pthread_mutex_unlock(sessionList_mutex);

                
                messagesend->type = NS_ACK;
                printf("Created new session.");

            }else if(type == MESSAGE){
                
            }else if(type == QUERY){
                // ADDING THE QUERY IMPLEMENTATION
                // I ALREADY DID THE PRINTF EQUIVALENT. 
                // CAN YOU ADD INTO messagesend?? THANKSSSS
                int i = 0;
                while(sessionList[i] != NULL){
                    int j = 0;
                    printf("Session: %d\n", sessionList[i]->sessionID);
                    while(sessionList[i]->users[j] != NULL){
                        printf("\tUser: %s\n", sessionList[i]->users[j]->id);
                    }
                }
            }else{
                printf("ERROR IN TYPE PANICCC");
            }
        }

        memcpy(messagesend->source, mainUser->id, MAX_NAME);
        messagesend->size = strlen(messagesend->data);
        memset(buf, 0, MAXBUFLEN);
        messageToString(buf, messagesend);
        
        numbytes = send(mainUser->sockfd, buf, MAXBUFLEN-1, 0);

        if(numbytes == -1){
            printf("ERROR IN SENDING");
        }

        //TODO ADD LOGOUT FEATURESFKLD:SLJKF:SD


    }

}


int main(int argc, char **argv){

    int n, port, client_len;
    int i, sockfd;
    char buf[MAXBUFLEN];
    struct sockaddr_in server, client;
    struct addrinfo hints, *servinfo;
    
    loggedList[0] = NULL;
    sessionList[0] = NULL;
    numUsers = 0;
    
    if(argc != 2){
        printf("MUST BE ONE ARG\n");
        return -1;
    }
    else
        port = atoi(argv[1]);


    // creating the server
    // PARTIALLY BASED OFF COMMMUNICATION NETWORKS FUNDAMENTALS

    if(sockfd = socket(AF_INET, SOCK_STREAM, 0) == -1){
        fprintf(stderr, "Can't create a socket\n");
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

	printf("port is %s\n", argv[1]);
    int success = getaddrinfo(NULL, argv[1], &hints, &servinfo);
    if(success != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(success));
        return -1;
    }


    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sockfd == -1)
        printf("ERROR GETTING SOCKET");

    if(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
        close(sockfd);
        printf("ERROR IN BIND\n");
        return -1;
    }
    printf("server: successfully binded");


    // Opening the list of possible users.
    FILE *fptr;
    fptr = fopen(".\\users.txt", "r");

    // iterate through the userlist and add them to 
    i = 0;
    while(1){
        struct user *tempUser = malloc(sizeof(struct user));
        int items =  fscanf(fptr, "%s : %s\n", tempUser->id, tempUser->password);
        if (items == -1){
            free(tempUser);
            break;
        }
        userList[i] = tempUser;
        i++;
    }
    userList[i] = NULL;
    fclose(fptr);
    printf("server: loaded user data");

    listen(sockfd, MAXLOGGEDINUSERS);

    do{
        while(1){
            struct user *tempUser = malloc(sizeof(struct user));
            client_len = sizeof(client);
            tempUser->sockfd = accept(sockfd, (struct sockaddr *)&client, &client_len);

            if(tempUser->sockfd == -1){
                fprintf(stderr, "Error with connecting to client");
                break;
            }
            
            pthread_mutex_lock(numUsers_mutex);
                numUsers ++;
            pthread_mutex_unlock(numUsers_mutex);

            pthread_create(&(tempUser->channel), NULL, mainLoop ,(void *)tempUser);

        }
    }while(numUsers != 0);

    

    
    
}