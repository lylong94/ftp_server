#include"client.h"

void recv_pwd(int fd)
{
	int head[2];
	char buf[1000];
	bzero(buf,sizeof(buf));
	recv(fd,head,sizeof(head),0);
	if(-1==head[1])
	{
		printf("operation failed\n");
	}
	recv(fd,buf,head[0],0);
	printf("%s\n",buf);
	return;
}

struct ls_t{
	long size;
	char type;
	char name[128];
};

void recv_ls(int fd)
{
	int head[2];
	char buf[200];
	struct ls_t *msg;
	recv(fd,head,sizeof(head),0);
	while(0!=head[0])
	{
		bzero(buf,sizeof(buf));
		recvn(fd,buf,head[0]);
		msg=(struct ls_t*)buf;
		printf("%c  %-20s %ld\n",msg->type,msg->name,msg->size);
		recv(fd,head,sizeof(head),0);
	}
	return;
}

void send_file(int new_fd,char *name)
{
	int file_fd=open(name,O_RDONLY);
	if(-1==file_fd)
	{
		printf("error name\n");
		return;
	}
	struct msg_send temp;
	struct stat f_stat;
	int ret;
	bzero(&temp,sizeof(temp));
	temp.cmd=5;
	bzero(&f_stat,sizeof(f_stat));
	fstat(file_fd,&f_stat);
	temp.len=sizeof(off_t);
	memcpy(temp.msg,&f_stat.st_size,sizeof(off_t));
	off_t file_size,senb=0;
	file_size=f_stat.st_size;
	float pst;//persent
	time_t start,now;
	printf("upload file:%s,size is:%ld\n",name,file_size);
	start=time(NULL);
	send(new_fd,&temp,temp.len+MSGSIZE,0);//发送文件大小
	while(0!=(ret=read(file_fd,&temp.msg,sizeof(temp.msg))))
	{
		temp.len=ret;
		sendn(new_fd,(char*)&temp,temp.len+MSGSIZE);
		senb+=ret;
		now=time(NULL);
		if(now-start)
		{
			pst=100*(float)senb/file_size;
			printf("\rsend byte:%ld,persent is:%5.2f%%",senb,pst);
			fflush(stdout);
			start=now;
		}
	}
	temp.len=0;
	send(new_fd,&temp,temp.len+MSGSIZE,0);
	close(file_fd);//关闭文件
	pst=100*(float)senb/file_size;
	printf("\rsend byte:%ld,persent is:%5.2f%%",senb,pst);
	fflush(stdout);
	printf("upload sucess\n");
	return;
}

void recv_file(int new_fd,char *name)
{
	off_t file_size,offset;
	int head[2];
	char path[128];
	char buf[1000];
	bzero(buf,sizeof(buf));
	int file_fd=open(name,O_RDWR|O_CREAT,0644);
	//发送文件偏移,同时lseek
	struct stat file_stat;
	fstat(file_fd,&file_stat);
	offset=file_stat.st_size;
	head[1]=5;
	head[0]=sizeof(offset);
	send(new_fd,head,sizeof(head),0);
	send(new_fd,&offset,head[0],0);//发送偏移
	lseek(file_fd,0,SEEK_END);//偏移到文件尾
	recv(new_fd,head,sizeof(head),0);
	recv(new_fd,&file_size,sizeof(off_t),0);//接收文件大小
	getcwd(path,sizeof(path));
	printf("recieve file name:%s size is=%ld offset is=%ld\n",name,file_size,offset);
	time_t start,now;
	off_t recb=0;
	float pst;//persent
	start=time(NULL);
	while(1)
	{
		recv(new_fd,head,sizeof(head),0);
		if(head[0]>0)
		{
			recvn(new_fd,buf,head[0]);
			write(file_fd,buf,head[0]);
			recb+=head[0];
			now=time(NULL);
			if(now-start)
			{
				pst=100*(float)(recb+offset)/file_size;
				printf("\rrecv byte:%ld,persent is:%5.2f%%",recb,pst);
				fflush(stdout);
				start=now;
			}
		}else{
			pst=100*(float)recb/file_size;
			printf("\rsend byte:%ld,persent is:%5.2f%%",recb,pst);
			fflush(stdout);
			printf("\ndownload sucess\n");
			close(file_fd);//关闭文件
			break;
		}
	}
}

void recv_rm(int fd)
{
	int head[2];
	recv(fd,head,sizeof(head),0);
	if(6==head[1])
	{
		printf("delect sucess\n");
	}
	if(-1==head[1])
	{
		printf("delect fail\n");
	}
	return;
}

//接收n个字节
int recvn(int fd,char *buf,int len)
{
	int sum=0;
	int ret;
	while(sum<len)
	{
		ret=recv(fd,buf+sum,len-sum,0);
		sum+=ret;
	}
}

//发送n个字节
int sendn(int fd,char *buf,int len)
{
	int sum=0;
	int ret;
	while(sum<len)
	{
		ret=send(fd,buf+sum,len-sum,0);
		sum+=ret;
	}
}

