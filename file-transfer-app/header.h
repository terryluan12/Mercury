struct packet {
	unsigned int total_frag;
	unsigned int frag_no;
	unsigned int size;
	char* filename;
	char filedata[1000];
};

struct node {
	char data[1000];
	struct node * next = nullptr;
	int num_bytes;
};
