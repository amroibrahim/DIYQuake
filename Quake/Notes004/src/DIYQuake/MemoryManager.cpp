#include "MemoryManager.h"
#include "Cache.h"
#include "Zone.h"

#include <cstring>

constexpr uint32_t ZONE_SIZE = 48 * 1024;
constexpr uint32_t HUNK_SENTINAL = 0x1DF001ED;

std::string sHunkZoneName = "zone";
std::string sHunkUnknownName = "unknown";
std::string sHunkTempName = "temp";

MemoryManager::MemoryManager() : m_pMemoryStartAddress(nullptr), m_iMemorySize(0), m_iLowUsedMark(0), m_iHighUsedMark(0), m_iTempStartMark(0), m_bTempActive(false)
{
}

MemoryManager::~MemoryManager()
{
   delete[] m_pMemoryStartAddress;
}

void MemoryManager::Init(int32_t iMemorySize)
{
   m_pMemoryStartAddress = new byte_t[iMemorySize];
   m_iMemorySize = iMemorySize;

   // Now request Cache and Zone to attach
   m_Cache.Attach(this);
   m_Zone.Attach(this);

   m_Cache.Init();
   // Zone size 48k
   m_Zone.Init(NewLowEndNamed(ZONE_SIZE, sHunkZoneName), ZONE_SIZE);
}

byte_t* MemoryManager::GetStartAddress()
{
   return m_pMemoryStartAddress;
}

void* MemoryManager::NewLowEnd(int32_t iSize)
{
   return NewLowEndNamed(iSize, sHunkUnknownName);
}

void* MemoryManager::NewLowEndNamed(int32_t iSize, std::string& sName)
{
   HunkHeader* pHeader;

   // Make sure size is 16 byte aligned 
   iSize = sizeof(HunkHeader) + Aligne(iSize, 16);

   pHeader = (HunkHeader*)(m_pMemoryStartAddress + m_iLowUsedMark);
   m_iLowUsedMark += iSize;

   // Make sure that the Cache is out of the way
   m_Cache.FreeLow(m_iLowUsedMark);

   HunkInit(pHeader, iSize, sName);

   return (void*)(pHeader + 1);
}

void* MemoryManager::NewHighEndNamed(int32_t iSize, std::string& sName)
{
   HunkHeader* pHeader;

   // If temp already used free it first
   DeleteTemp();

   // Make sure size is 16 byte aligned 
   iSize = sizeof(HunkHeader) + Aligne(iSize, 16);

   m_iHighUsedMark += iSize;

   m_Cache.FreeHigh(m_iHighUsedMark);

   pHeader = (HunkHeader*)(m_pMemoryStartAddress + m_iMemorySize - m_iHighUsedMark);

   HunkInit(pHeader, iSize, sName);

   return (void*)(pHeader + 1);
}

void* MemoryManager::NewTemp(int32_t iSize)
{
   void* pTempMemory;

   iSize = Aligne(iSize, 16);

   // If temp already used free it first
   DeleteTemp();

   // Get temp start address
   m_iTempStartMark = GetHighUsed();

   // Create a new temp area
   pTempMemory = NewHighEndNamed(iSize, sHunkTempName);

   // Set Temp is active
   m_bTempActive = true;

   return pTempMemory;
}

void* MemoryManager::Check(Cache::CacheData* pCacheData)
{
   return m_Cache.Check(pCacheData);
}

void MemoryManager::DeleteTemp()
{
   if (m_bTempActive)
   {
      m_bTempActive = false;
      DeleteToHighMark(m_iTempStartMark);
   }
}

void MemoryManager::DeleteToLowMark(int32_t iLowMark)
{
   // Just set memory to zero and update marker 
   memset(m_pMemoryStartAddress + iLowMark, 0, m_iLowUsedMark - iLowMark);
   m_iLowUsedMark = iLowMark;
}

void MemoryManager::DeleteToHighMark(int32_t iHighMark)
{
   // Check if temp is active?
   if (m_bTempActive)
   {
      m_bTempActive = false;
      DeleteToHighMark(m_iTempStartMark);
   }

   memset(m_pMemoryStartAddress + m_iMemorySize - m_iHighUsedMark, 0, m_iHighUsedMark - iHighMark);
   m_iHighUsedMark = iHighMark;
}

void MemoryManager::HunkInit(HunkHeader* pHeader, const int32_t& iSize, std::string& sName)
{
   // set hunk memory with zeros
   std::memset(pHeader, 0, iSize);

   // update the hunk header with correct values
   pHeader->iSize = iSize;
   pHeader->iSentinel = HUNK_SENTINAL;
   strncpy(pHeader->szName, sName.c_str(), 8);
}

void MemoryManager::CacheEvict(Cache::CacheData* pCacheData)
{
   m_Cache.Evict(pCacheData);
}

int32_t MemoryManager::Aligne(int32_t iSize, int32_t iAlignmentValue)
{
   --iAlignmentValue;
   return ((iSize + iAlignmentValue) & ~iAlignmentValue);
}

int32_t MemoryManager::GetMemorySize(void)
{
   return m_iMemorySize;
}

int32_t MemoryManager::GetLowUsed(void)
{
   return m_iLowUsedMark;
}

int32_t MemoryManager::GetHighUsed(void)
{
   DeleteTemp();
   return m_iHighUsedMark;
}
