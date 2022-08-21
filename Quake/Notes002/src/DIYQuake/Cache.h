#pragma once

#include <string>
#include <cstdint>
class MemoryManager;

// Cache functionality, it is a special area in hunk (between low and high used memory)
// Separating Zone functions in its own nested class
class Cache
{
public:
   Cache();
   ~Cache();

   struct CacheData
   {
      void* pData;
   };

   struct CacheNode
   {
      int32_t iSize;
      CacheData* pCacheData;
      char szName[16];
      CacheNode* pPrevious, * pNext;
      CacheNode* pLRUPrevious, * pLRUNext;
   };

   void Attach(MemoryManager* pMemoryManager); // Attach to memory manager
   void* NewNamed(CacheData* pCacheData, int32_t iSize, std::string& sName); // Allocate new with name
   void* Check(CacheData* pCacheData); // Get data if cached or else return null 

   void Init(void); // Initialize
   void FreeLow(int32_t m_LowAddressUsed); // Free space from low side
   void FreeHigh(int32_t m_HighAddressUsed); // Free space from hight side
   void Evict(CacheData* pData); // remove from cache
   void EvictAll(void); // Delete everything in cache

   CacheNode* TryNew(int32_t iSize, bool UseHighAddress); // Try to allocate, if can't return null

protected:
   void RemoveFromLRU(CacheNode* pCacheNode); // remove the node from the LRU list
   void MakeMRU(CacheNode* pCacheNode); // Make Most recently used (move to top of list)
   void Move(CacheNode* pCacheNode); // Mode data to a new free space

   CacheNode m_CacheHead;
   MemoryManager* m_pMemoryManager;
};
