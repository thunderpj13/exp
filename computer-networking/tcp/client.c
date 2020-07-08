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

int get_line(int sock, char *buf, int size) {
    int i = 0;
    char c = '\0';
    ssize_t n;

    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);
        if (n > 0)
        {
            if (c == '\r')
            {
                n = recv(sock, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n'))
                    recv(sock, &c, 1, 0);
            }
            else
            {
                buf[i] = c;
                i++;
            }
        }
        else
            return 0;
    }
    buf[i] = '\0';

    return (i);
}

int main(void) {
    int sock;
    struct sockaddr_in srv_addr;
	char ipstr[16];
	char name[50];
	char pass[50];
	char tag = '\0';

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        perror("socket()");

    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(DEFAULT_PORT);

	printf("Please enter the server ip like 0.0.0.0: ");
	fgets(ipstr, sizeof(ipstr), stdin);
	ipstr[strlen(ipstr) - 1] = '\0';
    while (inet_pton(AF_INET, ipstr, &srv_addr.sin_addr) != 1) {
		printf("%s\n", ipstr);
		printf("not a valid ip. try again:");
		fgets(ipstr, sizeof(ipstr), stdin);
		ipstr[strlen(ipstr) - 1] = '\0';
	}

	connect(sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

	printf("You have connected to the server.\n");

login:
	printf("username:");
	fgets(name, sizeof(name), stdin);
	printf("password:");
	fgets(pass, sizeof(pass), stdin);

	write(sock, name, strlen(name));	
	write(sock, pass, strlen(pass));

    recv(sock, &tag, 1, 0);
	switch(tag) {
			case '0' : 
				printf("You have logged in successfully! And that's all, bye.'\n");
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
