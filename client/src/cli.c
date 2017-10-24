#include "client.h"

//根据不同的cmd执行不同行动
void send_cmd(int fd,int cmd,char *buf)
{
	struct msg_send msg;
	msg.cmd=cmd;
	if(cmd<=2)
	{
		if(NULL!=buf)
		{
			printf("error args\n");
			return;
		}
		msg.len=0;
		send(fd,&msg,MSGSIZE,0);
	}else{
		if(NULL==buf)
		{
			printf("error args\n");
			return;
		}
		bzero(msg.msg,sizeof(msg.msg));
		strcpy(msg.msg,buf);
		msg.len=strlen(msg.msg);
		send(fd,&msg,msg.len+MSGSIZE,0);
	}
	switch(cmd){//接收数据
	case 1:case 3:recv_pwd(fd);break;
	case 2:recv_ls(fd);break;
	case 4:send_file(fd,buf);break;
	case 5:recv_file(fd,buf);break;
	case 6:recv_rm(fd);break;
	}
	return;
}
