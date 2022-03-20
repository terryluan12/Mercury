#include <string.h>
#include <stdio.h>
#include "master.h"



const char *LOGINCMD = "/login";
const char *LOGOUTCMD = "/logout";
const char *JOINCMD = "/joinsession";
const char *LEAVECMD = "/leavesession";
const char *CREATECMD = "/createsession";
const char *LISTCMD = "/list";
const char *QUITCMD = "/quit";


int main(int argc, char *argv[]) {

    char buf[MAXBUFLEN];
    char *com;
    int *sockfd;
    *sockfd = -1;

    while(1){
        fgets(com, MAXBUFLEN-1, stdin);
        buf[strcspn(com, "\n")] = '\0';
        

        while(*com == ' ') 
            com++;
        if(*com == '\0') continue;

        com = strtok(com, " ");


        if(strcmp(com, LOGINCMD) == 0){
            login(com, sockfd);
        }
        else if(strcmp(com, LOGOUTCMD) == 0){
            logout(sockfd);
        }
        else if(strcmp(com, JOINCMD) == 0){
            joinsess(sockfd, com);
        }
        else if(strcmp(com, LEAVECMD) == 0){
            leavesess(sockfd);
        }
        else if(strcmp(com, CREATECMD) == 0){
            createsess(sockfd);
        }
        else if(strcmp(com, LISTCMD) == 0){
            list(sockfd);
        }
        else if(strcmp(com, QUITCMD) == 0){
            logout(sockfd);
            break;
        }
        else{
            printf("Unexpected command, please try again");
            continue;
        }
        buf[strlen(com)] = ' ';
    }
   printf("Quit successfully");
   return 0;



}