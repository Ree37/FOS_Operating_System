// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//[PROJECT'24.MS3]
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...

	//wrap it
	struct semaphore S;
	uint32 size = sizeof(struct __semdata);
	//allocate memory
	S.semdata = smalloc(semaphoreName,size,1);
	//init data
	init_queue(&S.semdata->queue);
	S.semdata->count = value;
	S.semdata->lock = 0;
	strcpy(S.semdata->name,semaphoreName);
	return S;
}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//[PROJECT'24.MS3]
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("get_semaphore is not implemented yet");
	//Your Code is Here...
}

void wait_semaphore(struct semaphore sem)
{
	//[PROJECT'24.MS3]
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
}

void signal_semaphore(struct semaphore sem)
{
	//[PROJECT'24.MS3]
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
