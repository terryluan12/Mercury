#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "master.h"

void stringToMessage(char* str, struct message *msg){
    char * first_str = strchr(str, ':');
    int first_pos = first_str - str;
    char * type;
    type = malloc(first_pos);
    memcpy(type, msg, first_pos);
    msg->type = atoi(type);

    char * second_str = strchr(first_str + 1, ':');
    int sec_pos = second_str - first_str - 1;
    char * size;
    size = malloc(sec_pos);
    memcpy(size, first_str + 1, sec_pos);
    msg->size = atoi(size);

    char * third_str = strchr(second_str + 1, ':');
    int third_pos = third_str - second_str - 1;
    memcpy(msg->source, second_str + 1, third_pos);

    memcpy(msg->data, third_str + 1, strlen(str));
}


void messageToString(char* str, struct message *msg){
    
    int num_chars = sprintf(str, "%d:%d:%s:%s", msg->type, msg->size, msg->source, msg->data);

}