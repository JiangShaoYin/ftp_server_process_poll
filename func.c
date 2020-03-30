#include"func.h"

void make_child(pdata subProcess, int pro_num){
	int fds[2];
	pid_t pid;
	for(int i = 0; i < pro_num; i++){
		socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
		pid = fork();
		if (pid) {
			close(fds[0]);  // 父进程
			subProcess[i].pid = pid;
			subProcess[i].pipe_fd = fds[1]; // 父进程用fd[1]，用来接收子进程 处理完任务后，告诉自己不忙，可以接受新任务的状态
			subProcess[i].busy = 0;
		} else if (0 == pid) { // 在子进程中，fork()返回0
			close(fds[1]); // 子进程用fd[0]
			child_handle(fds[0]); // 处理读端
		}

	}
}

void child_handle(int pipe_fd) {
	int new_fd;
	char c;
	int ret;
	short cmd_exit = -1;
	char buf[100];
	signal(SIGPIPE, sig);                  //接收一个msghdr消息头的结构体，将退出的标志写在这个结构体里,用高级套接口sendmsg/recvmsg来发送接收。
	while (1) {                             //(在这个结构体中有一个变长的结构体cmsghdr，用来发文件描述符)
		recv_fd(pipe_fd, &new_fd, &cmd_exit); // 通过接收到的cmd_exit判断父进程的指令，cmd_exit=0是工作，cmd_exit=1是退出
		if(cmd_exit == 0){                // 父进程退出，子进程传输完成后回到while接收命令循环里，读到cmd_exit标志，结束子进程
			while (1) {		
				ret = cmd(new_fd);		// cmd函数是子进程与client之间交互的函数
				if (ret == -1) {          // 客户端断，子进程睡
					printf("客户端掉线\n");
					write(pipe_fd, &c, sizeof(c));	
					break;
				}
			}
		} else { 
			puts("父进程退出");
			exit(0);
		}
	}
}



int sendn(int new_fd,char*buf,int len){
	int total=0;
	int ret;
	while(total < len){
		ret = send(new_fd, buf + total, len - total, 0);
		if(ret == -1){
			printf("客户端下线\n");
			return -1;
		}
		total = total + ret;
	}
	return 0;
}

int recvn(int sfd, char* buf, int len) {
	int total = 0;
	int ret;
	while (total < len) {
		ret = recv(sfd, buf + total, len - total, 0);
		printf("\nret = %d\n", ret);
		total = total + ret;
		printf("total = %d, len = %d\n", total, len);
	}
	return ret;
}


void send_socketFd_to_pipe(int pipe_fd, int socket_fd, short cmd_flag) { // 将socket_fd send to pipe
	struct msghdr msg = {0};
	struct iovec iov[2];
	char buf2[10] = "world";
		iov[0].iov_base = &cmd_flag;
		iov[1].iov_base = buf2;
		iov[0].iov_len = 2;
		iov[1].iov_len = 5;
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	struct cmsghdr*cmsg = NULL;
		int len = CMSG_LEN(sizeof(int));
		cmsg = (struct cmsghdr*)calloc(1, len);
			cmsg->cmsg_len = len;	
			cmsg->cmsg_level = SOL_SOCKET;	
			cmsg->cmsg_type = SCM_RIGHTS;
		*(int*)CMSG_DATA(cmsg) = socket_fd;	
	msg.msg_control = cmsg;
	msg.msg_controllen = len;
	sendmsg(pipe_fd, &msg, 0);
}

void recv_fd(int pipe_fd, int *fd, short *cmd_flag) {
	struct msghdr msg = {0};
	struct iovec iov[2];
	char buf2[10] = "world";
		iov[0].iov_base = cmd_flag;
		iov[1].iov_base = buf2;
		iov[0].iov_len = 2;
		iov[1].iov_len = 5;
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	struct cmsghdr*cmsg = NULL;
		int len = CMSG_LEN(sizeof(int));
		cmsg = (struct cmsghdr*)calloc(1, len);
			cmsg->cmsg_len = len;	
			cmsg->cmsg_level = SOL_SOCKET;	
			cmsg->cmsg_type = SCM_RIGHTS;
		
	msg.msg_control = cmsg;
	msg.msg_controllen = len;

	recvmsg(pipe_fd, &msg, 0);
	*fd = *(int *)CMSG_DATA(cmsg);
}
