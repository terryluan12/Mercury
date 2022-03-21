#ifndef MASTER_H_
#define MASTER_H_

#include <pthread.h>

#define MAX_NAME 32
#define MAX_DATA 512
#define MAXBUFLEN 1000
#define MAXCOMLEN 20
#define MAXIDLEN 20
#define MAXLOGGEDINUSERS 6
#define MAXNUMUSERS 30
#define MAXNUMSESSIONS 3

enum cmd {LOGIN, LO_ACK, LO_NAK, EXIT, JOIN, JN_ACK, JN_NAK,
		  LEAVE_SESS, NEW_SESS, NS_ACK, MESSAGE, QUERY, QU_ACK };


struct message {
unsigned int type;
unsigned int size;
unsigned char source[MAX_NAME];
unsigned char data[MAX_DATA];
};


struct node {
	char data[1000];
	struct node *next;
	int num_bytes;
};

struct user {
	char id[MAXIDLEN];
	char password[100];
	int sockfd;
	pthread_t channel;
};

struct session{
	int sessionID;

	struct user *users[MAXLOGGEDINUSERS];
};


void login(char *msg, int *prevSockFD);
void logout(int *prevSockFD);
void joinsess(int *sockfd, char *sessID);
void leavesess(int *sockfd);
void createsess(int *sockfd, char *sessID);
void list(int *socketfd);
void stringToMessage(char* str, struct message *msg);
void messageToString(char* str, struct message *msg);
void message(int *sockfd, char *msg);
void printmenu();

#endif