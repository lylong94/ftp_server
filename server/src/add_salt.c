#include"factory.h"
#include"mysql.h"
//passwd14位,salt11位，存放+1,放'\0'

//生成n位随机数,传入传出参数
void get_rand(char *randm,int n)
{
	char str[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	int i,slen;
	slen=strlen(str);
	srand(time(NULL));
	for(i=0;i<n;i++)
	{
		randm[i]=str[rand()%slen];
	}
	return;
}

//加盐,返回密文(客户端,用户端用不上)
//char *salt_add(char *passwd,char *salt)
//{
//	char randm[9];
//	bzero(randm,sizeof(randm));
//	get_rand(randm,8);
//	sprintf(salt,"$6$%s",randm);
//	return crypt(passwd,salt);
//}

//用户14,密码14，盐11位,cmd=100,成功1,用户名存在0,失败-1
int user_new(int fd,char *name)
{
	int ret,head[2];
	struct msg_send msg;
	bzero(&msg,sizeof(msg));
	char randm[9]={0};
	char salt[12]={0};
	char salted[128]={0};
	ret=mysql_isuser(name);//查询用户名是否存在
	if(ret>0)
	{
		return 0;//用户名存在
	}
	if(-1==ret)
	{
		return -1;
	}
	get_rand(randm,8);//用户名不存在,生成盐
	sprintf(salt,"$6$%s",randm);
	msg.len=strlen(salt);
	msg.cmd=100;
	strcpy(msg.msg,salt);
	send(fd,&msg,msg.len+MSGSIZE,0);//发送盐
	recv(fd,head,sizeof(head),0);
	if(-1==head[1])
	{
		return -1;
	}
	recv(fd,salted,head[0],0);//收密文
	ret=mysql_insert(name,salt,salted);//用户插入数据库
	if(ret)//插入失败
	{
		return -1;
	}
	return 1;//插入成功,用户创建成功
}

//cmd=101,用户登录,成功id>0,帐密错误0,程序错误-1
int user_login(int fd,char *name)
{
	struct msg_send msg;
	msg.cmd=101;
	char salt[12]={0};
	int id,ret,head[2];
	char salted[128]={0};
	char recv_pswd[128]={8};
	id=mysql_isuser(name);//用户名是否存在
	if(0==id)
	{
		return 0;//账户错误
	}
	if(-1==id)
	{
		return -1;
	}
	ret=mysql_select("salt",id,salt);
	if(-1==ret)
	{
		return -1;
	}
	strcpy(msg.msg,salt);
	msg.len=strlen(salt);
	send(fd,&msg,msg.len+MSGSIZE,0);
	ret=mysql_select("password",id,salted);
	recv(fd,head,sizeof(head),0);
	recv(fd,recv_pswd,head[0],0);//接受客户端发送的密文
	if(ret==-1)
	{
		return -1;
	}
	if(!strcmp(recv_pswd,salted))
	{
		return id;//登录成功
	}else{
		return 0;//密码错误
	}
}
