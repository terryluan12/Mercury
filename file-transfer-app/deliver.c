#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

struct node {
	char data[1000];
	struct node * next;
	int num_bytes;
};

#define MAXBUFLEN 100

/*
 * Function: deliver <server address> <server port number>
 * -------------------------------------------------------
 * Given the port and IPv4 address, allows a user to 
 * do various actions. Only supports ftp at the moment
 */

int main(int argc, char *argv[]){

    struct addrinfo hints, *servinfo;
    struct in_addr servaddr;
    char buf[MAXBUFLEN];
	char file_name[MAXBUFLEN];
    char ftppt[3] = "ftp";
    int numbytes;

    // only works if the user gives 2 arguments
    if (argc != 3){
        printf("NOT ENOUGH ARGUMENTS\n");
        return -1;
    }

    // set the hints struct to use IPv4, UDP, and 
    // accept a numeric port
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICSERV;

    // try to convert the server address from "printable"
    // form to "network" form. Store it in servaddr
    int success = inet_pton(AF_INET, argv[1], &servaddr);

    // if it's successful, notify hint that the IP address
    // is numeric
    if(success == 1){
        hints.ai_flags |= AI_NUMERICHOST;
    }

    // get the info of the server and store in servinfo
    success = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
    if(success != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(success));
        printf("COULDN'T FIND ADDRESS: %s AND PORT %s\n", argv[1], argv[2]);
        return -1;
    }

    // try to get the socket info based on info of server
    int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sockfd == -1){
        printf("COULDN'T FIND SOCKET");
        return -1;
    }

    // go into a continuous loop as commands can be input
    while(1){
        printf("Please enter ftp <file> or -1 to exit: ");
        scanf("%s", buf);

        // check if first word input is "ftp". If not, exit.
        if(strcmp(buf, ftppt) != 0){
            printf("first command must be ftp. Cannot be %s\n", buf);
            return -1;

        }else if(strcmp(buf, "-1") == 0){
            break;
        }
        scanf("%s", file_name);

		clock_t timer;
        
        timer = clock();

     	// if the file exists, then send "ftp" to the server. Otherwise quit
        if(access(file_name, F_OK) == 0){
            numbytes = sendto(sockfd, ftppt, 3, 0, servinfo->ai_addr, servinfo->ai_addrlen);	
            if(numbytes == -1){
                perror("COULDN'T SEND\n");
                return -1;
            }
        }
        if(numbytes == -1){
        	perror("COULDN'T SEND\n");
        	return -1;
       	}
        
       	printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
       	printf("Waiting on response...\n");

        // wait for a response
        numbytes = recvfrom(sockfd, &buf, MAXBUFLEN-1, 0, servinfo->ai_addr, (socklen_t *) &servinfo->ai_addrlen);
		timer = clock() - timer;
        buf[numbytes] = '\0';

		double roundtriptime = ((double) timer)/(CLOCKS_PER_SEC/1000000);

		printf("ROUND TRIP TIME: %f microsec\n", roundtriptime);


        printf("listener: got packet, %d bytes long\n", numbytes);

        // check response
        if(strcmp(buf, "yes") == 0){
            printf("A file transfer can start.\n");
        }else{

           	printf("File transfer cannot occur\n");
       		return -1;
       	}
        //read file data
		FILE *file = fopen(file_name, "rb");
		if (file == NULL) 
			return -1;
		
		//char data[1000];
		int num_packets = 0;
			
		struct node * head;
		struct node * curr;
		
		int first = 1;
			
		struct node * new = malloc(sizeof(struct node));
		head = new;
		int bytes_read = fread(new -> data, sizeof(char), 1000, file);
		while (bytes_read != 0) {
				
			//new -> data = data;
			new -> num_bytes = bytes_read;
			num_packets++;
			new -> next = NULL;
			
			curr = malloc(sizeof(struct node));
			if (first != 1) {
				curr -> next= new;
			} else {
				first = 0;
			}
			curr = new;
			new = malloc(sizeof(struct node));
			bytes_read = fread(new -> data, sizeof(char), 1000, file);
			printf("asdfha\n");
		}	
				
		fclose(file);
		free(new);
				
		curr = head;
		char * packets[num_packets];
		int buff_size[num_packets];
			
		for (int i = 1; i <= num_packets; i++) {
			//printf("at loop %d\n", i);
			char * encapdata = malloc(sizeof(char) * 100);
			
			int num_chars = sprintf(encapdata, "%d:%d:%d:%s:", num_packets, i, curr -> num_bytes, file_name);
			
			//printf("passed first\n");
			memcpy(encapdata + num_chars, curr -> data, curr -> num_bytes);
				
			buff_size[i-1] = num_chars + curr -> num_bytes;
				
			packets[i-1] = encapdata;
			curr = curr -> next;
		}

		//free all memory
		curr = head;
		for (int i = 1; i <= num_packets; i++) {
			struct node * temp = curr -> next;
			free(curr);
			curr = temp;
		}
			
		int i = 0;
		while (i < num_packets) {
			printf("at loop %d\n", i);
			timer = clock();
				
			numbytes = sendto(sockfd, packets[i], buff_size[i], 0, servinfo->ai_addr, servinfo->ai_addrlen);
			if(numbytes == -1){
				perror("COULDN'T SEND\n");
				return -1;
			}
		
			//printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
			//printf("Waiting on response...\n");

			// wait for a response
			printf("waiting for response\n");
			/*
			//numbytes = recvfrom(sockfd, &buf, MAXBUFLEN-1, 0, servinfo->ai_addr, (socklen_t *) &servinfo->ai_addrlen);
			printf("got response\n");
			timer = clock() - timer;
			buf[numbytes] = '\0';

			double roundtriptime = ((double) timer)/(CLOCKS_PER_SEC/1000000);

			printf("ROUND TRIP TIME: %f microsec\n", roundtriptime);

			printf("listener: got packet, %d bytes long\n", numbytes);
			printf("Got a %s\n", buf);

			// check response
			if(strcmp(buf, "yes") == 0){
				i++;
			}
			*/
			i++;
		}
		printf("closing\n");
		close(sockfd);
		return 0;
	}
}

