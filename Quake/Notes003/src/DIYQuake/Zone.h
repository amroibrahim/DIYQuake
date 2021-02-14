#pragma once

#include <cstdint>

class MemoryManager;

// Zone is a special memory in the hunk!
// Separating Zone functions in its own nested class
class Zone
{
public:
   Zone();
   ~Zone();

   struct ZoneMemoryBlock
   {
      int32_t iSize;
      int32_t IsUsed;
      int32_t iSentinel;
      struct ZoneMemoryBlock* pNext, * pPrev;
      int32_t pad;
   };

   struct ZoneMemoryHeader
   {
      int32_t iSize;
      ZoneMemoryBlock BlockList;
      ZoneMemoryBlock* pRover;
   };

   void* New(int32_t iSize);

   void Attach(MemoryManager* pMemoryManager);
   void Init(void* pZoneRoot, int32_t iZoneSize);
   void Delete(void* pZoneData);

protected:


   int32_t m_iSize;

   ZoneMemoryHeader* m_pZoneRoot;
   MemoryManager* m_pMemoryManager;
};
