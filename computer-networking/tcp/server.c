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
int setup_listening_socket(int port) {
    int sock;
    struct sockaddr_in srv_addr;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        perror("socket()");

    int enable = 1;
    if (setsockopt(sock,
                   SOL_SOCKET, SO_REUSEADDR,
                   &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR)");

    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* We bind to a port and turn this socket into a listening
     * socket.
     * */
    if (bind(sock,
             (const struct sockaddr *)&srv_addr,
             sizeof(srv_addr)) < 0)
        perror("bind()");

    if (listen(sock, 10) < 0)
        perror("listen()");

    return (sock);
}
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
    buf[i - 1] = '\0';

    return (i - 1);
}
int validate(int client_socket) {
	char response;
	char buf[1024];
	int index = -1;
	int i;
	int rtv = -1;

	get_line(client_socket, buf, 1024);
	for(i = 0; i < sizeof(users) / sizeof(*users); i++) {
		if (strcmp(buf, users[i].name) == 0) {
			index = i;
		}
	}

	get_line(client_socket, buf, 1024);
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
	write(client_socket, &response, 1);
	return rtv;
}
void * handle_client(void* targ) {
	int client_socket = (long)targ;
	while(validate(client_socket) != 0);
}

void enter_server_loop(int server_socket)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t tid;

    while (1)
    {
        int client_socket = accept(
                server_socket,
                (struct sockaddr *)&client_addr,
                &client_addr_len);
        if (client_socket == -1)
            perror("accept()");

		printf("One client connected.\n");

        pthread_create(&tid, NULL, &handle_client, (void *)(intptr_t) client_socket);
    }
}

int main(void) {
	int socket = setup_listening_socket(DEFAULT_PORT);
	printf("Listening...\n");
	enter_server_loop(socket);
}
