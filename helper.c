#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "master.h"

void stringToMessage(char* str, struct message *msg){
    printf("recieving %s\n", str);
    str = strtok(str, ":");
    msg->type = atoi(str);

    printf("PT 1\n");

    str = strtok(NULL, ":");
    msg->size = atoi(str);

    printf("PT 2\n");

    str = strtok(NULL, ":");
    if(str != NULL)
        strcpy(msg->source, str);

    printf("PT 4\n");

    str = strtok(NULL, ":");
    if(str != NULL)    
        strcpy(msg->data, str);

    printf("It is %d:%d:%s:%s\n", msg->type, msg->size, msg->source, msg->data);
}


void messageToString(char* str, struct message *msg){
    printf("Changing\n");
    int num_chars = sprintf(str, "%d:%d:%s:%s", msg->type, msg->size, msg->source, msg->data);
    printf("got %s\n", str);

}