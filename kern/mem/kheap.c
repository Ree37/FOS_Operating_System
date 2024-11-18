#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

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

	    uint32 alignedStart = daStart;
	    uint32 alignedSegmentBreak = daStart + initSizeToAllocate;
	    // Align segment break to next page boundary if needed
	    if (alignedSegmentBreak % PAGE_SIZE != 0) {
	    	 alignedSegmentBreak = (alignedSegmentBreak + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	    }

	    if (alignedSegmentBreak > daLimit)
	    {
	    	panic("Aligned segment break exceeds memory limit");
	    }


	KernHeapStart = (void *)daStart;
	segment_break = (void *)(daStart+initSizeToAllocate);
	hard_limit = (void *)daLimit;


	/*if(!is_initialized){
		 panic("no enough memory");
	}*/
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
	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
		 * 				you should allocate pages and map them into the kernel virtual address space,
		 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
		 * numOfPages = 0: just return the current position of the segment break
		 *
		 * NOTES:
		 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
		 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, return -1
		 */

		//MS2: COMMENT THIS LINE BEFORE START CODING==========
		//return (void*)-1 ;
		//====================================================

		//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
		// Write your code here, remove the panic and write your code
		//panic("sbrk() is not implemented yet...!!");


    if (numOfPages == 0) {
        return segment_break;
    }

    uint32 current_break = (uint32)segment_break;
    uint32 new_brk = current_break + (numOfPages * PAGE_SIZE);


    if (new_brk >(uint32) hard_limit) {
        return (void*)-1;
    }

   	    uint32 old_brk = current_break;
   	    for (int i = 0; i < numOfPages; i++) {

   	        struct FrameInfo* frame = NULL;
   	        int ret = allocate_frame(&frame);
   	        if (ret == E_NO_MEM) {
   	            return (void*)-1;
   	        }

   	    int r = map_frame(ptr_page_directory,frame,current_break, PERM_WRITEABLE);
   	    if (r == E_NO_MEM){
   	 		free_frame(frame) ;
   	 		return (void*) -1 ;
   	 	}

   	        current_break += PAGE_SIZE;
   	    }


      //move break
    segment_break =(void*) new_brk;


    return (void*)old_brk;
}


//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
      uint32 hard = (uint32) hard_limit;
      uint32 total = hard + PAGE_SIZE;
	  uint32 *start_page_alloc =(uint32*) total ;
      uint32 MAX =(uint32) KERNEL_HEAP_MAX - (uint32)start_page_alloc;
      uint32 *start =NULL;
      uint32 count =0;

      if(size > MAX){
    	  return NULL;
      }

     if (size <= (PAGE_SIZE/2)) // block allocator
     {
       void * ret =(void*) alloc_block_FF(size);
       return ret;
     }

     uint32 num_of_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;


       for(uint32 i =(uint32)start_page_alloc  ; i < (uint32) KERNEL_HEAP_MAX ; i+=PAGE_SIZE )
       {

    	   uint32 *ptr_page_table = NULL;
    	   struct FrameInfo *ptr_frame_info = get_frame_info(ptr_page_directory , i , &ptr_page_table);
           if (ptr_frame_info != NULL){
        	   continue;
           }

    	   int ret = allocate_frame(&ptr_frame_info);
    	   if (ret == E_NO_MEM)
    	   {
    	         return NULL;
    	    }


    	    int r = map_frame(ptr_page_directory,ptr_frame_info,i, PERM_WRITEABLE|PERM_PRESENT);

    	    if(start == NULL){
    	       start = (void*)i;
    	     }

            count++;
            num_of_pages--;
            if (num_of_pages != 0 && i == (KERNEL_HEAP_MAX - PAGE_SIZE)) {

                for (uint32 j = (uint32)start; j < (uint32)(i+PAGE_SIZE); j += PAGE_SIZE) {
                    struct FrameInfo *frame = get_frame_info(ptr_page_directory, j, &ptr_page_table);
                    free_frame(frame);
                }
                return NULL;
            }

    	    if(num_of_pages == 0){
    	    	break;
    	    }


       }

     if(num_of_pages != 0){
    	 return NULL;
     }

     return start;


}


void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
	// Write your code here, remove the panic and write your code
	panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	// Write your code here, remove the panic and write your code
	panic("kheap_virtual_address() is not implemented yet...!!");

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
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
