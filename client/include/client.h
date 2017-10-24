#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "head.h"

#define MSGSIZE 8

struct msg_send{
	int len;//msg长度
	int cmd;//命令
	char msg[1000];//参数等数据
};

int recvn(int,char*,int);

int sendn(int,char*,int);

void send_cmd(int,int,char*);

void recv_pwd(int);

void recv_ls(int);

void send_file(int,char*);

void recv_file(int,char*);

void recv_rm(int);

int user_login(int);

int user_new(int);

#endif
