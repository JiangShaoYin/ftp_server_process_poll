#include<pwd.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<dirent.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>
#include<grp.h>

int show_authority(struct stat filestat){
	int num=filestat.st_mode;
	char mode[4]={'x','w','r','-'};
	char c[10]={0};
	int i,j,auth;
	for(i=0;i<9;i++){
		int mod=i%3;	
		auth=num&1;
			if(auth)
				c[i]=mode[mod];
			else
				c[i]='-';
		num=num>>1;
	}
	num=num>>3;
	if(num==4)
			c[i]='d';
	else
			c[i]='-';
	for(i=9;i>=0;i--)
	printf("%c",c[i]);
	return 0;
}	

int main(int args,char**argv){
	if(args!=2){
	printf("args error\n");
	return -1;
	}
	DIR*dir=opendir(argv[1]);
		if(NULL==dir){
		perror("opendir");
	return -1;
	}
	struct dirent*dinfo=NULL;
	struct stat filestat;
	char c[100]={0};
	while((dinfo=readdir(dir))!=NULL){
		if(strcmp(dinfo->d_name,".")==0||strcmp(dinfo->d_name,"..")==0){}
		else{
			int ret=stat(dinfo->d_name,&filestat);	
			if(ret==-1){
				printf("ret=-1\n");
				return -1;
			}
			memcpy(c,ctime(&filestat.st_mtime),strlen(ctime(&filestat.st_mtime))-1);
		show_authority(filestat);	
			printf("%3ld %s %s %ld	%s %s\n",filestat.st_nlink,getpwuid(filestat.st_uid)->pw_name,getgrgid(filestat.st_gid)->gr_name,filestat.st_size,c,dinfo->d_name);
			}	
		}	


return 0;
}
