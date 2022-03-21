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

    printf("Thread created\n");

    while(1){
        // all variables that need to be reset each packet
        struct message *messagerecv, *messagesend;
        memset(buf, 0, sizeof(char) * MAXBUFLEN);
        messagerecv = calloc(1, sizeof(struct message));
        messagesend = calloc(1, sizeof(struct message));

        // Get the next packet from the user
        // basically equal to recvfrom
        printf("waiting for next command\n");
        int numbytes = recv(mainUser->sockfd, buf, MAXBUFLEN-1, 0);
        if(numbytes == -1){
            printf("NOTHING RECEIVED\n");
            return returnval;
        }
        buf[numbytes] = '\0';

        // turn it into a string, input it into messagerecv and mainuser
        stringToMessage(buf, messagerecv);
        printf("listener: got packet, %d bytes long from \n", numbytes, mainUser->id);
        type = messagerecv->type;


        if(type == EXIT)
            break;
            
        // if not logged in yet
        else if(!loggedIn){
            if(type == LOGIN){
                int isDuplicate = 0;
                int isValidUser = 0;

                strcpy(mainUser->id, messagerecv->source);
                strcpy(mainUser->password, messagerecv->data);

                // iterate through the loggedList and check if
                // the user is logged in already
                for(int i = 0; loggedList[i]; i++){
                    if(strcmp(loggedList[i]->id, mainUser->id) == 0){
                        isDuplicate = 1;
                        break;
                    }
                } 

                // iterate through userList and check if the login info is valid
                for(int i = 0; userList[i]; i++){
                    if(strcmp(userList[i]->id, mainUser->id) == 0 &&
                       strcmp(userList[i]->password, mainUser->password) == 0){
                        isValidUser = 1;
                        break;
                    }
                } 
                // Check if invalid
                if(!isValidUser){
                    printf("ERROR LOGIN\n");
                    messagesend->type = LO_NAK;
                    strcpy(messagesend->data, "Invalid login info");
                }else if(isDuplicate){
                    printf("ERROR LOGGED IN\n");
                    messagesend->type = LO_NAK;
                    strcpy(messagesend->data, "User already logged in");
                }else{
                    printf("Logging in...\n");

                    // create new copy of the user to insert into loggedList
                    struct user *tempUser = malloc(sizeof(struct user));
                    memcpy(tempUser, mainUser, sizeof(struct user));

                    // Insert into loggedList
                    pthread_mutex_lock(loggedList_mutex);
                        pthread_mutex_lock(numUsers_mutex);
                            loggedList[numUsers-1] = tempUser;
                            loggedList[numUsers] = NULL;
                        pthread_mutex_unlock(numUsers_mutex);
                    pthread_mutex_unlock(loggedList_mutex);

                    // Update message to send
                    messagesend->type = LO_ACK;
                    loggedIn = 1;
                    printf("Successfully logged in!!\n");
                }

            }else{
                messagesend->type = LO_NAK;
                strcpy(messagesend->data, "Not logged in.");
            }

        }else{
            if(type == JOIN){
                printf("Attempting to join session\n");
                if(sessionIDLoc != -1){
                    messagesend->type = JN_NAK;
                    // TODO Sona can you find a less janky way to add the error dataaaaa
                    strcpy(messagesend->data, messagerecv->data);
                    strcat(messagesend -> data, " Already in a session.");
                    
                }else{
                    int inSession = 0;
                    int isValidSession = 0;
                    int i;
                    int sessionID = atoi(messagerecv->data);
                    
                    // Check if is a valid session
                    for(int i = 0; sessionList[i]; i++){
                        if(sessionID == sessionList[i]->sessionID){
                            printf("Valid session\n");
                            isValidSession = 1;
                            sessionID = i;
                            break;
                        }
                    }

                    // Check if in session already
                    if(isValidSession){
                        for(int i = 0; sessionList[sessionID]->users[i]; i++){
                            if(mainUser->id == sessionList[sessionID]->users[i]->id){
                                inSession = 1;
                                break;
                            }
                        }
                    }

                    if(!isValidSession){
                        // TODO here too plzzz
                        messagesend->type = JN_NAK;
                        strcpy(messagesend->data, messagerecv->data);
                        strcat(messagesend -> data, " Invalid session.");
                    }else if(inSession){
                        // TODO here too plzzz
                        messagesend->type = JN_NAK;
                        strcpy(messagesend->data, messagerecv->data);
                        strcat(messagesend -> data, " Session already joined.");
                    }else{

                        // Find the last user in the session
                        int i = 0;
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
                        printf("User joined session successfully\n");
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
                loggedIn = 0;

                printf("User left session successfully\n");
            }
            else if(type == NEW_SESS){
                // printf("Creating new Session\n");
                if(sessionIDLoc != -1){
                    strcpy(messagesend->data, "IN SESSION CURRENTLY. UNEXPECTED!");
                    printf("IN SESSION\n");
                }
                
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
                printf("Created new session %d.\n",sessionList[i]->sessionID);

            }else if(type == MESSAGE){
                int numbytes;

                messagesend->type = MESSAGE;

                memcpy(messagesend->source, mainUser->id, MAX_NAME);
                messagesend->size = strlen(messagesend->data);
                memcpy(messagesend->data, messagerecv->data, MAX_DATA);


                messageToString(buf, messagesend);

                for(int i = 0; sessionList[sessionIDLoc]->users[i]; i++){
                    printf("On %d", i);
                    if(sessionList[sessionIDLoc]->users[i]->id == mainUser->id)
                        continue;
                    numbytes = send(sessionList[sessionIDLoc]->users[i]->sockfd, buf, MAXBUFLEN - 1, 0);
                    if(numbytes == -1){
                        printf("ERROR SENDING\n");
                        return returnval;
                    }
                }
                printf("Finished sending to all people\n");
                memset(buf, 0, MAXBUFLEN);
                continue;
                
            }else if(type == QUERY){
                char *query = malloc(MAX_DATA);
                strcpy(query, "");
                char tempStr[10];
                strcat(query, "Sessions active:\n");
                for(int i = 0; sessionList[i]; i++){
                    strcat(query, "Session: ");
                    sprintf(tempStr, "%d", sessionList[i]->sessionID);
                    strcat(query, tempStr);
                    strcat(query, "\n");
                    for(int j = 0; sessionList[i]->users[j]; j++){
                        strcat(query, "\tUser: ");
                        sprintf(tempStr, "%d", sessionList[i]->users[j]->id);
                        strcat(query, tempStr);
                        strcat(query, "\n");
                    }
                }
                strcat(query, "\nUsers active:\n");

                for(int i = 0; loggedList[i]; i++){
                    strcat(query, "User: ");
                    strcat(query, loggedList[i]->id);
                    strcat(query, "\tSocket: ");
                    sprintf(tempStr, "%d", loggedList[i]->sockfd);
                    strcat(query, tempStr);
                    strcat(query, "\n");
                }
                strcat(query, "\n");


                strcpy(messagesend->data, query);
                messagesend->size = sizeof(query);
                messagesend->type = QU_ACK;
            }else{
                printf("ERROR IN TYPE PANICCC\n");
            }
        }


        memcpy(messagesend->source, mainUser->id, MAX_NAME);
        messagesend->size = strlen(messagesend->data);
        memset(buf, 0, MAXBUFLEN);
        messageToString(buf, messagesend);
        
        printf("Sending reply back\n");
        numbytes = send(mainUser->sockfd, buf, MAXBUFLEN-1, 0);

        if(numbytes == -1){
            printf("ERROR IN SENDING");
        }
        if(messagesend->type == LO_NAK)
            break;
        free(messagerecv);
        free(messagesend);
    }


    int i;
    if(loggedIn){
        printf("Logging out\n");
        pthread_mutex_lock(sessionList_mutex);
            // Get the location of the user, and free it
            i = 0;
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
        loggedIn = 0;
    }

    pthread_mutex_lock(loggedList_mutex);
        i = 0;
        while(strcmp(loggedList[i]->id, mainUser->id) != 0)
            i++;
        free(loggedList[i]);

        while(loggedList[i+1] != NULL){
            loggedList[i] = loggedList[i+1];
            i++;
        }
        loggedList[i] = NULL;
    pthread_mutex_unlock(loggedList_mutex);

    pthread_mutex_lock(numUsers_mutex);
        numUsers --;
    pthread_mutex_unlock(numUsers_mutex);
    printf("Exiting thread\n");
    free(mainUser);
    return NULL;

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
    numUsers_mutex = malloc(sizeof(pthread_mutex_t));
    loggedList_mutex = malloc(sizeof(pthread_mutex_t));
    sessionList_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(numUsers_mutex, NULL);
    pthread_mutex_init(loggedList_mutex, NULL);
    pthread_mutex_init(sessionList_mutex, NULL);


    if(argc != 2){
        printf("MUST BE ONE ARG\n");
        return -1;
    }
    else
        port = atoi(argv[1]);


    // creating the server
    // PARTIALLY BASED OFF COMMMUNICATION NETWORKS FUNDAMENTALS

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

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
    printf("server: successfully binded to port %d\n", sockfd);


    // Opening the list of possible users.
    FILE *fptr;
    fptr = fopen("users.txt", "r");

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
    printf("server: loaded user data\n");

    listen(sockfd, MAXLOGGEDINUSERS);

    while(1){
        struct user *tempUser = calloc(sizeof(struct user), 1);
        client_len = sizeof(client);
        printf("listener: waiting to recvfrom...\n");
        tempUser->sockfd = accept(sockfd, (struct sockaddr *)&client, &client_len);

        printf("listener: User received from socket %d. Attempting to connect...\n", tempUser->sockfd);
        if(tempUser->sockfd == -1){
            fprintf(stderr, "Error with connecting to client");
            break;
        }
        pthread_mutex_lock(numUsers_mutex);
            numUsers ++;
        pthread_mutex_unlock(numUsers_mutex);
        pthread_create(&(tempUser->channel), NULL, mainLoop ,(void *)tempUser);

    }

    printf("ERROR Closing server\n");
    close(sockfd);

    free(sessionList_mutex);
    free(numUsers_mutex);
    free(loggedList_mutex);
    for(int i = 0; userList[i]; i++)
        free(userList[i]);
    return -1;



    

    
    
}
