#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../include/global.h"


#define MAXBUFLEN 256
#define MAXPACKET 2000



/*
 * Function: server <UDP listen port>
 * -------------------------------------------------------
 * Given the port and IPv4 address, allows a user to 
 * do various actions. Only supports ftp at the moment
 */

int main(int argc, char *argv[]){
    char buf[MAXBUFLEN];
    struct addrinfo hints, *servinfo;
    struct sockaddr_storage their_addr;
    struct sockaddr_in *return_addr;
    return_addr = malloc(sizeof(struct sockaddr_in));
    socklen_t their_addr_len;
	their_addr_len = sizeof(their_addr);
    int success;
    char read_packet[2000];


    // one argument must be provided
    if(argc != 2){
        printf("MUST BE ONE ARG");
        return -1;
    }
    // set the hints struct to use IPv4, UDP, and 
    // use default IP address
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    
    // get the info of the server and store in servinfo
    success = getaddrinfo(NULL, argv[1], &hints, &servinfo);
    if(success != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(success));
        return -1;
    }

    // get socket info
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // bind socket so that the port is associated with
    // the specific IP address
    if(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
        close(sockfd);
        printf("ERROR IN BIND\n");
        return -1;
    }

    while(1){
        printf("listener: waiting to recvfrom...\n");

        // wait to receive something from user. If received
        // store users IP address
        int numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
                                (struct sockaddr *)return_addr, &their_addr_len);
        if(numbytes == -1){
            printf("NOTHING RECEIVED");
            close(sockfd);
            return -1;
        }


        buf[numbytes] = '\0';
        printf("listener: got packet, %d bytes long\n", numbytes);

        // if the server gets "ftp", send a "yes" back. 
        // else send back "no"
        if(strcmp(buf, "ftp") == 0){

		printf("trying to send yes to %s\n", their_addr);

		numbytes = sendto(sockfd, "yes", 3, 0, (struct sockaddr *)return_addr, their_addr_len);

		
        }else{
            numbytes = sendto(sockfd, "no", 2, 0, (struct sockaddr *)return_addr, their_addr_len);
        }

		//printf("error: %s\n", xn_geterror_string(xn_getlasterror()));

		struct packet first_pack;
		char filename[100];
		while(1) {
			numbytes = recvfrom(sockfd, read_packet, MAXPACKET, 0, (struct sockaddr *)return_addr, &their_addr_len);
			//get name of file
			//printf("read packet: %s\n", read_packet);
			
			char * first_str = strchr(read_packet, ':');
			int first_pos = first_str - read_packet;
			char * num_frags;
			num_frags = malloc(first_pos);
			memcpy(num_frags, read_packet, first_pos);
			first_pack.total_frag = atoi(num_frags);
		
			char * second_str = strchr(first_str + 1, ':');
			int sec_pos = second_str - first_str - 1;
			char * curr_frag;
			curr_frag = malloc(sec_pos);
			memcpy(curr_frag, first_str + 1, sec_pos);
			first_pack.frag_no = atoi(curr_frag);

			
			
			char * third_str = strchr(second_str + 1, ':');
			int third_pos = third_str - second_str - 1;
			char * size;
			size = malloc(third_pos);
			memcpy(size, second_str + 1, third_pos);
			first_pack.size = atoi(size);
			//printf("got past 3\n");
			//printf("size is %d\n", first_pack.size);
			
			//printf("something is happening\n");
			
			char * fourth_str = strchr(third_str + 1, ':');
			int fourth_pos = fourth_str - third_str - 1;
			//first_pack.filename = malloc((fourth_pos + 1) * sizeof(char));
			memcpy(filename, third_str + 1, fourth_pos);
			
			filename[fourth_pos] = '\0';
			memcpy(first_pack.filedata, (void *) (fourth_str + 1), 1000);
			
			if (first_pack.frag_no != 1) {
				numbytes = sendto(sockfd, "0", 1, 0, (struct sockaddr *)return_addr, their_addr_len);
			} else {
				printf("send\n");
				numbytes = sendto(sockfd, "2", 1, 0, (struct sockaddr *)return_addr, their_addr_len);
				break;
			}

			
		
		}

		FILE * file = fopen(filename, "wb");
		fwrite(first_pack.filedata, sizeof(char), first_pack.size, file);

		
		//numbytes = sendto(sockfd, "yes", 3, 0, (struct sockaddr *)&return_addr, their_addr_len);
		//printf("sent yes, should be %d bytes\n", numbytes);
		
		int frags_read = first_pack.frag_no;
		int total_frags = first_pack.total_frag;
		printf("frags read is %d while total frags is %d\n", frags_read, total_frags);
		struct packet curr_pack;
		
		while (frags_read < total_frags) {

			numbytes = recvfrom(sockfd, read_packet, MAXPACKET, 0, (struct sockaddr *)return_addr, &their_addr_len);
			//get name of file
			//printf("read packet: %s\n", read_packet);
			
			if (rand() % 100 < 10) {
				//simulate packet loss
				continue; //don't send ack and don't process packet
			}
			
			char * first_str = strchr(read_packet, ':');
			int first_pos = first_str - read_packet;
			char * num_frags;
			num_frags = malloc(first_pos);
			memcpy(num_frags, read_packet, first_pos);
			curr_pack.total_frag = atoi(num_frags);
		
			char * second_str = strchr(first_str + 1, ':');
			int sec_pos = second_str - first_str - 1;
			char * curr_frag;
			curr_frag = malloc(sec_pos);
			memcpy(curr_frag, first_str + 1, sec_pos);
			curr_pack.frag_no = atoi(curr_frag);

			char * third_str = strchr(second_str + 1, ':');
			int third_pos = third_str - second_str - 1;
			char * size;
			size = malloc(third_pos);
			memcpy(size, second_str + 1, third_pos);
			curr_pack.size = atoi(size);
			
			char * fourth_str = strchr(third_str + 1, ':');
			int fourth_pos = fourth_str - third_str - 1;
			curr_pack.filename = malloc(fourth_pos + 1);
			memcpy(curr_pack.filename, third_str + 1, fourth_pos);
			curr_pack.filename[fourth_pos] ='\0';
			
			int next_pack = curr_pack.frag_no + 1;
			char ack[50];
			sprintf(ack, "%d", next_pack);
			size_t bytes_send = strlen(ack);

			

			if (total_frags != curr_pack.total_frag || strcmp(filename, curr_pack.filename) != 0) {
				printf("trying to send no\n");
				numbytes = sendto(sockfd, "0", 1, 0, (struct sockaddr *)return_addr, their_addr_len);
			} else if (curr_pack.frag_no == frags_read + 1) {
				printf("trying to send ack\n");
				numbytes = sendto(sockfd, ack, bytes_send, 0, (struct sockaddr *)return_addr, their_addr_len);
				frags_read++;
			}
			else {
				//duplicate received... go to next
				continue;
			}
			
			memcpy(first_pack.filedata, (void *) (fourth_str + 1), 1000);
			fwrite(first_pack.filedata, sizeof(char), first_pack.size, file);
			
			printf("at frag %d\n", frags_read);
	}
	fclose(file);
	printf("finished creating file\n");
    }
    close(sockfd);
    printf("everything good. Closing");

    return 0;


    

}

