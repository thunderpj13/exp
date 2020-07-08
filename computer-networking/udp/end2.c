#define _GNU_SOURCE /* For asprintf() */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

const int DEFAULT_PORT = 12345;


int main(void) {
    int sock;
    struct sockaddr_in peer_addr;
	char ipstr[16];
	char name[50];
	char pass[50];
	char tag = '\0';
	ssize_t n;
	socklen_t len;

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        perror("socket()");

    bzero(&peer_addr, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(DEFAULT_PORT);

	printf("Please enter the peer ip like 0.0.0.0: ");
	fgets(ipstr, sizeof(ipstr), stdin);
	ipstr[strlen(ipstr) - 1] = '\0';
    while (inet_pton(AF_INET, ipstr, &peer_addr.sin_addr) != 1) {
		printf("%s\n", ipstr);
		printf("not a valid ip. try again:");
		fgets(ipstr, sizeof(ipstr), stdin);
		ipstr[strlen(ipstr) - 1] = '\0';
	}


login:
	printf("username:");
	fgets(name, sizeof(name), stdin);
	name[strlen(name) - 1] = '\0';
	sendto(sock, name, strlen(name), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));

	printf("password:");
	fgets(pass, sizeof(pass), stdin);
	pass[strlen(pass) - 1] = '\0';
	sendto(sock, pass, strlen(pass), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));


    n = recvfrom(sock, &tag, 1, 0, (struct sockaddr*)&peer_addr, &len);
	
	printf("%d\n", n);
	printf("%c\n", tag);
	switch(tag) {
			case '0' : 
				printf("You have logged in successfully! And that's all, bye.\n");
				break;
			case '1' : 
				printf("This user doesn't exists! Try again.\n");
				goto login;
			case '2' : 
				printf("The password is not correct. Try again.\n");
				goto login;
			default : 
				printf("not an option.\n");
	}
	
	return 0;
}
