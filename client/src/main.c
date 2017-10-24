#include "client.h"

//得到命令
int getnum(char *s)
{
	if(!strcmp(s,"exit")) return 0;
	if(!strcmp(s,"pwd")) return 1;
	if(!strcmp(s,"ls")) return 2;
	if(!strcmp(s,"cd")) return 3;
	if(!strcmp(s,"puts")) return 4;
	if(!strcmp(s,"gets")) return 5;
	if(!strcmp(s,"rm")) return 6;
	return -1;
}

//登录程序成功返回1,失败返回0
int  user_login(int fd)
{
	printf("请输入用户名:\n");
	int ret;
	int head[2]={0,101};
	char name[15]={0};
	char salt[15]={0};
	char *passwd;
	char *salted;
	read(0,name,sizeof(name));
	head[0]=strlen(name)-1;//有\n
	send(fd,head,sizeof(head),0);
	send(fd,name,head[0],0);
	recv(fd,head,sizeof(head),0);
	if(101==head[1])
	{
		recv(fd,salt,sizeof(salt),0);
		passwd=getpass("请输入密码\n");//没有\n
		salted=crypt(passwd,salt);
		head[0]=strlen(salted);
		send(fd,head,sizeof(head),0);
		send(fd,salted,head[0],0);
		printf("send sucess\n");
		recv(fd,head,sizeof(head),0);
		printf("recv %d\n",head[1]);
		if(101==head[1])
		{
			printf("登陆成功\n");
			return 1;
		}else if(0==head[1])
		{
			printf("账户密码错误\n");
			return 0;
		}else{
			printf("系统错误\n");
			return 0;
		}
	}else if(0==head[1]){
		printf("账户密码错误\n");
		return 0;
	}else{
		printf("系统错误\n");
		return 0;
	}
}

//注册程序成功1,失败0
int user_new(int fd)
{
	printf("请输入用户名(最多14位):\n");
	int ret;
	int head[2]={0,100};
	char name[15]={0};
	char salt[15]={0};
	char *passwd1,*passwd2;
	char *salted;
	read(0,name,sizeof(name));
	head[0]=strlen(name)-1;
	send(fd,head,sizeof(head),0);
	send(fd,name,head[0],0);//发送用户名
	recv(fd,head,sizeof(head),0);
	if(100==head[1])//正确
	{
		recv(fd,salt,sizeof(salt),0);//接收盐
		passwd1=getpass("请输入密码(最多14位):\n");
		if(strlen(passwd1)>14)
		{
			printf("密码超过14位\n");
			return 0;
		}
		passwd2=getpass("请再次输入密码(与上次相同):\n");
		if(strcmp(passwd1,passwd2))
		{
			printf("2次输入密码不同\n");
			return 0;
		}
		salted=crypt(passwd1,salt);
		head[0]=strlen(salted);
		send(fd,head,sizeof(head),0);
		send(fd,salted,head[0],0);//发送密文
		recv(fd,head,sizeof(head),0);
		if(100==head[1])
		{
			return 1;
		}else{
			printf("系统错误\n");
			return 0;
		}
	}else if(0==head[0]){
		printf("用户名已存在!\n");
		return 0;
	}else{
		printf("系统错误\n");
		return 0;
	}
}

int main(int argc,char *argv[])
{
	if(3!=argc)
	{
		printf("error args\n");
		return -1;
	}
	int ret;
	int sfd=socket(AF_INET,SOCK_STREAM,0);//生成套接口
	if(-1==sfd)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=inet_addr(argv[1]);
	addr.sin_port=htons(atoi(argv[2]));
	ret=connect(sfd,(struct sockaddr*)&addr,sizeof(addr));
	if(-1==ret)
	{
		perror("connect");
		return -1;
	}
	int cmd,head[2];
	char buf[128];
	char delim1=' ';
	char delim2='\n';
	char *result;
	//登录,cmd=0(退出),cmd=100(新用户),cmd=101(登录)
	while(1)
	{
		printf("请选择所需要的操作序号:1.登录 2.注册新用户 0.退出\n");
		bzero(buf,sizeof(buf));
		read(0,buf,sizeof(buf));
		cmd=atoi(buf);
		if(1==cmd)//登录程序
		{
			ret=user_login(sfd);
			if(ret)
			{
				break;
			}else{
				continue;
			}
		}else if(2==cmd){//注册
			ret=user_new(sfd);
			if(0==ret)
			{
				head[1]=-1;
				send(sfd,head,sizeof(head),0);
				recv(sfd,head,sizeof(head),0);
				printf("注册失败\n");
			}else{
				printf("注册成功\n");
			}
			continue;
		}else if(0==cmd){
			goto laber;
		}else{
			printf("请输入正确的序号！\n");
		}
	}
	while(1)
	{
		printf("please enter the cmd:\n");
		bzero(buf,sizeof(buf));
		result=NULL;
		read(0,buf,sizeof(buf));
		result=strtok(buf,&delim1);
		ret=getnum(result);
		result=strtok(NULL,&delim1);
		if(NULL!=result)
		{
			result=strtok(result,&delim2);
		}
		if(ret==-1)
		{
			printf("error command\n");
		}else if(ret==0){
			break;//结束
		}else{
			send_cmd(sfd,ret,result);
		}
	}
laber:
	head[0]=0;
	head[1]=0;
	send(sfd,head,sizeof(head),0);
	close(sfd);//结束程序
	return 0;
}
