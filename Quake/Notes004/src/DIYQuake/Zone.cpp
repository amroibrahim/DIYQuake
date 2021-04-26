#include "Zone.h"

#include "MemoryManager.h"

constexpr uint32_t ZONE_SENTINAL = 0x1D4A11;
constexpr uint32_t MEMORY_MIN_FRAGMENT = 64;

Zone::Zone() : m_pZoneRoot(nullptr), m_pMemoryManager(nullptr), m_iSize(0)
{
}

Zone::~Zone()
{
}

void* Zone::New(int32_t iSize)
{
   // Adding block header size
   iSize += sizeof(ZoneMemoryBlock);
   // 4-bytes separator used in validation 
   iSize += 4;
   // Align to 8 bytes
   iSize = m_pMemoryManager->Aligne(iSize, 8);

   ZoneMemoryBlock* pRover = m_pZoneRoot->pRover;
   ZoneMemoryBlock* pFreeNode = m_pZoneRoot->pRover;
   ZoneMemoryBlock* pStart = pFreeNode->pPrev;

   // Check if the free block is actually free
   // and check if the free block size is enough
   do
   {
      if (pRover == pStart)
      {
         // Went though full list and didn't 
         // find a big enough free node
         return NULL;
      }

      if (pRover->IsUsed)
      {
         pRover = pRover->pNext;
         pFreeNode = pRover;
      }
      else
      {
         pRover = pRover->pNext;
      }

   } while (pFreeNode->IsUsed || pFreeNode->iSize < iSize);


   // Create a new free block out of left space
   int iLeftoverSpace = pFreeNode->iSize - iSize;
   if (iLeftoverSpace > MEMORY_MIN_FRAGMENT)
   {
      // there will be a free fragment after the allocated block
      ZoneMemoryBlock* pNewFreeNode = (ZoneMemoryBlock*)((byte_t*)pFreeNode + iSize);
      pNewFreeNode->iSize = iLeftoverSpace;
      pNewFreeNode->IsUsed = 0;
      pNewFreeNode->pPrev = pFreeNode;
      pNewFreeNode->iSentinel = ZONE_SENTINAL;
      pNewFreeNode->pNext = pFreeNode->pNext;
      pNewFreeNode->pNext->pPrev = pNewFreeNode;

      // Connect the new free node
      pFreeNode->pNext = pNewFreeNode;
      pFreeNode->iSize = iSize;
   }

   // Now Update the free node we found to be used
   pFreeNode->IsUsed = 1;
   m_pZoneRoot->pRover = pFreeNode->pNext;
   pFreeNode->iSentinel = ZONE_SENTINAL;

   // Update the 4 trash bytes
   *(int32_t*)((byte_t*)pFreeNode + pFreeNode->iSize - 4) = ZONE_SENTINAL;

   void* pNewBlock = (void*)((byte_t*)pFreeNode + sizeof(ZoneMemoryBlock));

   memset(pNewBlock, 0, iSize);

   return pNewBlock;
}

void Zone::Attach(MemoryManager* pMemoryManager)
{
   m_pMemoryManager = pMemoryManager;
}

void Zone::Init(void* pZoneRoot, int32_t iZoneSize)
{
   m_iSize = iZoneSize;
   m_pZoneRoot = (ZoneMemoryHeader*)pZoneRoot;

   ZoneMemoryBlock* pMemoryBlock;

   pMemoryBlock = (ZoneMemoryBlock*)((byte_t*)m_pZoneRoot + sizeof(ZoneMemoryHeader));

   m_pZoneRoot->BlockList.pNext = pMemoryBlock;
   m_pZoneRoot->BlockList.pPrev = pMemoryBlock;
   m_pZoneRoot->BlockList.IsUsed = 1;
   m_pZoneRoot->BlockList.iSentinel = 0;
   m_pZoneRoot->BlockList.iSize = 0;
   m_pZoneRoot->pRover = pMemoryBlock;

   pMemoryBlock->pNext = &m_pZoneRoot->BlockList;
   pMemoryBlock->pPrev = &m_pZoneRoot->BlockList;
   pMemoryBlock->IsUsed = 0;
   pMemoryBlock->iSentinel = ZONE_SENTINAL;

   // The header is eating up space, subtract it.
   pMemoryBlock->iSize = iZoneSize - sizeof(ZoneMemoryHeader);
}

void Zone::Delete(void* pZoneData)
{
   // From data pointer we need to backup the size of ZonememoryBlock to get 
   // its content
   ZoneMemoryBlock* pBlockToDelete = (ZoneMemoryBlock*)((byte_t*)pZoneData - sizeof(ZoneMemoryBlock));
   pBlockToDelete->IsUsed = 0;

   ZoneMemoryBlock* pPrevNode = pBlockToDelete->pPrev;

   // Merge previous node if free
   if (!pPrevNode->IsUsed)
   {
      pPrevNode->iSize += pBlockToDelete->iSize;
      pPrevNode->pNext = pBlockToDelete->pNext;
      pPrevNode->pNext->pPrev = pPrevNode;


      if (pBlockToDelete == m_pZoneRoot->pRover)
      {
         m_pZoneRoot->pRover = pPrevNode;
      }

      pBlockToDelete = pPrevNode;
   }

   // Merge next node if free
   ZoneMemoryBlock* pNextNode = pBlockToDelete->pNext;

   if (!pNextNode->IsUsed)
   {
      // merge the next free block onto the end
      pBlockToDelete->iSize += pNextNode->iSize;
      pBlockToDelete->pNext = pNextNode->pNext;
      pBlockToDelete->pNext->pPrev = pBlockToDelete;

      if (pNextNode == m_pZoneRoot->pRover)
      {
         m_pZoneRoot->pRover = pBlockToDelete;
      }
   }
}
