#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

struct spinlock kheap; // lock

//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
		//TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
		// Write your code here, remove the panic and write your code
		//panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");

	    if((initSizeToAllocate+daStart)>daLimit){
	        panic("initial size to allocate exceeds the hard limit");
	     }

	KernHeapStart = (void *)daStart;
	segment_break = (void *)(daStart+initSizeToAllocate);
	hard_limit = (void *)daLimit;

	//init_spinlock(&kheap, "Data of kheap");

	for(uint32 i = daStart ; i < (uint32)segment_break ; i+=PAGE_SIZE )
		{
		   struct FrameInfo * FrameWillBeMapped = NULL;
		   int ret = allocate_frame(&FrameWillBeMapped);
		   if(ret!=0)
			  panic("No allocated Frames");
		   map_frame(ptr_page_directory,FrameWillBeMapped,i, PERM_WRITEABLE);
		}

	initialize_dynamic_allocator(daStart,initSizeToAllocate);

		return 0;
}


void* sbrk(int numOfPages)
{
		//MS2: COMMENT THIS LINE BEFORE START CODING==========
		//return (void*)-1 ;
		//====================================================

		//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
		// Write your code here, remove the panic and write your code
		//panic("sbrk() is not implemented yet...!!");

    if (numOfPages == 0) {
        return segment_break;
    }

    uint32 current_brk = (uint32)segment_break;
    uint32 new_brk = current_brk + (numOfPages * PAGE_SIZE);


    if (new_brk >(uint32) hard_limit) {
        return (void*)-1;
    }

   	    uint32 old_brk = current_brk;
   	    for (int i = 0; i < numOfPages; i++) {

   	        struct FrameInfo* frame = NULL;
   	        int ret = allocate_frame(&frame);
   	        if (ret == E_NO_MEM) {
   	        }

   	    int r = map_frame(ptr_page_directory,frame,current_brk, PERM_WRITEABLE);
   	    if (r == E_NO_MEM){
   	 		free_frame(frame);
   	 		return (void*) -1 ;
   	 	}

   	        current_brk += PAGE_SIZE;
   	    }


      //move break
    segment_break =(void*) new_brk;

    return (void*)old_brk;
}

struct program_size {
	uint32 size  ;
	void *start ;
};
struct program_size prog[8000] = {0}; // array to  store start va and num of pages


///////////////////////////////////////////////////////////
void* firstva(uint32 num , uint32 start){

	uint32 count = 0;
    void* va = NULL;


    for(uint32 i = start ; i < (uint32)KERNEL_HEAP_MAX  ; i+=PAGE_SIZE){
    		bool mark = 0;

    		for(int x = 0 ; x<8000 ; x++){
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
		for(int x = 0 ; x<8000 ; x++){
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

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	     //acquire_spinlock(&kheap);


	uint32 hard = (uint32) hard_limit;
	uint32 total = hard + PAGE_SIZE;
    uint32 *start_page_alloc =(uint32*) total ; // start of page alloc

	uint32 MAX =(uint32) KERNEL_HEAP_MAX - (uint32)start_page_alloc; // size of page alloc
	uint32 num_of_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	if(size > MAX){
        //release_spinlock(&kheap);
	    return NULL;
	 }
	 acquire_spinlock(&MemFrameLists.mfllock);
	 if (size <= (PAGE_SIZE/2)){ // block allocator
		   void * alloc_block =(void*) alloc_block_FF(size);
		 release_spinlock(&MemFrameLists.mfllock);
	       return alloc_block;
	 }

     void* alloc_page = firstva(num_of_pages ,(uint32)start_page_alloc);
     void* va = alloc_page;
     if (alloc_page != NULL){
    	 while (num_of_pages > 0){
    	 uint32 *ptr_page_table = NULL;
    	 struct FrameInfo *ptr_frame_info = get_frame_info(ptr_page_directory ,(uint32)alloc_page , &ptr_page_table);
    	 int alloc = allocate_frame(&ptr_frame_info);
    	 if (alloc == E_NO_MEM){
    		 release_spinlock(&MemFrameLists.mfllock);
    	     return NULL ;
    	  }
    	 int r = map_frame(ptr_page_directory,ptr_frame_info,(uint32)alloc_page, PERM_WRITEABLE|PERM_PRESENT);
    	 num_of_pages--;
    	 alloc_page+=PAGE_SIZE;

    	 }
    	 release_spinlock(&MemFrameLists.mfllock);
    	 return va ;
     }
     release_spinlock(&MemFrameLists.mfllock);
     return NULL;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details
	if (virtual_address == NULL || virtual_address == (void*)hard_limit || virtual_address < (void*)KernHeapStart || virtual_address >(void*)KERNEL_HEAP_MAX ){
		panic("Invalid address");
	}
	 //acquire_spinlock(&kheap);

     uint32 size = 0;
     uint32 index;

	if (virtual_address < (void*)segment_break ){
		//release_spinlock(&kheap);
		free_block(virtual_address);
	}



	else {
		for (uint32 x = 0 ; x<8000 ; x++ ){
			if (prog[x].start == virtual_address){
				size = prog[x].size;
				index = x;
				break;
			}
		}
		uint32 va = (uint32)virtual_address;
		while (size > 0){
			unmap_frame(ptr_page_directory, va);
			va+=PAGE_SIZE;
			size--;

		}

		prog[index].size = 0;
		prog[index].start = NULL;
       	//release_spinlock(&kheap);

	}
}
unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//[PROJECT'24.MS2] [KERNEL HEAP] kheap_physical_address
		// Write your code here, remove the panic and write your code
		uint32 *ptr_page_table = NULL ;
		get_page_table(ptr_page_directory,virtual_address,&ptr_page_table);
		struct FrameInfo * VirtualAddressFrame = get_frame_info(ptr_page_directory,virtual_address,&ptr_page_table);
		if(VirtualAddressFrame==NULL)
			return 0 ;
		uint32 PhysicalAddress = to_physical_address(VirtualAddressFrame)+ PGOFF(virtual_address);
		return PhysicalAddress;
		//return the physical address corresponding to given virtual_address
		//refer to the project presentation and documentation for details

		//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	// Write your code here, remove the panic and write your code
	struct FrameInfo* FrameData = to_frame_info(physical_address);
	if (FrameData->virtualAddress == 0)
	{ return 0;   }
	return (FrameData->virtualAddress+PGOFF(physical_address));
	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}
//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, if moved to another loc: the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().


void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	//return NULL;
	//panic("krealloc() is not implemented yet...!!");
	uint32 hard = (uint32) hard_limit;
	uint32 total = hard + PAGE_SIZE;
	uint32 *start_page_alloc =(uint32*) total ; // start of page alloc
	uint32 num_of_pages_old = 0;
	uint32 index ;
	uint32 num_of_pages_new = (new_size + PAGE_SIZE - 1) / PAGE_SIZE;

	 //case (1) va = va & size = 0
	if (virtual_address != NULL && new_size == 0){
		if (virtual_address >=(void*) start_page_alloc && virtual_address < (void*)KERNEL_HEAP_MAX){
			kfree(virtual_address);  //page allocator
			return NULL;
		}
		else{
			free_block(virtual_address); //block allocator
			return NULL;
		}
	}

	// case (2) va = NULL & size = 0
     if (virtual_address == NULL && new_size == 0){
		return NULL;
	}

	// case (3) va = NULL & size = size
	 if (virtual_address == NULL && new_size != 0){
		if (new_size <= (PAGE_SIZE/2)){ // block allocator
			return alloc_block_FF(new_size);
		}
		else{
			return kmalloc(new_size);
		}
	}

	//case (4) same size
	 if (virtual_address >=(void*) start_page_alloc && virtual_address < (void*)KERNEL_HEAP_MAX){
	for (uint32 x = 0 ; x<8000 ; x++ ){
		if (prog[x].start == virtual_address && prog[x].size == num_of_pages_new ){
			return virtual_address;
			break;
		}
	 }
	}

	//case(5) size s8ir allocate in block
	 if (virtual_address >=(void*) start_page_alloc && virtual_address < (void*)KERNEL_HEAP_MAX){
		if (new_size <= (PAGE_SIZE/2)){
			kfree(virtual_address);
			return alloc_block_FF(new_size);
		}
		//case (6) still in page allocator
		else if (new_size > (PAGE_SIZE/2)){
			num_of_pages_old = 0 ;
			for (uint32 x = 0 ; x<8000 ; x++ ){
					if (prog[x].start == virtual_address){
						num_of_pages_old = prog[x].size;
						index = x;
						break;
					}
				}
			//case (6.1) size s8ir
			if (num_of_pages_old > num_of_pages_new){
				uint32 va = (uint32)virtual_address + new_size;
				uint32 diff = num_of_pages_old - num_of_pages_new;
				while (diff > 0){
							unmap_frame(ptr_page_directory, va);
							va+=PAGE_SIZE;
							diff--;

						}
				prog[index].size = num_of_pages_new;
				return virtual_address;
			}
			//case(6.2) size akbr
		    else if (num_of_pages_old < num_of_pages_new){
			     uint32 diff = num_of_pages_new - num_of_pages_old;
			     uint32 va = (uint32)virtual_address + (num_of_pages_old*PAGE_SIZE);
			     bool mark = 1;
			        while(diff > 0){ // check if enough memory ba3do
			        		for(int x = 0 ; x<8000 ; x++){
			        		     if (prog[x].start == (void*)va){
			        		    	 mark = 0;
			        		    	 break;
			        		     }
			        		}
			        		diff--;
			        		va+=PAGE_SIZE;
			        }
			        if (mark == 0){ // no enough free frame next
			        	kfree(virtual_address);
			        	return kmalloc(new_size);
			        }
			        else{ // can map
			        	diff = num_of_pages_new - num_of_pages_old;
			            va = (uint32)virtual_address + (num_of_pages_old*PAGE_SIZE);
			        	while(diff > 0){
			        		uint32 *ptr_page_table = NULL;
			        		struct FrameInfo *ptr_frame_info = get_frame_info(ptr_page_directory ,va , &ptr_page_table);
			        		int alloc = allocate_frame(&ptr_frame_info);
			        		if (alloc == E_NO_MEM){
			        		    return NULL ;
			        		 }
			        		int r = map_frame(ptr_page_directory,ptr_frame_info,va, PERM_WRITEABLE|PERM_PRESENT);
			        		diff--;
			                va+=PAGE_SIZE;
			        	}
			        	for(int x = 0 ; x<8000 ; x++){
			        	    if (prog[x].start == virtual_address){
			        	    	prog[x].size = num_of_pages_new;
			        	    	break;
			        	    }
			        	}
			        	return virtual_address;
			        }
			}
		}
	}
	//size kber page allocator
	else if (virtual_address <(void*) start_page_alloc && virtual_address > (void*)KernHeapStart){
		if (new_size > (PAGE_SIZE/2)){
			free_block(virtual_address);
			return kmalloc(new_size);
		}
		else{
			return realloc_block_FF(virtual_address , new_size);
		}
	}


		return NULL;

}
