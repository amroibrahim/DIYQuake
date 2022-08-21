/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// Z_zone.pCurrent

#include "quakedef.h"

#define DYNAMIC_SIZE 0xc000

#define ZONEID 0x1d4a11
#define MINFRAGMENT 64

// AngryCPPCoder: I was thinking why would IsUsed be an int (32 bit)? All was needed was just
// a single bit, for used or not, now looking at it, most likely the decision to make it 32 bit
// was to help with memory alignment to be a multiple of 64.
typedef struct ZoneMemoryBlock_S
{
    int size; // including the header and possibly tiny fragments
    int IsUsed; // a IsUsed of 0 is a free block
    int id; // should be ZONEID
    struct ZoneMemoryBlock_S* next, * prev;
    int pad; // pad to 64 bit boundary
}
ZoneMemoryBlock;

typedef struct
{
    int size; // total bytes malloced, including header
    ZoneMemoryBlock BlockList; // pStart / end cap for linked list
    ZoneMemoryBlock* pRover;
}
ZoneMemoryHeader;

void Cache_FreeLow(int new_low_hunk);
void Cache_FreeHigh(int new_high_hunk);

/*
==============================================================================

                  ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

The zone calls are pretty much only used for small strings and structures,
all big things are allocated on the hunk.
==============================================================================
*/

ZoneMemoryHeader* pZoneRoot;

/*
========================
Z_ClearZone
========================
*/
void Z_ClearZone(ZoneMemoryHeader* pZoneRoot, int size)
{
    ZoneMemoryBlock* block;

    // set the entire zone to one free block
    block = (ZoneMemoryBlock*)((byte*)pZoneRoot + sizeof(ZoneMemoryHeader));
    //pZoneRoot->size is never set or used
    pZoneRoot->BlockList.IsUsed = 1; // in use block
    pZoneRoot->BlockList.id = 0;
    pZoneRoot->BlockList.size = 0;
    pZoneRoot->pRover = block;

    block->size = size - sizeof(ZoneMemoryHeader);
    block->IsUsed = 0; // free block
    block->id = ZONEID;
    block->next = &pZoneRoot->BlockList;
    block->prev = &pZoneRoot->BlockList;
}

/*
========================
Z_Free
========================
*/
void Z_Free(void* ptr)
{
    ZoneMemoryBlock* pMemoryBlock, * other;

    if (!ptr)
        Sys_Error("Z_Free: NULL pointer");

    // AngryCPPCoder: we are pointing at data, backup to
    // the memblock section to access the struct
    pMemoryBlock = (ZoneMemoryBlock*)((byte*)ptr - sizeof(ZoneMemoryBlock));

    if (pMemoryBlock->id != ZONEID)
        Sys_Error("Z_Free: freed a pointer without ZONEID");
    if (pMemoryBlock->IsUsed == 0)
        Sys_Error("Z_Free: freed a freed pointer");

    pMemoryBlock->IsUsed = 0; // mark as free

    other = pMemoryBlock->prev;
    if (!other->IsUsed)
    {
        // merge with previous free block
        other->size += pMemoryBlock->size;
        other->next = pMemoryBlock->next;
        other->next->prev = other;
        if (pMemoryBlock == pZoneRoot->pRover)
            pZoneRoot->pRover = other;
        pMemoryBlock = other;
    }

    other = pMemoryBlock->next;
    if (!other->IsUsed)
    {
        // merge the next free block onto the end
        pMemoryBlock->size += other->size;
        pMemoryBlock->next = other->next;
        pMemoryBlock->next->prev = pMemoryBlock;
        if (other == pZoneRoot->pRover)
            pZoneRoot->pRover = pMemoryBlock;
    }
}

/*
========================
Z_Malloc
========================
*/
void* Z_Malloc(int size)
{
    void* buf;

    // Z_CheckHeap();	// DEBUG
    buf = Z_TagMalloc(size, 1);
    if (!buf)
        Sys_Error("Z_Malloc: failed on allocation of %i bytes", size);
    Q_memset(buf, 0, size);

    return buf;
}

void* Z_TagMalloc(int size, int IsUsed)
{
    int iLeftoverSpace;
    ZoneMemoryBlock* pStart, * pRover, * pNewNode, * pFreeBase;

    if (!IsUsed)
        Sys_Error("Z_TagMalloc: tried to use a 0 IsUsed");

    //
    // scan through the block list looking for the first free block
    // of sufficient size
    //

    size += sizeof(ZoneMemoryBlock); // account for size of block header
    size += 4; // space for memory trash tester
    size = (size + 7) & ~7; // align to 8-byte boundary

    pFreeBase = pRover = pZoneRoot->pRover;
    pStart = pFreeBase->prev;

    do
    {
        if (pRover == pStart) // scanned all the way around the list
            return NULL;
        if (pRover->IsUsed)
            pFreeBase = pRover = pRover->next;
        else
            pRover = pRover->next;
    }
    while (pFreeBase->IsUsed || pFreeBase->size < size);

    //
    // found a block big enough
    //
    iLeftoverSpace = pFreeBase->size - size;
    if (iLeftoverSpace > MINFRAGMENT)
    {
        // there will be a free fragment after the allocated block
        pNewNode = (ZoneMemoryBlock*)((byte*)pFreeBase + size);
        pNewNode->size = iLeftoverSpace;
        pNewNode->IsUsed = 0; // free block
        pNewNode->prev = pFreeBase;
        pNewNode->id = ZONEID;
        pNewNode->next = pFreeBase->next;
        pNewNode->next->prev = pNewNode;
        pFreeBase->next = pNewNode;
        pFreeBase->size = size;
    }

    pFreeBase->IsUsed = IsUsed; // no longer a free block

    pZoneRoot->pRover = pFreeBase->next; // next allocation will pStart looking here

    pFreeBase->id = ZONEID;

    // marker for memory trash testing
    *(int*)((byte*)pFreeBase + pFreeBase->size - 4) = ZONEID;

    return (void*)((byte*)pFreeBase + sizeof(ZoneMemoryBlock));
}

/*
========================
Z_Print
========================
*/
void Z_Print(ZoneMemoryHeader* zone)
{
    ZoneMemoryBlock* block;

    Con_Printf("zone size: %i  location: %p\n", pZoneRoot->size, pZoneRoot);

    for (block = zone->BlockList.next;; block = block->next)
    {
        Con_Printf("block:%p    size:%7i    IsUsed:%3i\n",
                   block, block->size, block->IsUsed);

        if (block->next == &zone->BlockList)
            break; // all blocks have been hit
        if ((byte*)block + block->size != (byte*)block->next)
            Con_Printf("ERROR: block size does not touch the next block\n");
        if (block->next->prev != block)
            Con_Printf("ERROR: next block doesn't have proper back link\n");
        if (!block->IsUsed && !block->next->IsUsed)
            Con_Printf("ERROR: two consecutive free blocks\n");
    }
}

/*
========================
Z_CheckHeap
========================
*/
void Z_CheckHeap(void)
{
    ZoneMemoryBlock* pMemoryBlock;

    for (pMemoryBlock = pZoneRoot->BlockList.next;; pMemoryBlock = pMemoryBlock->next)
    {
        if (pMemoryBlock->next == &pZoneRoot->BlockList)
            break; // all blocks have been hit
        if ((byte*)pMemoryBlock + pMemoryBlock->size != (byte*)pMemoryBlock->next)
            Sys_Error("Z_CheckHeap: block size does not touch the next block\n");
        if (pMemoryBlock->next->prev != pMemoryBlock)
            Sys_Error("Z_CheckHeap: next block doesn't have proper back link\n");
        if (!pMemoryBlock->IsUsed && !pMemoryBlock->next->IsUsed)
            Sys_Error("Z_CheckHeap: two consecutive free blocks\n");
    }
}

//============================================================================

#define HUNK_SENTINAL 0x1df001ed

typedef struct
{
    int sentinal;
    int size; // including sizeof(HunkHeader), -1 = not allocated
    char name[8];
}
HunkHeader; // AngryCPPCoder: hunk_t renamed to HunkHeader

// AngryCPPCoder: rename hunk_base to pHunkBase;
// AngryCPPCoder: rename hunk_size to iHunkSize;
byte* pHunkBase;
int iHunkSize;

int hunk_low_used;
int hunk_high_used;

qboolean hunk_tempactive;
int hunk_tempmark;

//void R_FreeTextures(void);

/*
==============
Hunk_Check

Run consistancy and sentinal trahing checks
==============
*/
void Hunk_Check(void)
{
    HunkHeader* header; // AngryCPPCoder: h renamed to header

    for (header = (HunkHeader*)pHunkBase;
            (byte*)header != pHunkBase + hunk_low_used;)
    {
        if (header->sentinal != HUNK_SENTINAL)
            Sys_Error("Hunk_Check: trahsed sentinal");
        if (header->size < 16 || header->size + (byte*)header - pHunkBase > iHunkSize)
            Sys_Error("Hunk_Check: bad size");
        header = (HunkHeader*)((byte*)header + header->size);
    }
}

/*
==============
Hunk_Print

If "all" is specified, every single allocation is printed.
Otherwise, allocations with the same name will be totaled up before printing.
==============
*/
void Hunk_Print(qboolean all)
{
    // AngryCPPCoder: 	hunk_t*           h, * next,   * endlow, * starthigh, * endhigh;
    //                  HunkHeader* pHeader, * pNext, * pEndLow, * pStartHigh, * pEndHigh;

    HunkHeader* pHeader, * pNext, * pEndLow, * pStartHigh, * pEndHigh;
    int count, sum;
    int totalblocks;
    char name[9];

    name[8] = 0;
    count = 0;
    sum = 0;
    totalblocks = 0;

    pHeader = (HunkHeader*)pHunkBase;
    pEndLow = (HunkHeader*)(pHunkBase + hunk_low_used);
    pStartHigh = (HunkHeader*)(pHunkBase + iHunkSize - hunk_high_used);
    pEndHigh = (HunkHeader*)(pHunkBase + iHunkSize);

    Con_Printf("          :%8i total hunk size\n", iHunkSize);
    Con_Printf("-------------------------\n");

    while (1)
    {
        //
        // skip to the high hunk if done with low hunk
        //
        if (pHeader == pEndLow)
        {
            Con_Printf("-------------------------\n");
            Con_Printf("          :%8i REMAINING\n", iHunkSize - hunk_low_used - hunk_high_used);
            Con_Printf("-------------------------\n");
            pHeader = pStartHigh;
        }

        //
        // if totally done, break
        //
        if (pHeader == pEndHigh)
            break;

        //
        // run consistancy checks
        //
        if (pHeader->sentinal != HUNK_SENTINAL)
            Sys_Error("Hunk_Check: trahsed sentinal");
        if (pHeader->size < 16 || pHeader->size + (byte*)pHeader - pHunkBase > iHunkSize)
            Sys_Error("Hunk_Check: bad size");

        pNext = (HunkHeader*)((byte*)pHeader + pHeader->size);
        count++;
        totalblocks++;
        sum += pHeader->size;

        //
        // print the single block
        //
        memcpy(name, pHeader->name, 8);
        if (all)
            Con_Printf("%8p :%8i %8s\n", pHeader, pHeader->size, name);

        //
        // print the total
        //
        if (pNext == pEndLow || pNext == pEndHigh ||
                strncmp(pHeader->name, pNext->name, 8))
        {
            if (!all)
                Con_Printf("          :%8i %8s (TOTAL)\n", sum, name);
            count = 0;
            sum = 0;
        }

        pHeader = pNext;
    }

    Con_Printf("-------------------------\n");
    Con_Printf("%8i total blocks\n", totalblocks);

}

/*
===================
Hunk_AllocName
===================
*/
void* Hunk_AllocName(int size, char* name)
{
    HunkHeader* pHeader;

#ifdef PARANOID
    Hunk_Check();
#endif

    if (size < 0)
        Sys_Error("Hunk_Alloc: bad size: %i", size);

    // AngryCPPCoder: Calculate block size (Header + Size required then rounding to 16 (16 bytes alignment))
    size = sizeof(HunkHeader) + ((size + 15) & ~15);

    if (iHunkSize - hunk_low_used - hunk_high_used < size)
        Sys_Error("Hunk_Alloc: failed on %i bytes", size);

    pHeader = (HunkHeader*)(pHunkBase + hunk_low_used);
    hunk_low_used += size;

    Cache_FreeLow(hunk_low_used);

    memset(pHeader, 0, size);

    pHeader->size = size;
    pHeader->sentinal = HUNK_SENTINAL;
    Q_strncpy(pHeader->name, name, 8);

    return (void*)(pHeader + 1);
}

/*
===================
Hunk_Alloc
===================
*/
void* Hunk_Alloc(int size)
{
    return Hunk_AllocName(size, "unknown");
}

int Hunk_LowMark(void)
{
    return hunk_low_used;
}

void Hunk_FreeToLowMark(int mark)
{
    if (mark < 0 || mark > hunk_low_used)
        Sys_Error("Hunk_FreeToLowMark: bad mark %i", mark);
    memset(pHunkBase + mark, 0, hunk_low_used - mark);
    hunk_low_used = mark;
}

int Hunk_HighMark(void)
{
    if (hunk_tempactive)
    {
        hunk_tempactive = false;
        Hunk_FreeToHighMark(hunk_tempmark);
    }

    return hunk_high_used;
}

void Hunk_FreeToHighMark(int mark)
{
    if (hunk_tempactive)
    {
        hunk_tempactive = false;
        Hunk_FreeToHighMark(hunk_tempmark);
    }
    if (mark < 0 || mark > hunk_high_used)
        Sys_Error("Hunk_FreeToHighMark: bad mark %i", mark);
    memset(pHunkBase + iHunkSize - hunk_high_used, 0, hunk_high_used - mark);
    hunk_high_used = mark;
}

/*
===================
Hunk_HighAllocName
===================
*/
void* Hunk_HighAllocName(int size, char* name)
{
    HunkHeader* pHeader;

    if (size < 0)
        Sys_Error("Hunk_HighAllocName: bad size: %i", size);

    if (hunk_tempactive)
    {
        Hunk_FreeToHighMark(hunk_tempmark);
        hunk_tempactive = false;
    }

#ifdef PARANOID
    Hunk_Check();
#endif

    size = sizeof(HunkHeader) + ((size + 15) & ~15);

    if (iHunkSize - hunk_low_used - hunk_high_used < size)
    {
        Con_Printf("Hunk_HighAlloc: failed on %i bytes\n", size);
        return NULL;
    }

    hunk_high_used += size;
    Cache_FreeHigh(hunk_high_used);

    pHeader = (HunkHeader*)(pHunkBase + iHunkSize - hunk_high_used);

    memset(pHeader, 0, size);
    pHeader->size = size;
    pHeader->sentinal = HUNK_SENTINAL;
    Q_strncpy(pHeader->name, name, 8);

    return (void*)(pHeader + 1);
}

/*
=================
Hunk_TempAlloc

Return space from the top of the hunk
=================
*/
void* Hunk_TempAlloc(int size)
{
    void* buf;

    size = (size + 15) & ~15;

    // AngryCPPCoder: No need for validating hunk_tempactive
    // Hunk_HighMark() function call with do that for us
    //----------------------------
    if (hunk_tempactive)
    {
        Hunk_FreeToHighMark(hunk_tempmark);
        hunk_tempactive = false;
    }
    //----------------------------

    hunk_tempmark = Hunk_HighMark();

    buf = Hunk_HighAllocName(size, "temp");

    hunk_tempactive = true;

    return buf;
}

/*
===============================================================================

CACHE MEMORY

===============================================================================
*/

typedef struct cache_system_s
{
    int size; // including this header
    cache_data_t* user;
    char name[16];
    struct cache_system_s* prev, * next;
    struct cache_system_s* lru_prev, * lru_next; // for LRU flushing
}
cache_system_t;

cache_system_t* Cache_TryAlloc(int size, qboolean nobottom);

cache_system_t CacheHead;

/*
===========
Cache_Move
===========
*/
void Cache_Move(cache_system_t* pCurrent)
{
    cache_system_t* pNewNode;

    // we are clearing up space at the bottom, so only allocate it late
    pNewNode = Cache_TryAlloc(pCurrent->size, true);
    if (pNewNode)
    {
        //		Con_Printf ("cache_move ok\n");

       // AngryCPPCoder Copy the Cache data (user data) to the new location
        Q_memcpy(pNewNode + 1, pCurrent + 1, pCurrent->size - sizeof(cache_system_t));
        pNewNode->user = pCurrent->user; // update the pointer
        Q_memcpy(pNewNode->name, pCurrent->name, sizeof(pNewNode->name));
        Cache_Free(pCurrent->user);
        pNewNode->user->data = (void*)(pNewNode + 1);
    }
    else
    {
        //		Con_Printf ("cache_move failed\n");

        Cache_Free(pCurrent->user); // tough luck...
    }
}

/*
============
Cache_FreeLow

Throw things out until the hunk can be expanded to the given point
============
*/
void Cache_FreeLow(int new_low_hunk)
{
    cache_system_t* pCurrent;

    while (1)
    {
        pCurrent = CacheHead.next;
        if (pCurrent == &CacheHead)
            return; // nothing in cache at all
        if ((byte*)pCurrent >= pHunkBase + new_low_hunk)
            return; // there is space to grow the hunk
        Cache_Move(pCurrent); // reclaim the space
    }
}

/*
============
Cache_FreeHigh

Throw things out until the hunk can be expanded to the given point
============
*/
void Cache_FreeHigh(int new_high_hunk)
{
    cache_system_t* pCurrent, * pPrevious;

    pPrevious = NULL;
    while (1)
    {
        pCurrent = CacheHead.prev;
        if (pCurrent == &CacheHead)
            return; // nothing in cache at all
        if ((byte*)pCurrent + pCurrent->size <= pHunkBase + iHunkSize - new_high_hunk)
            return; // there is space to grow the hunk
        if (pCurrent == pPrevious)
            Cache_Free(pCurrent->user); // didn't move out of the way
        else
        {
            Cache_Move(pCurrent); // try to move it
            pPrevious = pCurrent;
        }
    }
}

void Cache_UnlinkLRU(cache_system_t* pCacheNode)
{
    if (!pCacheNode->lru_next || !pCacheNode->lru_prev)
        Sys_Error("Cache_UnlinkLRU: NULL link");

    pCacheNode->lru_next->lru_prev = pCacheNode->lru_prev;
    pCacheNode->lru_prev->lru_next = pCacheNode->lru_next;

    pCacheNode->lru_prev = pCacheNode->lru_next = NULL;
}

void Cache_MakeLRU(cache_system_t* pCacheNode)
{
    if (pCacheNode->lru_next || pCacheNode->lru_prev)
        Sys_Error("Cache_MakeLRU: active link");

    CacheHead.lru_next->lru_prev = pCacheNode;
    pCacheNode->lru_next = CacheHead.lru_next;
    pCacheNode->lru_prev = &CacheHead;
    CacheHead.lru_next = pCacheNode;
}

/*
============
Cache_TryAlloc

Looks for a free block of memory between the high and low hunk marks
Size should already include the header and padding
============
*/
cache_system_t* Cache_TryAlloc(int size, qboolean nobottom)
{
    cache_system_t *pCacheNode, *pNewCacheNode;

    // is the cache completely empty?
    if (!nobottom && CacheHead.prev == &CacheHead)
    {
        if (iHunkSize - hunk_high_used - hunk_low_used < size)
            Sys_Error("Cache_TryAlloc: %i is greater then free hunk", size);

        pNewCacheNode = (cache_system_t*)(pHunkBase + hunk_low_used);
        memset(pNewCacheNode, 0, sizeof(*pNewCacheNode));
        pNewCacheNode->size = size;

        CacheHead.prev = CacheHead.next = pNewCacheNode;
        pNewCacheNode->prev = pNewCacheNode->next = &CacheHead;

        Cache_MakeLRU(pNewCacheNode);
        return pNewCacheNode;
    }

    // search from the bottom up for space

    pNewCacheNode = (cache_system_t*)(pHunkBase + hunk_low_used);
    pCacheNode = CacheHead.next;

    // AngryCPPCoder: Next block tries to look for a free space between 
    // cache nodes maybe one has been freed previously,
    // this would avoid fragmentation. 
    do
    {
        if (!nobottom || pCacheNode != CacheHead.next)
        {
            if ((byte*)pCacheNode - (byte*)pNewCacheNode >= size)
            {
                // found space
                memset(pNewCacheNode, 0, sizeof(*pNewCacheNode));
                pNewCacheNode->size = size;

                pNewCacheNode->next = pCacheNode;
                pNewCacheNode->prev = pCacheNode->prev;
                pCacheNode->prev->next = pNewCacheNode;
                pCacheNode->prev = pNewCacheNode;

                Cache_MakeLRU(pNewCacheNode);

                return pNewCacheNode;
            }
        }

        // continue looking
        pNewCacheNode = (cache_system_t*)((byte*)pCacheNode + pCacheNode->size);
        pCacheNode = pCacheNode->next;

    }
    while (pCacheNode != &CacheHead);

    // try to allocate one at the very end
    if (pHunkBase + iHunkSize - hunk_high_used - (byte*)pNewCacheNode >= size)
    {
        memset(pNewCacheNode, 0, sizeof(*pNewCacheNode));
        pNewCacheNode->size = size;

        pNewCacheNode->next = &CacheHead;
        pNewCacheNode->prev = CacheHead.prev;
        CacheHead.prev->next = pNewCacheNode;
        CacheHead.prev = pNewCacheNode;

        Cache_MakeLRU(pNewCacheNode);

        return pNewCacheNode;
    }

    return NULL; // couldn't allocate
}

/*
============
Cache_Flush

Throw everything out, so new data will be demand cached
============
*/
void Cache_Flush(void)
{
    while (CacheHead.next != &CacheHead)
        Cache_Free(CacheHead.next->user); // reclaim the space
}

/*
============
Cache_Print

============
*/
void Cache_Print(void)
{
    cache_system_t* pCacheNode;

    for (pCacheNode = CacheHead.next; pCacheNode != &CacheHead; pCacheNode = pCacheNode->next)
    {
        Con_Printf("%8i : %s\n", pCacheNode->size, pCacheNode->name);
    }
}

/*
============
Cache_Report

============
*/
void Cache_Report(void)
{
    Con_DPrintf("%4.1f megabyte data cache\n", (iHunkSize - hunk_high_used - hunk_low_used) / (float)(1024 * 1024));
}

/*
============
Cache_Compact

============
*/
void Cache_Compact(void) {}

/*
============
Cache_Init

============
*/
void Cache_Init(void)
{
    CacheHead.next = CacheHead.prev = &CacheHead;
    CacheHead.lru_next = CacheHead.lru_prev = &CacheHead;

    Cmd_AddCommand("flush", Cache_Flush);
}

/*
==============
Cache_Free

Frees the memory and removes it from the LRU list
==============
*/
void Cache_Free(cache_data_t* pData)
{
    cache_system_t* pCacheNode;

    if (!pData->data)
        Sys_Error("Cache_Free: not allocated");

    pCacheNode = ((cache_system_t*)pData->data) - 1;

    pCacheNode->prev->next = pCacheNode->next;
    pCacheNode->next->prev = pCacheNode->prev;
    pCacheNode->next = pCacheNode->prev = NULL;

    pData->data = NULL;

    Cache_UnlinkLRU(pCacheNode);
}

/*
==============

Cache_Check
==============
*/
// AngryCPPCoder: Check if data is in memory, if so return it and update LRU
void* Cache_Check(cache_data_t* pData)
{
    cache_system_t* pCacheNode;

    if (!pData->data)
        return NULL;

    // AngryCPPCoder: Backup to the Cache struct region 
    pCacheNode = ((cache_system_t*)pData->data) - 1;

    // move to head of LRU
    Cache_UnlinkLRU(pCacheNode);
    Cache_MakeLRU(pCacheNode);

    return pData->data;
}

/*
==============
Cache_Alloc
==============
*/
void* Cache_Alloc(cache_data_t* pData, int size, char* name)
{
    cache_system_t* pCacheNode;

    if (pData->data)
        Sys_Error("Cache_Alloc: already allocated");

    if (size <= 0)
        Sys_Error("Cache_Alloc: size %i", size);

    size = (size + sizeof(cache_system_t) + 15) & ~15;

    // find memory for it
    while (1)
    {
        pCacheNode = Cache_TryAlloc(size, false);
        if (pCacheNode)
        {
            strncpy(pCacheNode->name, name, sizeof(pCacheNode->name) - 1);
            pData->data = (void*)(pCacheNode + 1);// AngryCPPCoder: The data region is just after the cache node 
            pCacheNode->user = pData;
            break;
        }

        // free the least recently used cahedat
        if (CacheHead.lru_prev == &CacheHead)
            Sys_Error("Cache_Alloc: out of memory");
        // not enough memory at all
        Cache_Free(CacheHead.lru_prev->user);
    }

    return Cache_Check(pData);
}

//============================================================================

/*
========================
Memory_Init
========================
*/
void Memory_Init(void* buf, int size)
{
    int iParamIndex;
    int iZoneSize = DYNAMIC_SIZE;

    pHunkBase = buf;
    iHunkSize = size;
    hunk_low_used = 0;
    hunk_high_used = 0;

    Cache_Init();

    // AngryCPPCoder: Zone Init
    iParamIndex = COM_CheckParm("-zone");
    if (iParamIndex)
    {
        if (iParamIndex < com_argc - 1)
            iZoneSize = Q_atoi(com_argv[iParamIndex + 1]) * 1024;
        else
            Sys_Error("Memory_Init: you must specify a size in KB after -zone");
    }

    pZoneRoot = Hunk_AllocName(iZoneSize, "zone");
    Z_ClearZone(pZoneRoot, iZoneSize);
}