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
typedef struct {
	char* name;
	char* pass;
} User;

User users[] = {
	{"admin1", "12345"},
	{"admin2", "12345"},
	{"admin3", "12345"},
	{"admin4", "12345"},
	{"admin5", "12345"}
};

int validate(int sock, struct sockaddr* peer_addr) {
	char response;
	char buf[1024];
	int index = -1;
	int i;
	int rtv = -1;
	ssize_t n;
	socklen_t len;

	buf[0] = '\0';

    n = recvfrom(sock, buf, 1024, 0, peer_addr, &len);
	buf[ n ] = '\0';

	printf("name: %s\n", buf);
	for(i = 0; i < sizeof(users) / sizeof(*users); i++) {
		if (strcmp(buf, users[i].name) == 0) {
			index = i;
		}
	}

	buf[0] = '\0';

    n = recvfrom(sock, buf, 1024, 0, peer_addr, &len);
	buf[ n ] = '\0';

	printf("pass: %s\n", buf);
	if(index == -1) {
		response = '1';
		printf("A user failed to logged in.\n");
	} else {
		if (strcmp(buf, users[index].pass) == 0) {
			response = '0';
			printf("%s logged in.\n", users[index].name);
			rtv = 0;
		} else {
			response = '2';
			printf("A user failed to logged in.\n");
		}
	}
	printf("send back %c\n", response);
	sendto(sock, &response, 1, 0, peer_addr, sizeof(struct sockaddr_in));
	return rtv;
}

int main(void) {
	char name[50];
	char pass[50];
	char buf[1024];
    int sock;
    struct sockaddr_in self_addr;
    struct sockaddr_in peer_addr;

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        perror("socket()");

    bzero(&self_addr, sizeof(self_addr));
    self_addr.sin_family = AF_INET;
    self_addr.sin_port = htons(DEFAULT_PORT);
    self_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock,
             (const struct sockaddr *)&self_addr,
             sizeof(self_addr)) < 0)
        perror("bind()");
	
	for(;;)
		while(validate(sock, (struct sockaddr*)&peer_addr) != 0);

}
