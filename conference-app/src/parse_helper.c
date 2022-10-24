#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/global.h"

void stringToMessage(char* str, struct message *msg){
    // printf("recieving %s\n", str);
    str = strtok(str, ":");
    msg->type = atoi(str);

    str = strtok(NULL, ":");
    msg->size = atoi(str);

    str = strtok(NULL, ":");
    if(str != NULL)
        strcpy(msg->source, str);

    str = strtok(NULL, "\0");
    if(str != NULL)    
        strcpy(msg->data, str);

}


void messageToString(char* str, struct message *msg){
    int num_chars = sprintf(str, "%d:%d:%s:%s", msg->type, msg->size, msg->source, msg->data);
    // printf("got %s\n", str);

}