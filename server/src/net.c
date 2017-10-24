#include"factory.h"

//网络初始化
int net_init(int *fd,in_addr_t ip,uint16_t port)
{
	int ret;
	*fd=socket(AF_INET,SOCK_STREAM,0);//生成套接字
	if(-1==ret)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in addr;
	int addrlen=sizeof(struct sockaddr);
	bzero(&addr,addrlen);
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=ip;
	addr.sin_port=port;
	ret=bind(*fd,(struct sockaddr*)&addr,addrlen);
	if(-1==ret)
	{
		perror("bind");
		return -1;
	}
	return 0;
}

//得到客户端命令，参数
void recv_cmd(int fd,int *pcmd,char *pbuf)
{
	int len;
	recv(fd,&len,4,0);
	recv(fd,pcmd,4,0);
	if(len!=0)
	{
		recvn(fd,pbuf,len);
	}
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
