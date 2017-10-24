#ifndef __FACTORY_H__
#define __FACTORY_H__
#include"head.h"
#define MSGSIZE 8

//信息发送结构体
/*	cmd=0:exit(客户端退出)
	cmd=1:pwd(显示当前路径)
	cmd=2:ls(显示当前文件夹内容)
	cmd=3:cd(切换路径)
	cmd=4:puts(上传)
	cmd=5:gets(下载)
	cmd=6:rm(删除服务器文件)
	cmd=100:new(新建用户)
	cmd=101:login(用户登录)
	cmd=102:update(更改密码)
*/
struct msg_send{
	int len;//msg长度
	int cmd;//命令
	char msg[1000];//命令参数，文件内容等
};

//队列节点
struct que_msg{
	int fd;
	struct que_msg *pnext;
};

//队列信息
struct que_info{
	int cap;//容量
	int size;//当前大小
	struct que_msg *phead,*ptail;
};

typedef void*(*pfunc)(void*);

//线程信息
struct pth_info{
	int pth_num;
	pthread_t *pthid;//指向存放pthid数组的空间
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pfunc pthfunc;//线程实现函数
};

//主线程与子线程传递信息结构体
struct factory{
	struct que_info que;
	struct pth_info pth;
};

void history(int,int,char*);

int que_insert(struct que_info*,struct que_msg*);

void que_get(struct que_info*,struct que_msg**);

void que_del(struct que_info*);

void factory_init(struct factory*,int,int);

void factory_create(struct factory*);

void send_pwd(int,char*);

void send_ls(int,char*);

void send_cd(int,char*,char*);

void recv_file(int,char*);

void send_file(int,char*);

void send_rm(int,char*);

void send_err(int);

int net_init(int*,in_addr_t,uint16_t);

void recv_cmd(int,int*,char*);

int recvn(int,char*,int);

int sendn(int,char*,int);

int user_new(int,char*);

int user_login(int,char*);

#endif
