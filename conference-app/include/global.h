#ifndef GLOBAL_H_
#define GLOBAL_H_
#include <assert.h>


#include <pthread.h>

#define MAX_NAME 32
#define MAX_DATA 512
#define MAXBUFLEN 1000
#define MAXCOMLEN 20
#define MAXLOGGEDINUSERS 6
#define MAXNUMUSERS 30
#define MAXNUMSESSIONS 3


enum cmd {LOGIN, LO_ACK, LO_NAK, EXIT, JOIN, JN_ACK, JN_NAK, ADDMIN,
		  LEAVE_SESS, NEW_SESS, REGISTER, KICK, KICK_ACK, KICK_NAK, NS_ACK, MESSAGE, QUERY, QU_ACK, USERKICK, REG_ACK, REG_NACK, ADDMIN_ACK, ADDMIN_NAK};


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
	char id[MAX_NAME];
	char password[100];
	int sockfd;
	int isPrivileged;
	pthread_t channel;
};

struct session{
	char sessionID[MAX_NAME];
	struct user *users[MAXLOGGEDINUSERS];
};


void login(char *msg, int *prevSockFD, int login);
void logout(int *prevSockFD);
void joinsess(int *sockfd, char *sessID);
void leavesess(int *sockfd);
void createsess(int *sockfd, char *sessID);
void kick(int *sockfd);
void addmin(int *sockfd);
void list(int *socketfd);

void stringToMessage(char* str, struct message *msg);
void messageToString(char* str, struct message *msg);
void message(int *sockfd, char *msg);
void printmenu();
void *textsession(void* socketfd);

#endif
