// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...

	//wrap it
	struct semaphore sem;
		uint32 size = sizeof(struct __semdata);
		//allocate memory
		sem.semdata = smalloc(semaphoreName,size,1);
		//init data
		sys_init_queue(&sem.semdata->queue);
		sem.semdata->count = value;
		sem.semdata->lock = 0;
		strcpy(sem.semdata->name,semaphoreName);
		return sem;
}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...
	struct semaphore sem;
	sem.semdata = sget(ownerEnvID,semaphoreName);
	return sem;
}

void wait_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
	uint32 key = 1;
		while (xchg(&sem.semdata->lock,key) != 0);
		sem.semdata->count--;
		if (sem.semdata->count < 0) {
			sys_sleep(&sem.semdata->queue, &sem.semdata->lock);
		}
		sem.semdata->lock = 0;
}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
	//init key = 1
		uint32 key = 1;
		//do xchg(&keys, &s.lock) while (keyw != 0)
		while (xchg(&sem.semdata->lock,key) != 0);
		//increment for count()
		sem.semdata->count++;
		//if (s.count <= 0)
		if(sem.semdata->count <= 0){
			//1.remove the proccess from the s.queue
			//2.put the proccess to the ready list
			sys_insert_ready(&sem.semdata->queue);
		}
		//se the s.lock = 0
		sem.semdata->lock = 0;
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
