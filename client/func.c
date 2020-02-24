#include "func.h"
extern train t;

int show(int socket_fd){
	int ret = recvn(socket_fd, t.buf, t.len);
	puts(t.buf);
}



int receive_from_server(int socket_fd){
	recvn(socket_fd, t.buf, t.len);
	int fd;
	fd = open(t.buf, O_RDWR|O_CREAT, 0666); // 接收文件名
	if (-1 == fd) {
		perror("open");
		return -1;
	}
	while (1) { // 循环读取server上的file
		recvn(socket_fd, (char*)&t.len, sizeof(t.len));
		if (t.len > 0) {
			bzero(t.buf, sizeof(t.buf));
			recvn(socket_fd, (char*)&t.flag, sizeof(t.flag));
			recvn(socket_fd, t.buf, t.len);
			write(fd,t.buf,t.len);
		}else{
			break;
		}
	}
	close(fd);
	return 0;
}
void sig(int signum){
	printf("sigpipe is coming\n");
}



int trans_file(int new_fd, char *parameters) {
	printf("%s--------\n", parameters);
	signal(SIGPIPE, sig);
	train t;
	strcpy(t.buf, parameters);
		t.len = strlen(t.buf);
		sendn(new_fd, (char*)&t, 8 + t.len);
	int fd = open(parameters, O_RDONLY);

	while(bzero(&t, sizeof(t)), (t.len = read(fd, t.buf, sizeof(t.buf))) > 0){
		int ret = sendn(new_fd, (char*)&t, 8+t.len);
		if(ret == -1){
		goto end;
		}	
	}
	t.len=0;				//可以不加这句话，因为最后一次read返回0
	sendn(new_fd, (char*)&t, sizeof(t.len));//高危处
end:
	close(fd);
		char c = 'c';
	return 0;
}

int sendn(int socket_fd, char* buf, int len) {
	int total = 0;
	int ret;
	while(total < len) {
		ret=send(socket_fd, buf + total, len - total, 0);
			if (ret == -1) {
				printf("客户端掉线\n");
			}
			if (ret == 0) {
				printf("发送0字节\n");
			}
		total = total + ret;
	}
	return ret;
}


int recvn(int socket_fd, char* buf, int len) {
	int total = 0;
	int ret;
	while (total < len) {
		ret = recv(socket_fd, buf + total, len - total,0);
		total = total + ret;
	}
	return ret;
}
