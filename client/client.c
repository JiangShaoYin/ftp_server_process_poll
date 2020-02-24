#include "func.h"
	train t = {0};
// typedef struct{
// 	int len;
// 	int flag;
// 	char buf[1000];
// }train;
int main(){
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in ser = {0};
			ser.sin_family = AF_INET;
			ser.sin_port = htons(1990);
			ser.sin_addr.s_addr = inet_addr("192.168.4.200");
	int ret = connect(socket_fd, (struct sockaddr*)&ser, sizeof(struct sockaddr));
	if(-1 == ret){
		perror("connect");
		return -1;
	}
	int epoll_fd = epoll_create(1);
	struct epoll_event event, *epoll_monitor_events;
	epoll_monitor_events = (struct epoll_event*)calloc(2, sizeof(struct epoll_event));

	event.data.fd = socket_fd;
	event.events = EPOLLIN;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);	
		
	event.data.fd = 0; 
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &event);	
	while (1) {
		int to_be_processed_nums = epoll_wait(epoll_fd, epoll_monitor_events, 2, -1); // 监控socket_fd
		for(int i=0; i < to_be_processed_nums; i++) {
			if(0 == epoll_monitor_events[i].data.fd) {
				bzero(&t, sizeof(t));
					ret = read(0, t.buf, sizeof(t.buf));
					t.len = strlen(t.buf); // 发送小火车（命令+命令长度）
					ret = send(socket_fd, &t, 8+t.len - 1, 0); // 从小火车前2个元素占8个字节，-1是最后的buf后面的\n去掉，不发送
				char buf[4] = {0};
				memcpy(buf, t. buf,3); // 把命令字符串，的前3个字节拷贝下来
					if (strcmp(buf, "put") == 0) { // 如果是上传命令，则把字符串后面的要上传的文件名取下来
						char filename[100] = {0};
						memcpy(filename, t.buf + 4, strlen(t.buf) - 5); // t.buf的前4个字符为put+空格 
						trans_file(socket_fd, filename);
					}
				}
			if (socket_fd == epoll_monitor_events[i].data.fd) { // 
					bzero(&t, sizeof(t));
					recvn(socket_fd, (char*)&t.len, sizeof(t.len)); // 接收文件名
					recvn(socket_fd, (char*)&t.flag, sizeof(t.flag)); // 接收控制信息
					if(t.flag == 1)
						show(socket_fd);
					else if(t.flag == 2)
						jieshou(socket_fd);
			}
		}
	}
	printf("close socket_fd done\n");	
	close(socket_fd);
}			
