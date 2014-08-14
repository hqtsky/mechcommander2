#pragma once
//===========================================================================//
// File:	 MemoryManager.cpp												 //
// Contents: MemoryManager subsyystem										 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

struct _MEMORYPOOL;
struct _HEAPHEADER;
struct _LARGEBLOCKHEADER;

//
// Single byte before allocations
//
// 0	 = Invalid (duplicate free detection)
// 1-252 = Heap number for a 'pool' allocated block
// 253   = Large block (not in pools)
// 254   = Block allocated before init.
// 255   = Unused
//
//
const int Magic_LargeBlock=253;					// Larger than 4k memory block (no in pools)
const int Magic_BeforeInit=254;					// Allocated before inited  (was 0xabadcafe)

//
// This is the largest 'pool' size - blocks larger than this use the system allocator
//
const int LargestMemoryBlock=1024;				// Largest memory block managed
//
// This is the total number of pools being managed
//
const int MemoryPools=18;						// Number of memory pools (defined in PoolSizes[])

//
// Magic number placed after allocations in DEBUG/ARMOR
//
const int MemoryEndMarker=0x7fb1deaf;			// Placed at the end of allocations in _ARMOR builds





#pragma pack(push,1)
//
// Information block for all blocks in pools with < 256 bytes
//
typedef struct SMALLPOOLBLOCK {
#if defined(LAB_ONLY)
	UCHAR			Size;						// Size of block (note pools are always <64K)
#endif
#ifdef _DEBUG
	DOUBLE			dTimeStamp;					// at what time was the block alloc'd
	ULONG			pContext[0];				// we'll allocate the right size based on gMemoryStackWalkLevel
#endif
} SMALLPOOLBLOCK;

//
// Information block for all blocks in pools with >= 256 bytes
//
typedef struct POOLBLOCK {
#if defined(LAB_ONLY)
	USHORT			Size;						// Size of block (note pools are always <64K)
#endif
#ifdef _DEBUG
	DOUBLE			dTimeStamp;					// at what time was the block alloc'd
	ULONG			pContext[0];				// we'll allocate the right size based on gMemoryStackWalkLevel
#endif
} POOLBLOCK;

//
// Header for large blocks or blocks before memory manager inited.
//
typedef struct _LARGEBLOCK
{
	ULONG				Size;					// Size of block (note can be any size)
	_LARGEBLOCKHEADER*	pLast;					// Pointer to previous large memory block
	_LARGEBLOCKHEADER*	pNext;					// Pointer to next large memory block
	UCHAR				Heap;
#ifdef _DEBUG
	DOUBLE				dTimeStamp;				// at what time was the block alloc'd
	ULONG				pContext[0];			// we'll allocate the right size based on gMemoryStackWalkLevel
#endif
} LARGEBLOCK;

//
// Structure placed directly BEFORE each memory block
//
typedef struct _LARGEBLOCKHEADER
{
	LARGEBLOCK*		pLargeBlockInfo;
	UCHAR			LargeMagicNumber;			// Magic_LargeBlock or Magic_BeforeInit
} LARGEBLOCKHEADER;

//
// Structure used to hold information about 'pools' of memory blocks
//
typedef struct _MEMORYPOOL
{
	ULONG				HeapTag;				// Tag so that memory is easily spotted in VM viewers
	_MEMORYPOOL*		pLast;					// Points to the last heap of the SAME block size
	_MEMORYPOOL*		pNext;					// Points to the next heap of the SAME block size
	POOLBLOCK*			pInfoBlocks;			// an array of blocks which describe particular memory blocks (may point to a SMALLPOOLBLOCK header for >256 bytes)
	PUCHAR				pMemoryPool;			// Pointer to the base of the memory blocks (pointer to header byte before allocation)
	USHORT				wBlockSize;				// what is the size of the individual blocks?
	USHORT				wTotalBlocks;			// Total blocks available
	#if defined(LAB_ONLY)
	USHORT				wUserBytes;				// the amount of memory in the pool that is actual user data
	#endif
	USHORT				AllocCount[16];			// Number of blocks allocated in each 4K page (when 0, block can be decommitted)
	USHORT				FreeBlockPtr;			// Next available block in FreeBlockStack
	USHORT				FreeBlockStack[0];		// Free block offsets (from base of pool - pointer to header byte before allocation)
} MEMORYPOOL;

//
// Used to hold GameOS level heap information
//
typedef struct gos_Heap
{
	gos_Heap*	pParent;
	gos_Heap*	pNext;
	gos_Heap*	pChild;
	ULONG		Magic;
	int			Instances;
	char		Name[128];
#if defined(LAB_ONLY)
	ULONG		MaximumSize;
	int			BytesAllocated;
	int			TotalAllocatedLastLoop;
	int			BytesAllocatedThisLoop;
	int			BytesFreedThisLoop;
	int			AllocationsThisLoop;
	int			AllocationsLastLoop;
	int			TotalAllocations;
	int			PeakSize;
	UCHAR		bExpanded;							// this is used for collapsing tree in debugger
	UCHAR		bExamined;							// this is used for collapsing tree in debugger
	int			LargeAllocations;
	int			LargeAllocated;
	int			DXAllocations;
	int			DXAllocated;
#endif
	UCHAR		HeapNumber;							// Heap number in HeapList[]  (<<24)
} gos_Heap;
#pragma pack(pop)





extern gos_Heap* AllHeaps;
extern USHORT PoolSizes[MemoryPools];
extern HGOSHEAP HeapList[256];
extern ULONG LargeMemorySize;
extern ULONG LargeMemoryAllocations;
extern ULONG gMemoryBlockOverhead;
extern HGOSHEAP Heap_Texture;
extern HGOSHEAP Heap_Texture32;
extern HGOSHEAP Heap_Texture16;
extern HGOSHEAP Heap_TextureOther;
extern HGOSHEAP Heap_Network;
extern HGOSHEAP ParentGameOSHeap;
extern HGOSHEAP DefaultHeap;
extern MEMORYPOOL* gMemoryPool[MemoryPools];


void __stdcall gos_ChangeHeapSize( HGOSHEAP Heap, int Change, UCHAR SystemAllocation=0 );
void __stdcall MM_CheckRegistered(void);
void __stdcall MM_Shutdown(void);
void __stdcall MM_UpdateStatistics( HGOSHEAP Heap );
void __stdcall MM_Startup(void);
void __stdcall SetupVirtualMemory(void);
void __stdcall AnalyzeWS( PSTR Title );


extern PSAPI_WS_WATCH_INFORMATION* pMemBlockInfo;
extern HANDLE HWSProcess;
