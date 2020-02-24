#include"func.h"

int cmd(int new_fd) {
		printf("服务器子进程准备接收命令：\n");
		train t = {0};
		int i,j; // for循环里面用的变量
		char command[3] = {0};
		char parameters[100] = {0};
		char *mycmd[] = {"cd","ls","put","get","del","pwd","exit",NULL};

		int ret = recv(new_fd, (char*)&t.len, sizeof(t.len), 0);//接收长度
		recv(new_fd,(char*)&t.flag, sizeof(t.flag), 0);
		recv(new_fd, t.buf, t.len - 1, 0); 
		if (0 == ret || ret < 0 ) { //send端掉线，rec返回0；
			perror("recv");
			return -1;		
		}

		printf("cmd received\n");
		printf("%s\n", t.buf);
		for(i=0; t.buf[i] != ' ' && t.buf[i] != 0; i++)
			command[i]=t.buf[i];
		int pos = i + 1;
		for(j = 0; t.buf[pos + j] != 0; j++)
			parameters[j]=t.buf[pos+j];
		int cmd_idx = -1;
		for(int i = 0; mycmd[i] != NULL; i++){
			if(strcmp(command, mycmd[i]) == 0){
				cmd_idx=i;
				break;
			}
		}
		switch(cmd_idx) {
			case 0 : CD(new_fd,parameters); break;
			case 1 : LS(new_fd); break;
			case 2 : receive_from_server(new_fd); break;
			case 3 : trans_file(new_fd,parameters); break;
			case 4 : DEL(new_fd,parameters); break;
			case 5 : PWD(new_fd); break;
			case 6 : break;
			case 7 : break;
		}
		return 0;
	}

int CD(int new_fd, char *parameters) {
	char *path = NULL;
		chdir(parameters);//危险操作，非法访问
		path=getcwd(NULL, 0);
	if(strcmp(path, ROOTDIR) < 0 || strcmp(path, ROOTDIR)==0)//path小于等于rootdir，说明越界了， #define ROOTDIR "/home/jiang/ftp"
		parameters = ROOTDIR;	
	chdir(parameters);
	LS(new_fd);
	return 0;
}

int LS(int new_fd){
	char *path=NULL;
	train t = {0};
	path = getcwd(NULL, 0);
	DIR* dir = opendir(path); // open返回指向DIR结构体的指针，该指针可被几个与目录有关的函数（readdir，）使用，返回目录的相关信息
	struct dirent* dir_info = NULL; // dirent结构体代表了，由dir指向的目录流中的下一个目录项，保存着目录项的相关信息(相对于目录起始位置的偏移，文件类型，文件名等)，想得到文件的具体信息，用stat函数获取
	struct stat filestat;//filestat结构体返回文件详细信息
		while((dir_info = readdir(dir)) != NULL){//readdir迭代读取目录
			if (strcmp(dir_info->d_name, ".") == 0 || strcmp(dir_info->d_name, "..") == 0){}
			else {
				stat(dir_info->d_name, &filestat);
				if(dir_info->d_type == 8)
				sprintf(t.buf, "%-25s%ldB", dir_info->d_name, filestat.st_size);
				else if(dir_info->d_type == 4)
				sprintf(t.buf, "%s/", dir_info->d_name);

				t.len = strlen(t.buf);
				t.flag = 1;
				puts(t.buf);
					sendn(new_fd, (char*)&t, 8 + t.len);
				}
		}
	return 0;
}


void sig(int signum) {
	printf("sigpipe is coming\n");
}

int trans_file(int new_fd, char *parameters) {
	printf("%s--------\n", parameters);
	train t;
	strcpy(t.buf, parameters);
	t.len=strlen(t.buf);
	t.flag = 2;
		sendn(new_fd, (char*)&t, 8 + t.len);
	int fd = open(parameters, O_RDONLY);

	while(bzero(&t, sizeof(t)), t.flag = 2, (t.len = read(fd, t.buf, sizeof(t.buf))) > 0){
		int ret = sendn(new_fd, (char*)&t, 8 + t.len);
		if(ret == -1){
			goto end;
		}	
	}
	t.len = 0;				//可以不加这句话，因为最后一次read返回0
	sendn(new_fd, (char*)&t, 4);//高危处
end:
	close(fd);				//如果client掉线，server子进程关闭传文件的描述符，return 0，此时子程序在一直等待接收client的命令，
							//由于client掉线，所以server子进程的cmd函数中recv返回-1，此时子进程发现客户断掉，退出接收client命令的循环，给父进程管道里写个消息，开始睡眠。
	char c = 'c';
	return 0;
}

int receive_from_server(int socket_fd) {
	train t;
	bzero(&t, sizeof(t));
		recvn(socket_fd, (char*)&t.len, sizeof(t.len));//接收文件名
		recvn(socket_fd, (char*)&t.flag, sizeof(t.flag));
		recvn(socket_fd, t.buf, t.len);
	int fd;
	printf("-------------------------------");
	printf("t.len = %d\n", t.len);
	printf("t.flag = %d\n", t.flag);
	printf("t.buf = %s\n", t.buf);
	fd=open(t.buf, O_RDWR|O_CREAT, 0666);
	if(-1 == fd) {
		perror("open");
		return -1;
	}
	while (1) {
		recvn(socket_fd, (char*)&t.len, sizeof(t.len));
		if (t.len > 0) {
				printf("t.len = %d", t.len);
			bzero(t.buf, sizeof(t.buf));
			recvn(socket_fd, (char*)&t.flag, sizeof(t.flag));
			recvn(socket_fd, t.buf, t.len);
			write(fd, t.buf, t.len);
		} else {
			break;
		}
	}
	close(fd);
	return 0;
}

int DEL(int new_fd, char *parameters){
	remove(parameters);
	LS(new_fd);
 	return 0;
}

int PWD(int new_fd){
	char *path=NULL;
	path=getcwd(NULL,0);
	puts(path);
}