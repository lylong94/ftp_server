#include"factory.h"

//发送当前路径
void send_pwd(int fd,char *path)
{
	struct msg_send msg;
	msg.cmd=1;
	strcpy(msg.msg,path);//发送path
	msg.len=strlen(msg.msg);
	send(fd,&msg,msg.len+MSGSIZE,0);
}

struct ls_t{
	long size;
	char type;
	char name[128];
};

//显示当前目录下的文件信息:type,size,name
void send_ls(int fd,char *path)
{
	struct msg_send msg;
	msg.cmd=2;
	DIR *dir=NULL;
	struct dirent *rent;
	struct stat buf;
	struct ls_t file;
	char pathname[128];
	dir=opendir(path);//打开当前目录
	while(NULL!=(rent=readdir(dir)))
	{
		if(strcmp(rent->d_name,".")&&strcmp(rent->d_name,".."))
		{
			sprintf(pathname,"%s/%s",path,rent->d_name);
			stat(pathname,&buf);//查看文件信息
			if(S_ISDIR(buf.st_mode))
			{
				file.type='d';//类型
			}else{
				file.type='-';
			}
			file.size=buf.st_size;//大小
			strcpy(file.name,rent->d_name);//名字
			msg.len=9+strlen(rent->d_name);
			memcpy(msg.msg,&file,msg.len);
			sendn(fd,(char*)&msg,msg.len+MSGSIZE);
		}
	}
	msg.len=0;
	send(fd,&msg,MSGSIZE,0);//发送长度为0,发送结束
	return;
}

//改变目录,并发送切换至的目录,buf:参数,path:当前路径
void send_cd(int fd,char *buf,char *path)
{
	char newpath[128]={0};
	if(!strncmp(buf,"/",1))
	{
		strcpy(newpath,buf);
	}else if(!strcmp(buf,"..")){
		int i,n;
		int len=strlen(path);
		for(i=0;i<len;i++)//找到最后一个'/'
		{
			if('/'==path[i])
			{
				n=i;
			}
		}
		strncpy(newpath,path,n);//斜杠不要
	}else{
		sprintf(newpath,"%s/%s",path,buf);
	}
	DIR *dir=opendir(newpath);
	if(NULL==dir)//不能打开,不合法
	{
		send_err(fd);//切换失败发送错误
		return;
	}
	closedir(dir);//合法
	strcpy(path,newpath);//新地址
	send_pwd(fd,path);
	return;
}

//接收客户端上传文件
void recv_file(int new_fd,char *name)
{
	off_t file_size;
	int head[2];
	char buf[1000];
	bzero(buf,sizeof(buf));
	int file_fd=open(name,O_RDWR|O_CREAT,0644);
	recv(new_fd,head,sizeof(head),0);
	recv(new_fd,&file_size,sizeof(off_t),0);
	printf("recieve file:%s size is=%ld\n",name,file_size);
	while(1)
	{
		recv(new_fd,head,sizeof(head),0);
		if(head[0]>0)
		{
			recvn(new_fd,buf,head[0]);
			write(file_fd,buf,head[0]);
		}else{
			printf("\ndownload sucess\n");
			close(file_fd);
			break;
		}
	}
}

//发送文件
void send_file(int new_fd,char *name)
{
	int file_fd=open(name,O_RDWR);
	if(-1==file_fd)
	{
		send_err(new_fd);
		return;
	}
	struct msg_send temp;
	struct stat f_stat;
	int ret;
	off_t offset,size;
	int head[2];
	recv(new_fd,head,sizeof(head),0);
	recv(new_fd,&offset,sizeof(offset),0);//接收偏移
	bzero(&temp,sizeof(temp));
	temp.cmd=5;
	bzero(&f_stat,sizeof(f_stat));
	fstat(file_fd,&f_stat);
	size=f_stat.st_size;//文件大小
	temp.len=sizeof(off_t);
	memcpy(temp.msg,&size,sizeof(off_t));
	send(new_fd,&temp,temp.len+MSGSIZE,0);//发送文件大小
	if(0==size>>26)//没有64M
	{
		lseek(new_fd,offset,SEEK_SET);
		while(0!=(ret=read(file_fd,&temp.msg,sizeof(temp.msg))))
		{
			temp.len=ret;
			sendn(new_fd,(char*)&temp,temp.len+MSGSIZE);
		}
	}else{//大文件mmap
		char *mp=(char*)mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,file_fd,0);
		head[1]=5;
		while(offset<size)
		{
			if(1000<size-offset)
			{
				head[0]=1000;
			}else{
				head[0]=size-offset;
			}
			send(new_fd,head,sizeof(head),0);//发送数据头len+cmd
			sendn(new_fd,mp+offset,head[0]);//发送数据msg.msg
			offset+=head[0];//偏移
		}
		munmap(mp,size);
	}
	temp.len=0;
	send(new_fd,&temp,temp.len+MSGSIZE,0);
	close(file_fd);
	return;
}

void send_rm(int fd,char *name)
{
	if(unlink(name))
	{
		send_err(fd);
		return;
	}
	struct msg_send temp;
	temp.len=0;
	temp.cmd=6;
	send(fd,&temp,temp.len+MSGSIZE,0);
}

void send_err(int fd)
{
	struct msg_send temp;
	temp.len=0;
	temp.cmd=-1;
	send(fd,&temp,temp.len+MSGSIZE,0);
	return;
}
