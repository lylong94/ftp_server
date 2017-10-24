#include"factory.h"

//插入队列
int que_insert(struct que_info *pque,struct que_msg *new)
{
	if(pque->size==pque->cap)
	{
		return -1;
	}
	if(pque->size==0)
	{
		pque->phead=new;
		pque->ptail=new;
		pque->size++;
	}else{
		pque->ptail->pnext=new;
		pque->ptail=new;
		pque->size++;
	}
	return 0;
}

//队列取数据
void que_get(struct que_info *pque,struct que_msg**ppnod)
{
	*ppnod=pque->phead;
	pque->phead=pque->phead->pnext;
	pque->size--;
	return;
}

//删除队列
void que_del(struct que_info *pque)
{
	if(!pque->size)
	{
		return;
	}
	struct que_msg *p;
	while(NULL!=pque->phead)
	{
		p=pque->phead;
		pque->phead=pque->phead->pnext;
		free(p);
	}
	return;
}
