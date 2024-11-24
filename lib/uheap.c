#include <inc/lib.h>

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);

}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================

struct program_size {
	uint32 size  ;
	void *start ;
};
struct program_size prog[2000] = {0};
///////////////////////////////////////////////////////////
void* firstva(uint32 num , uint32 start){
	uint32 count = 0;
    void* va = NULL;


	for(uint32 i = start ; i < (uint32) USER_HEAP_MAX ; i+=PAGE_SIZE){
		bool mark = 0;

		for(int x = 0 ; x<2000 ; x++){
		     if (prog[x].start == (void*)i){
		    	 i = (uint32)prog[x].start + (prog[x].size*PAGE_SIZE) - PAGE_SIZE;
		    	 mark = 1;
		    	 count = 0;
		    	 break;

		     }
		}

	     if (mark == 0) {
	    	 if (count == 0){
	    		 va =(void*) i;
	    	 }
	    	 count++;
	    	 if (count == num){

	    		 break;
	    	 }
	     }
	}
	if (count == num){
		for(int x = 0 ; x<2000 ; x++){
		    if (prog[x].size == 0){
		         prog[x].size = num;
		         prog[x].start = va;
		         break;
		    }
		}
		return va;

	}
	else {
		return NULL;
	}
}

void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	//return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy

	uint32 hard = (uint32) myEnv->hard_limit;
    uint32 total = hard + PAGE_SIZE;
	uint32 *start_page_alloc =(uint32*) total ; // start of page alloc

	uint32 MAX =(uint32) USER_HEAP_MAX - (uint32)start_page_alloc; // size of page alloc
	uint32 num_of_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	if(size > MAX){

		 return NULL;
	}

	if (size <= (PAGE_SIZE/2)){ // block allocator
	    void * alloc_block =(void*) alloc_block_FF(size);
		return alloc_block;
	}

	void* alloc_page = firstva(num_of_pages ,(uint32)start_page_alloc);
	if (alloc_page == NULL){
		 return NULL;
	}
	sys_allocate_user_mem((uint32)alloc_page , size);
	return alloc_page;
	}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	if (virtual_address == NULL || virtual_address == (void*)myEnv->hard_limit || virtual_address < (void*)myEnv->UserHeapStart || virtual_address >(void*)USER_HEAP_MAX){
			panic("Invalid address");
		}
	uint32 size = 0;
	uint32 index;
	if (virtual_address < (void*)myEnv->segment_break ){
	    free_block(virtual_address);
    }

	else {
	      for (uint32 x = 0 ; x<2000 ; x++ ){
			if (prog[x].start == virtual_address){
				size = prog[x].size;
				index = x;
					break;
			}
	     }
	      prog[index].size = 0;
	      prog[index].start = NULL;
	     sys_free_user_mem((uint32)virtual_address , size);

	}

}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
