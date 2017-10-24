#include"factory.h"
#define HISFILE ".server_history"

int his_fd;
int exit_fd[2];

static char event[][20]={"system start","system shutdown","new user","login","exit","change password","pwd","ls","cd","puts start","puts over","gets start","gets over","rm"};

//历史记录函数
void history(int id,int num,char *param)
{
	static pthread_mutex_t mutex;
	time_t now=time(NULL);
	char buf[256]={0};
	if(NULL!=param)
	{
		sprintf(buf,"id=%d %-20s %-20s %s",id,event[num],param,ctime(&now));
	}else{
		sprintf(buf,"id=%d %-41s %s",id,event[num],ctime(&now));
	}
	pthread_mutex_lock(&mutex);
	write(his_fd,buf,strlen(buf));
	pthread_mutex_unlock(&mutex);
	return;
}

//中断执行函数
void sig(int num)
{
	printf("start close\n");
	write(exit_fd[1],&num,sizeof(int));
	return;
}

int main(int argc,char *argv[])
{
	if(5!=argc)
	{
		printf("%s:need args like this:\nIP PORT PTHREAD_SIZE QUE_CUP\n",argv[0]);
		return -1;
	}
	int ret;
	pipe(exit_fd);
	his_fd=open(HISFILE,O_WRONLY|O_APPEND|O_CREAT,0640);
	struct factory msg;
	factory_init(&msg,atoi(argv[3]),atoi(argv[4]));
	factory_create(&msg);
	int sfd;//监听端口
	ret=net_init(&sfd,inet_addr(argv[1]),htons(atoi(argv[2])));
	if(-1==ret)
	{
		return -1;
	}
	int epfd=epoll_create(1);
	struct epoll_event event,evs[2];
	bzero(&event,sizeof(event));
	bzero(evs,sizeof(evs));
	event.events=EPOLLIN;
	event.data.fd=sfd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&event);//epoll注册
	event.data.fd=exit_fd[0];
	epoll_ctl(epfd,EPOLL_CTL_ADD,exit_fd[0],&event);//注册结束端口
	int new_fd;
	int i,qret;//插入队列的返回值
	struct que_msg *pnod;
	ret=listen(sfd,10);//准备好再监听
	if(-1==ret)
	{
		perror("listen");
		return -1;
	}
	history(0,0,NULL);//系统启动
	printf("open sucess\n");
	signal(SIGINT,sig);//注册中断
	while(1)
	{
		bzero(evs,sizeof(evs));
		ret=epoll_wait(epfd,evs,2,-1);//等待
		for(i=0;i<ret;i++)
		{
			if(evs[i].data.fd==exit_fd[0])
			{
				printf("proc receive\n");
				close(sfd);//关闭连接请求
				goto exit;
			}
			if(evs[i].data.fd==sfd)
			{
				new_fd=accept(sfd,NULL,NULL);
				pnod=(struct que_msg*)calloc(1,sizeof(struct que_msg));
				pnod->fd=new_fd;
				pthread_mutex_lock(&msg.pth.mutex);//加锁
				qret=que_insert(&msg.que,pnod);
				pthread_mutex_unlock(&msg.pth.mutex);
				pthread_cond_signal(&msg.pth.cond);//激发子线程
				if(qret)//如果满了，删除节点
				{
					close(new_fd);
					free(pnod);
				}
			}
		}
	}
exit:
	printf("port close\n");
	for(i=0;i<msg.pth.pth_num;i++)
	{
		pthread_cancel(msg.pth.pthid[i]);
	}
	for(i=0;i<msg.pth.pth_num;i++)
	{
		pthread_join(msg.pth.pthid[i],NULL);
	}
	pthread_cond_destroy(&msg.pth.cond);
	pthread_mutex_destroy(&msg.pth.mutex);
	que_del(&msg.que);
	free(msg.pth.pthid);
	printf("join sucess\nexit\n");
	history(0,1,NULL);//结束程序
	close(his_fd);
	return 0;
}
