#pragma once
#include <memory>
#include <string>
#include <cstdint>

#include "Zone.h"
#include "Cache.h"

typedef std::uint8_t byte_t;

// Main hunk functionality for allocation and de-allocation
class MemoryManager
{
public:
   MemoryManager();
   ~MemoryManager();

   struct HunkHeader
   {
      int32_t iSentinel;
      int32_t iSize;
      char	szName[8];
   };

   void Init(int32_t iMemorySize); // Initialize 

   byte_t* GetStartAddress(); // Get the start address

   void* NewLowEnd(int32_t iSize); // Allocate with no name
   void* NewLowEndNamed(int32_t iSize, std::string& sName); // Allocate low end, with name
   void* NewHighEndNamed(int32_t iSize, std::string& sName); // Allocate high end, with name
   void* NewTemp(int32_t iSize); // Create a temp hunk
   void* Check(Cache::CacheData* pCacheData);

   void DeleteTemp(); // Delete the temp hunk
   void DeleteToLowMark(int32_t LowMark); // Delete low end to the given low mark
   void DeleteToHighMark(int32_t HightMark); // Delete high end to the given low mark
   void HunkInit(HunkHeader* pHeader, const int32_t& iSize, std::string& sName); // Initialize a new hunk
   void CacheEvict(Cache::CacheData* pCacheData);

   int32_t Aligne(int32_t iSize, int32_t iAlignmentValue); // Align memory to a given number
   int32_t GetMemorySize(void); // Return memory size (16MB)
   int32_t GetLowUsed(void); // Get used from low size
   int32_t GetHighUsed(void); // Get used from high size

   Zone m_Zone;
   Cache m_Cache;

protected:
   byte_t* m_pMemoryStartAddress; // Start address
   int32_t m_iMemorySize; // Hunk Memory Size (16MB)
   int32_t m_iLowUsedMark; // Low address mark 
   int32_t m_iHighUsedMark; // High address mark 
   int32_t m_iTempStartMark; // Temp start address mark
   bool m_bTempActive; // Temp active flag
};
