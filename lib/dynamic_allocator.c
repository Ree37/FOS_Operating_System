/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
__inline__ uint32 get_block_size(void* va) {
	uint32 *curBlkMetaData = ((uint32 *) va - 1);
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va) {
	uint32 *curBlkMetaData = ((uint32 *) va - 1);
	return (~(*curBlkMetaData) & 0x1);
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY) {
	void *va = NULL;
	switch (ALLOC_STRATEGY) {
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list) {
	cprintf("=========================================\n");
	struct BlockElement* blk;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", get_block_size(blk),
				is_free_block(blk));
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart,
		uint32 initSizeOfAllocatedSpace) {
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0)
			initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...
	LIST_INIT(&freeBlocksList);
	uint32 *BNG = (uint32 *) daStart;
	*BNG = 1;
	uint32 *END = (uint32 *) (daStart + initSizeOfAllocatedSpace - sizeof(int));
	*END = 1;
	struct BlockElement *x = (struct BlockElement*) (BNG + 2);

	uint32 initSize = initSizeOfAllocatedSpace - 2 * sizeof(uint32);
	set_block_data(x, initSize, 0);
	LIST_INSERT_HEAD(&freeBlocksList, x);

}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated) {
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("set_block_data is not implemented yet");
	//Your Code is Here...

	uint32 *header = (uint32*) ((uint8*) va - 4);
	uint32 *footer = (uint32*) ((uint8*) va + totalSize - 2 * sizeof(uint32));
	*header = totalSize | isAllocated;
	*footer = totalSize | isAllocated;
}

//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size) {
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (size % 2 != 0)
			size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE;
		if (!is_initialized) {
			uint32 required_size = size + 2 * sizeof(int) /*header & footer*/
					+ 2 * sizeof(int) /*da begin & end*/;
			uint32 da_start = (uint32) sbrk(
					ROUNDUP(required_size, PAGE_SIZE) / PAGE_SIZE);
			uint32 da_break = (uint32) sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_FF is not implemented yet");
	//Your Code is Here...

	if (size == 0) {
		return NULL;
	}

	//h7tag a7sb al size 3la b3do
	uint32 totalSize = size + 8;
	//hlf 3al list
	struct BlockElement *element = NULL;
	LIST_FOREACH(element, &freeBlocksList)
	{
		if (totalSize <= get_block_size(element)) {
			//1.hn7sb al difference
			int32 diff = get_block_size(element) - totalSize;

			//2.lw al difference as8r mn 16
			if (diff < 16) {
				//1.hset al element block
				set_block_data(element, totalSize, 1);
				//h4ilha mn al list
				LIST_REMOVE(&freeBlocksList, element);
				return element;
			}
			//2.lw al difference akbr aw tsawi mn 16
			else if (diff >= 16) {
				//1.hnminus al difference mn al block al kbira
				uint32 freeBlockSize = get_block_size(element) - totalSize;
				//2.hnset 3la asas al total size
				set_block_data(element, totalSize, 1);
				//3.hn7sb al address bta3 al element al gdid
				void *freeBlockAddr = (uint8*)(element) + get_block_size(element);
				//4.hncreate new element bsize al difference
				set_block_data(freeBlockAddr, freeBlockSize, 0);
				//5.hninsert fy al list b3d al element
				LIST_INSERT_AFTER(&freeBlocksList, element, (struct BlockElement* )freeBlockAddr);
				//h4il al element ali etmlt mn al free list
				LIST_REMOVE(&freeBlocksList, element);
				return element;
			}
		}
	}

	// call sbrk
	return NULL;
}
//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size) {
	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("alloc_block_BF is not implemented yet");
	//Your Code is Here...

}

//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va) {
	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_block is not implemented yet");
	//Your Code is Here...

	if (va == NULL) {
		return;
	}

	//change the flag to 0
	uint32 vaSize = get_block_size(va);
	set_block_data(va, vaSize, 0);

	//get prv va
	uint32 footerPrv = *((uint32 *) va - 2);
	uint32 sizePrv = footerPrv & (~0x1);
	void *prv = (uint8 *) va - (sizePrv - 8) - 8;

	//get nxt va
	void * nxt = (uint8 *) va + get_block_size(va);

	bool merged = 0;

	// check if the prv is free
	if (is_free_block(prv)) {
		// add the two blocks
		uint32 mergedBlock = get_block_size(prv) + get_block_size(va);
		// merge (set the new block) with flag 0
		set_block_data(prv, mergedBlock, 0);
		// set element to prv
//		va = prv;
		merged = 1;
	}

	// check if the nxt is free
	if (is_free_block(nxt)) {
		// add the two blocks
		uint32 mergedBlock = get_block_size(va) + get_block_size(nxt);
		// merge (set the new block) with flag 0
		set_block_data(va, mergedBlock, 0);

		if (merged) {
			LIST_REMOVE(&freeBlocksList, (struct BlockElement*)nxt);
		}
	}

	// if should insert in free list
	int listSize = LIST_SIZE(&freeBlocksList);
	if (!merged) {
		//Case 1 list if empty
		if (listSize == 0) {
			LIST_INSERT_HEAD(&freeBlocksList, (struct BlockElement * ) va);
		}
		//Case 2 the freed block is the first block in the list (head)
		else if ((struct BlockElement *) va < LIST_FIRST(&freeBlocksList)) {
			LIST_INSERT_HEAD(&freeBlocksList, (struct BlockElement * ) va);
		}
		//Case 3 the freed block is the last block in the list (tail)
		else if ((struct BlockElement *) va > LIST_LAST(&freeBlocksList)) {
			LIST_INSERT_TAIL(&freeBlocksList, (struct BlockElement * ) va);
		}
		//Case 4 insert in sorted position
		else {
			struct BlockElement *element = NULL;
			LIST_FOREACH(element, &freeBlocksList)
			{
				if (element > (struct BlockElement *) va) {
					LIST_INSERT_BEFORE(&freeBlocksList, element, (struct BlockElement* ) va);
					break;
				}
			}
		}
	}
}

//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size) {
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("realloc_block_FF is not implemented yet");
	//Your Code is Here...
}

/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size) {
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size) {
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}
