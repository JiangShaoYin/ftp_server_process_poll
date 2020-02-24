#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <strings.h>
#include <time.h>
#include <sys/msg.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/epoll.h>
typedef struct{
	int len;
	int flag;
	char buf[1000];
}train;

int sendn(int socket_fd,char* buf,int len);
int recvn(int,char*,int);
int receive_from_server(int socket_fd);
int trans_file(int new_fd,char *parameters);
void sig(int signum);
int show(int socket_fd);
