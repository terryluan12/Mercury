#ifndef GLOBAL_H_
#define GLOBAL_H_

struct packet {
	unsigned int total_frag;
	unsigned int frag_no;
	unsigned int size;
	char* filename;
	char filedata[1000];
};

struct node {
	char data[1000];
	struct node * next;
	int num_bytes;
};

#endif