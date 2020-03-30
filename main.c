#include"func.h"

void sig_exit(int signum){
	char c = 'a';
	write(fds[1], &c, sizeof(c));
}

int main() {
	int pro_num = 10;
	pdata subProcess = (pdata)calloc(pro_num, sizeof(data)); // 进程
	make_child(subProcess, pro_num); // 创建子进程
	
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0); // 生成套接口描述符
	int reuse = 1; // 设置socket_fd属性为reuse，这样当客户端断开后，端口不必等待（否则要等待tcp4次挥手完成后，这个socket描述符才可以重新使用）
	int ret = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)); // 设置socket选项，（&resue是指针，指向存放选项值的缓冲区，缓冲区大小为sizeof（int））
		struct sockaddr_in ser = {0}; // 用一个套接字地址结构体，将服务器的ip地址和端口号等信息填入
			ser.sin_family = AF_INET;
			ser.sin_port = htons(1990);
			ser.sin_addr.s_addr = inet_addr("192.168.4.200");
	bind(socket_fd, (struct sockaddr*)&ser, sizeof(struct sockaddr));//（将套接口与指定的端口号和ip地址相连）
	
	int epoll_fd = epoll_create(1); // 创建epoll的fd
	struct epoll_event event, *epoll_monitor_events;
	epoll_monitor_events = (struct epoll_event*)calloc(pro_num, sizeof(struct epoll_event)); // 创建epoll_event结构体，将要监控的描述符和监控的事件填入
	
	event.data.fd = socket_fd; // server监听的fd
	event.events = EPOLLIN;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event); // 向内核，注册要监控的socket_fd, 监视是否有client的connect请求。
	
	for(int i = 0; i < pro_num; i++) { 
		event.data.fd = subProcess[i].pipe_fd; // 将子进程与父进程通信的pipe_fd, 添加进epoll
		event.events = EPOLLIN;
			epoll_ctl(epoll_fd, EPOLL_CTL_ADD, subProcess[i].pipe_fd, &event);
		}

	int fds[2];
	pipe(fds); // 建立匿名管道， 将读端注册进epoll， 监控父进程是否异常退出
		event.events = EPOLLIN; // 默认是LT边缘触发
		event.data.fd = fds[0]; 
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fds[0], &event);
	signal(SIGUSR1, sig_exit); // 主进程退出， 由sig_exit函数向管道写1个char， epoll发现后就知道父进程退出了

	listen(socket_fd, pro_num); //
	char buffer; // 把fds[0]字符读掉
	while (1) {
		int to_be_processed_nums = epoll_wait(epoll_fd, epoll_monitor_events, pro_num + 2, -1); // 2 * pro_num个管道描述符，监控父进程中的pro_num个 + socket_fd + 描绘父进程是否异常退出的fd
		for (int i = 0; i < to_be_processed_nums; i++) {
			if (epoll_monitor_events[i].data.fd == fds[0]) { // 父进程管道被触发
				printf("father want exit\n");
					read(fds[0], &buffer, sizeof(buffer));
						event.events = EPOLLIN;
						event.data.fd = socket_fd; // 删除socket，不再接受新的client连接
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, &event);
					for (available_process_idx = 0; available_process_idx < pro_num; available_process_idx++) {
						send_socketFd_to_pipe(subProcess[available_process_idx].pipe_fd, 0, 1); // buffer=1表示，父进程告诉子进程自己要退出，随便发个文件
					}					//描述符过去（本例是0），主要是利用sendmsg发送结束标识buffer
					for (available_process_idx = 0; available_process_idx < pro_num; available_process_idx++) {
						wait(NULL);		//回收子进程资源			
					}
					exit(0);
				}
			int available_process_idx;
			if (epoll_monitor_events[i].data.fd == socket_fd) { // 父进程socket_fd上, 来消息了。启用子进程建立连接。
				int new_socket_fd = accept(socket_fd, NULL, NULL);
					for (available_process_idx = 0; available_process_idx < pro_num; j++) { // 找一个不忙的进程出来干活。
						if (false == subProcess[available_process_idx].busy) {
							break;
						}
					}
				send_socketFd_to_pipe(subProcess[available_process_idx].pipe_fd, new_socket_fd, 0);
				subProcess[available_process_idx].busy = true;
				printf("%d child is busy\n", subProcess[available_process_idx].pid);
				close(new_socket_fd);
			}
			for (available_process_idx = 0; available_process_idx < pro_num; available_process_idx++) {   //监控父进程的读端，判断与父进程相连的哪一个管道里面有消息（子进程通知父进程，我闲了）
				if (epoll_monitor_events[i].data.fd == subProcess[available_process_idx].pipe_fd) {
					read(subProcess[available_process_idx].pipe_fd, &buffer, sizeof(buffer)); // 如果是ET模式，在这里循环读，read设置为非阻塞，当读空的时候，返回0
					subProcess[available_process_idx].busy = false;
					printf("%d child is not busy\n", subProcess[available_process_idx].pid);
				}
			}
		}
	}
}
