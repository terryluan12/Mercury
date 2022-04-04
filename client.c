#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "master.h"
#include <pthread.h>



const char *LOGINCMD = "/login";
const char *LOGOUTCMD = "/logout";
const char *JOINCMD = "/joinsession";
const char *LEAVECMD = "/leavesession";
const char *CREATECMD = "/createsession";
const char *LISTCMD = "/list";
const char *QUITCMD = "/quit";
const char *REGCMD = "/register";


int main(int argc, char *argv[]) {

    char *buf;
    buf = malloc(sizeof(char) * MAXBUFLEN);
    int *sockfd = malloc(sizeof(int));
    *sockfd = -1;

    system("clear");
    printf(".");
    fflush(stdout);
    sleep(1);
    printf(".");
    fflush(stdout);
    sleep(1);
    printf(".");
    fflush(stdout);
    sleep(1);
    system("clear");

    while(1){
        printf("client[? for commands]: ");

        fgets(buf, MAXBUFLEN, stdin);
        buf[strcspn(buf, "\n")] = '\0';
        char *com = buf;
        while(*com == ' ') com++;
        if(*com == '\0') continue;
	
        com = strtok(buf, " ");

        if(strcmp(com, LOGINCMD) == 0){
            printf("Logging in...\n");
            login(com, sockfd);
        }
        else if(strcmp(com, LOGOUTCMD) == 0){
            printf("Logging out...\n");
            logout(sockfd);
        }
        else if(strcmp(com, JOINCMD) == 0){
            printf("Joining Session...\n");
            joinsess(sockfd, com);
        }
        else if(strcmp(com, LEAVECMD) == 0){
            printf("Leaving...\n");
            leavesess(sockfd);
        }
        else if(strcmp(com, CREATECMD) == 0){
            printf("Creating Session...\n");
            createsess(sockfd, com);
        }
        else if(strcmp(com, LISTCMD) == 0){
            printf("Listing...\n");
            list(sockfd);
        }
        else if(strcmp(com, QUITCMD) == 0){
            printf("Quitting...\n");
            logout(sockfd);
            break;
        }
        else if(strcmp(com, REGCMD) == 0){
            printf("Registering new User...\n");
            reg(sockfd, com);
        }
        else if(strcmp(com, "clear") == 0){
            system("clear");
        }
        else if(strcmp(com, "?") == 0){
            printmenu();
        }
        else if(strcmp(com, "/getIP") == 0){
            system("hostname -I");
        }
        else{
            printf("printing message\n");
            message(sockfd, com);
        }
        buf[strlen(com)] = ' ';
        sleep(1);
    }
   printf("Quit successfully\n");
   sleep(1);
   system("clear");
   return 0;



}
