#include"factory.h"

struct clean{
	int fd;
	pthread_mutex_t *pmutex;
	pthread_cond_t *pcond;
};

//线程清理函数
void cleanup(void *p)
{
	struct clean *cln=(struct clean*)p;
	pthread_mutex_unlock(cln->pmutex);
	if(cln->fd)
	{
		close(cln->fd);
	}
}

//线程等待函数
void *pth_wait(void *pmain)
{
	struct factory *pfac=(struct factory*)pmain;
	struct clean cln;
	bzero(&cln,sizeof(cln));
	struct que_msg *pcur;//存放队列元素
	struct msg_send msg;
	cln.pcond=&pfac->pth.cond;
	cln.pmutex=&pfac->pth.mutex;
	int id;//用户id
	int cmd,ret;
	char buf[128];//存放参数
	char path[128];//路径
	char pathname[128];//文件等的绝对路径
	pthread_cleanup_push(cleanup,&cln);
	while(1)
	{
		getcwd(path,sizeof(path));//添加path分离个线程地址
		pthread_mutex_lock(cln.pmutex);
		if(!pfac->que.size)
		{
			pthread_cond_wait(cln.pcond,cln.pmutex);
		}
		que_get(&pfac->que,&pcur);
		pthread_mutex_unlock(cln.pmutex);
		cln.fd=pcur->fd;//客户端fd
		while(1)//连接客户端,添加账户密码验证
		{
			bzero(buf,sizeof(buf));
			recv_cmd(cln.fd,&cmd,buf);//得到命令，参数
			if(100==cmd)//新建用户
			{
				ret=user_new(cln.fd,buf);
				msg.len=0;
				if(1==ret)//创建成功
				{
					msg.cmd=100;
				}else if(0==ret){//创建失败,用户名存在
					msg.cmd=0;
				}else{//ret==-1,创建失败,fail
					msg.cmd=-1;
				}
				send(cln.fd,&msg,MSGSIZE,0);
			}else if(101==cmd){//登录
				id=user_login(cln.fd,buf);
				printf("id=%d\n",id);
				msg.len=0;
				if(0<id)//登陆成功
				{
					msg.cmd=101;
					send(cln.fd,&msg,MSGSIZE,0);
					break;
				}else if(0==id){//帐号密码错误
					msg.cmd=0;
				}else{//ret=-1,程序执行错误
					msg.cmd=-1;
				}
				send(cln.fd,&msg,MSGSIZE,0);
			}else{//cmd==0,退出
				goto laber;
			}
		}
		while(1)//保持连接，接收命令
		{
			bzero(buf,sizeof(buf));
			recv_cmd(cln.fd,&cmd,buf);//得到命令，参数
			if(cmd)
			{
				switch(cmd)//执行命令
				 {
					 case 1:send_pwd(cln.fd,path);
							history(id,6,path);
							break;
					 case 2:send_ls(cln.fd,path);
							history(id,7,path);
							break;
					 case 3:send_cd(cln.fd,buf,path);
							history(id,8,buf);
							break;
					 case 4:bzero(pathname,sizeof(pathname));
							sprintf(pathname,"%s/%s",path,buf);
							history(id,9,pathname);
							recv_file(cln.fd,pathname);//接受文件
							history(id,10,pathname);
							break;
					 case 5:bzero(pathname,sizeof(pathname));
							sprintf(pathname,"%s/%s",path,buf);
							history(id,11,pathname);
							send_file(cln.fd,pathname);//发送
							history(id,12,pathname);
							break;
					 case 6:bzero(pathname,sizeof(pathname));
							sprintf(pathname,"%s/%s",path,buf);
							send_rm(cln.fd,pathname);
							history(id,13,pathname);
							break;
					 default:send_err(cln.fd);break;
				 }
			}else{
				break;
			}
		}
laber:
		close(cln.fd);
		cln.fd=0;
	}
	pthread_cleanup_pop(1);
}

//线程结构体初始化
void factory_init(struct factory *pfac,int pth_num,int que_cap)
{
	bzero(pfac,sizeof(struct factory));
	pfac->que.cap=que_cap;
	pfac->pth.pth_num=pth_num;
	pfac->pth.pthid=(pthread_t*)calloc(pth_num,sizeof(pthread_t));
	pthread_mutex_init(&pfac->pth.mutex,NULL);
	pthread_cond_init(&pfac->pth.cond,NULL);
	pfac->pth.pthfunc=pth_wait;
}

//线程创建函数
void factory_create(struct factory *pfac)
{
	int i;
	pthread_t pthid;
	for(i=0;i<pfac->pth.pth_num;i++)
	{
		pthread_create(&pfac->pth.pthid[i],NULL,pfac->pth.pthfunc,pfac);
	}
	printf("create pthread sucess\n");
}
