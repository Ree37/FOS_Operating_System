#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_spinlock(&AllShares.shareslock, "shares lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_frames_storage is not implemented yet");
	//Your Code is Here...
 acquire_spinlock(&AllShares.shareslock);
	struct FrameInfo** frame = kmalloc(sizeof(void*) * numOfFrames);
 release_spinlock(&AllShares.shareslock);
	if(frame == NULL){
		return NULL;
	}
	return frame;

}

//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_share is not implemented yet");
	//Your Code is Here...
 acquire_spinlock(&AllShares.shareslock);
	struct Share* share = kmalloc(sizeof(struct Share));
 release_spinlock(&AllShares.shareslock);

	if(share == NULL){
		return NULL;
	}
	share->ownerID = ownerID;
	share->ID = ((uint32) share << 1) >> 1;
	int stringSize = strlen(shareName) + 1;
	for (int i = 0; i < stringSize && i < 64; ++i) {
		share->name[i]=shareName[i];
	}
	share->isWritable = isWritable;
	share->size = size;
	share->references = 1;
	uint32 noOfFrames = ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	share->framesStorage = create_frames_storage(noOfFrames);
	if(share->framesStorage == NULL){
	acquire_spinlock(&AllShares.shareslock);
		kfree(share);
	release_spinlock(&AllShares.shareslock);
		return NULL;
	}
	return share;

}

//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_share is not implemented yet");
	//Your Code is Here...
	struct Share *i;

	acquire_spinlock(&AllShares.shareslock);
	LIST_FOREACH(i, &AllShares.shares_list){
		if(i->ownerID == ownerID && strcmp(i->name,name) == 0){
			release_spinlock(&AllShares.shareslock);
			return i;
		}
	}
	release_spinlock(&AllShares.shareslock);

	return NULL;
}
//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	//Your Code is Here...


	struct Env* myenv = get_cpu_proc(); //The calling environment
	//acquire_spinlock(&AllShares.shareslock);
	struct Share *obj;
	//1. Allocate & Initialize a new share object
	if (get_share(ownerID,shareName) != NULL){
		//release_spinlock(&AllShares.shareslock);
			return E_SHARED_MEM_EXISTS;
	}

	uint32 page_num = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

	if (LIST_SIZE(&MemFrameLists.free_frame_list) < page_num) {
		//release_spinlock(&AllShares.shareslock);
		return E_NO_SHARE;
	}

	obj = create_share(ownerID,shareName,size,isWritable);

	if (obj == NULL) {
		//release_spinlock(&AllShares.shareslock);
		return E_NO_SHARE;
	} else {

	acquire_spinlock(&AllShares.shareslock);
		LIST_INSERT_HEAD(&AllShares.shares_list, obj);
	release_spinlock(&AllShares.shareslock);
	}
	//cprintf("[TRACE]: createSharedObject va: %x, page_num: %d, name: %s\n", virtual_address, page_num, obj->name);


	for(int i = 0; i < page_num; i++){
		struct FrameInfo *frame;
		acquire_spinlock(&AllShares.shareslock);
		allocate_frame(&frame);

		if (frame == NULL) {
			//release_spinlock(&AllShares.shareslock);
			panic("No free frames");
		} else {

			map_frame(myenv->env_page_directory,frame,(uint32)virtual_address,PERM_PRESENT | PERM_WRITEABLE | PERM_USER);

			obj->framesStorage[i] = frame;
			virtual_address += PAGE_SIZE;
			release_spinlock(&AllShares.shareslock);
		}
	}
	//release_spinlock(&AllShares.shareslock);
	return obj->ID;
}


//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("getSharedObject is not implemented yet");
	//Your Code is Here...


	struct Env* myenv = get_cpu_proc(); //The calling environment
	//acquire_spinlock(&AllShares.shareslock);
	struct Share* share = get_share(ownerID, shareName);

	if (share == NULL) {
		//release_spinlock(&AllShares.shareslock);
		return E_SHARED_MEM_NOT_EXISTS;
	} else {
		uint32 noOfFrames = ROUNDUP(share->size, PAGE_SIZE)/PAGE_SIZE;
		share->references++;
		acquire_spinlock(&AllShares.shareslock);
		for (int i = 0; i < noOfFrames; i++){
			uint32 perms = PERM_PRESENT | PERM_USER;
			if (share->isWritable) {
				perms = perms | PERM_WRITEABLE;
			}

			map_frame(myenv->env_page_directory, share->framesStorage[i], (uint32)virtual_address, perms);
			virtual_address += PAGE_SIZE;
		}
		release_spinlock(&AllShares.shareslock);
		return share->ID;
	}
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_share is not implemented yet");
	//Your Code is Here...
	acquire_spinlock(&AllShares.shareslock);
	LIST_REMOVE(&AllShares.shares_list,ptrShare);
	kfree(ptrShare->framesStorage);
	kfree(ptrShare);
	release_spinlock(&AllShares.shareslock);

}
//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("freeSharedObject is not implemented yet");
	//Your Code is Here...

}
