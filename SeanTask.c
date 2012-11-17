//static struct StTask* _task = 0;
#include "dlink.h"

typedef int (*rd)(void*, char* , int);

struct StTask
{
	int Fd;
	int TaskType;
	rd readn;
};

void* InitTask()
{
	return dl_head();
}

struct StTask * CreateTask(int fd)
{
	struct StTask* ptask = dl_new(struct StTask);
	dl_init(ptask);

	ptask->Fd = fd;
	ptask->TaskType = 0;
	ptask->readn = NULL;

	return ptask;
}

void RealseTask(struct StTask* task)
{
	dl_free(task);
}

void AddTask(void* head, struct StTask* task)
{
	dl_add(head , task);
}

void RemoveTask(struct StTask* task)
{
	dl_del(task);
}

