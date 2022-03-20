#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
    char * com = malloc(sizeof(char) * MAXBUFLEN);
    int *sockfd;
    *sockfd = -1;

    while(1){
        fgets(com, 999, stdin);
        buf[strcspn(com, "\n")] = '\0';
        
	
        while(*com == ' ') 
            com++;
        if(*com == '\0') continue;
	
        char * com_n;
	com_n = strtok(com, " ");
	printf("com: %s\n", com);

        if(strcmp(com_n, LOGINCMD) == 0){
            login(com, sockfd);
        }
        else if(strcmp(com_n, LOGOUTCMD) == 0){
            logout(sockfd);
        }
        else if(strcmp(com_n, JOINCMD) == 0){
            joinsess(sockfd, com);
        }
        else if(strcmp(com_n, LEAVECMD) == 0){
            leavesess(sockfd);
        }
        else if(strcmp(com_n, CREATECMD) == 0){
            createsess(sockfd);
        }
        else if(strcmp(com_n, LISTCMD) == 0){
            list(sockfd);
        }
        else if(strcmp(com_n, QUITCMD) == 0){
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
