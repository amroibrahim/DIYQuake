#include "Cache.h"
#include "MemoryManager.h"

Cache::Cache() : m_pMemoryManager(nullptr), m_CacheHead()
{
}

Cache::~Cache()
{
}

void Cache::Attach(MemoryManager* pMemoryManager)
{
   m_pMemoryManager = pMemoryManager;
}

void* Cache::NewNamed(CacheData* pCacheData, int32_t iSize, std::string& sName)
{
   CacheNode* pCacheNode;

   // Make space of user passed cached indirect pointer
   iSize += sizeof(CacheNode);
   iSize = m_pMemoryManager->Aligne(iSize, 16);

   // Search for a spot for the new cache node
   while (true)
   {
      pCacheNode = TryNew(iSize, false);
      if (pCacheNode)
      {
         strncpy(pCacheNode->szName, sName.c_str(), sizeof(pCacheNode->szName) - 1);

         // The data region is just after the cache node 
         pCacheData->pData = (void*)(pCacheNode + 1);
         pCacheNode->pCacheData = pCacheData;
         break;
      }

      // No enough memory try to evict some data
      Evict(m_CacheHead.pLRUPrevious->pCacheData);
   }

   return Check(pCacheData);
}

void* Cache::Check(CacheData* pCacheData)
{
   if (!pCacheData->pData)
      return nullptr;

   // You need to backup to get access to the cache node
   CacheNode* pCacheNode = ((CacheNode*)pCacheData->pData) - 1;

   RemoveFromLRU(pCacheNode);
   MakeMRU(pCacheNode);

   return pCacheData->pData;
}

void Cache::Init(void)
{
   // On initialization just make cache head point to it self
   m_CacheHead.pNext = m_CacheHead.pPrevious = &m_CacheHead;
   m_CacheHead.pLRUNext = m_CacheHead.pLRUPrevious = &m_CacheHead;

   // TODO: AngryCPPCoder: Add flush command
}

void Cache::FreeLow(int32_t m_LowAddressUsed)
{
   CacheNode* pCacheNode;

   while (1)
   {
      pCacheNode = m_CacheHead.pNext;

      //Is cache empty?
      if (pCacheNode == &m_CacheHead)
         return;

      //Is the space free to grow? (checking if cache node is blow the low used)
      if ((byte_t*)pCacheNode >= m_pMemoryManager->GetStartAddress() + m_pMemoryManager->GetLowUsed())
         return;

      // Try to move the cache node, or evict it
      Move(pCacheNode);
   }
}

void Cache::FreeHigh(int32_t m_HighAddressUsed)
{
   CacheNode* pCurrentNode = NULL;
   CacheNode* pPreviousNode = NULL;

   while (1)
   {
      pCurrentNode = m_CacheHead.pPrevious;

      // Is cache empty
      if (pCurrentNode == &m_CacheHead)
         return;

      // is there space to grow?
      if ((byte_t*)pCurrentNode + pCurrentNode->iSize <= m_pMemoryManager->GetStartAddress() + m_pMemoryManager->GetMemorySize() - m_pMemoryManager->GetHighUsed())
         return;

      // Couldn't move, we need to evict it
      if (pCurrentNode && pCurrentNode == pPreviousNode)
      {
         Evict(pCurrentNode->pCacheData);
      }
      else
      {
         // Can we try to move it?
         Move(pCurrentNode);
         pPreviousNode = pCurrentNode;
      }
   }
}

void Cache::Evict(CacheData* pCacheData)
{
   // You need to backup to get access to the cache node
   CacheNode* pCacheNode = ((CacheNode*)pCacheData->pData) - 1;;

   pCacheNode->pPrevious->pNext = pCacheNode->pNext;
   pCacheNode->pNext->pPrevious = pCacheNode->pPrevious;
   pCacheNode->pNext = pCacheNode->pPrevious = NULL;

   pCacheData->pData = NULL;
   RemoveFromLRU(pCacheNode);
}

void Cache::EvictAll(void)
{
   // Is cache empty?
   while (m_CacheHead.pNext != &m_CacheHead)
   {
      // Evict data
      Evict(m_CacheHead.pNext->pCacheData);
   }
}

Cache::CacheNode* Cache::TryNew(int32_t iSize, bool UseHighAddress)
{
   // Try to get the first possible spot (used or not used we don't know yet)
   CacheNode* pNewCacheNode = (CacheNode*)(m_pMemoryManager->GetStartAddress() + m_pMemoryManager->GetLowUsed());

   // Is cache is empty
   if (!UseHighAddress && m_CacheHead.pLRUPrevious == &m_CacheHead)
   {
      memset(pNewCacheNode, 0, sizeof(*pNewCacheNode));
      pNewCacheNode->iSize = iSize;

      m_CacheHead.pNext = pNewCacheNode;
      m_CacheHead.pPrevious = pNewCacheNode;

      pNewCacheNode->pNext = &m_CacheHead;
      pNewCacheNode->pPrevious = &m_CacheHead;

      MakeMRU(pNewCacheNode);
      return pNewCacheNode;
   }

   // search from the bottom up for space
   CacheNode* pCacheNode = m_CacheHead.pNext;

   // Cache not empty try and look for a free space between 
   // cache nodes maybe one has been freed previously,
   // this would avoid fragmentation. 
   do
   {
      if (!UseHighAddress || pCacheNode != m_CacheHead.pNext)
      {
         if ((byte_t*)pCacheNode - (byte_t*)pNewCacheNode >= iSize)
         {
            // found space
            memset(pNewCacheNode, 0, sizeof(*pNewCacheNode));
            pNewCacheNode->iSize = iSize;

            pNewCacheNode->pNext = pCacheNode;
            pNewCacheNode->pPrevious = pCacheNode->pPrevious;
            pCacheNode->pPrevious->pNext = pNewCacheNode;
            pCacheNode->pPrevious = pNewCacheNode;

            MakeMRU(pNewCacheNode);

            return pNewCacheNode;
         }
      }

      // continue looking
      pNewCacheNode = (CacheNode*)((byte_t*)pCacheNode + pCacheNode->iSize);
      pCacheNode = pCacheNode->pNext;

   } while (pCacheNode != &m_CacheHead);

   // try to allocate one at the very end
   if (m_pMemoryManager->GetStartAddress() + m_pMemoryManager->GetMemorySize() - m_pMemoryManager->GetHighUsed() - (byte_t*)pNewCacheNode >= iSize)
   {
      memset(pNewCacheNode, 0, sizeof(*pNewCacheNode));
      pNewCacheNode->iSize = iSize;

      pNewCacheNode->pNext = &m_CacheHead;
      pNewCacheNode->pPrevious = m_CacheHead.pPrevious;
      m_CacheHead.pPrevious->pNext = pNewCacheNode;
      m_CacheHead.pPrevious = pNewCacheNode;

      MakeMRU(pNewCacheNode);

      return pNewCacheNode;
   }

   return NULL; // couldn't allocate
}

void Cache::RemoveFromLRU(CacheNode* pCacheNode)
{
   pCacheNode->pLRUNext->pLRUPrevious = pCacheNode->pLRUPrevious;
   pCacheNode->pLRUPrevious->pLRUNext = pCacheNode->pLRUNext;

   pCacheNode->pLRUNext = NULL;
   pCacheNode->pLRUPrevious = NULL;
}

void Cache::MakeMRU(CacheNode* pCacheNode)
{
   // Just move it after head 
   m_CacheHead.pLRUNext->pLRUPrevious = pCacheNode;
   pCacheNode->pLRUNext = m_CacheHead.pLRUNext;
   pCacheNode->pLRUPrevious = &m_CacheHead;
   m_CacheHead.pLRUNext = pCacheNode;
}

void Cache::Move(CacheNode* pCacheNode)
{
   CacheNode* pNewNode;

   // Try to allocate space on the higher end of memory
   pNewNode = TryNew(pCacheNode->iSize, true);
   if (pNewNode)
   {
      // Copy the cache node
      memcpy(pNewNode + 1, pCacheNode + 1, pCacheNode->iSize - sizeof(CacheNode));

      // Update caller code cache pointer
      pNewNode->pCacheData = pCacheNode->pCacheData;

      memcpy(pNewNode->szName, pCacheNode->szName, sizeof(pNewNode->szName));

      // Now evict old node
      Evict(pCacheNode->pCacheData);

      pNewNode->pCacheData->pData = (void*)(pNewNode + 1);
   }
   else
   {
      Evict(pCacheNode->pCacheData);
   }
}
