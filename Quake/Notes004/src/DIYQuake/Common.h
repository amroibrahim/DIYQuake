#pragma once

#include <string>

#include "Parameters.h"
#include "MemoryManager.h"
#include "System.h"

#define MAX_PATH_LENGTH 128
#define MAX_PACK_NAME 64 
#define MAX_PACK_NAME_DISK 56

struct PackFile
{
   char szName[MAX_PACK_NAME];
   int32_t iFileOffset;
   int32_t iFileSize;
};

struct Pack
{
   char szFileName[MAX_PATH_LENGTH];
   int32_t iFileHandleIndex;
   int32_t iNumberOfFiles;
   PackFile* pFiles;
};

struct PackFileOnDisk
{
   char szName[MAX_PACK_NAME_DISK];
   int32_t iFileOffset;
   int32_t iFileSize;
};

struct PackHeader
{
   char ID[4];
   int32_t iDirectoryOffset;
   int32_t iDirectoryLength;
};

struct SearchPath
{
   char szPathName[MAX_PATH_LENGTH];
   Pack* pPack;
   SearchPath* pNext;
};

class Common
{
public:
   enum class LOAD_MEMORY 
   { 
      HUNK_ZONE,
      HUNK_NAMED,
      HUNK_TEMP,
      HUNK_CACHE,
      HUNK_TEMP_STACK
   };

   Common();
   ~Common();

   void Init(Parameters* pParameters);
   void SetSubSystems(MemoryManager* pMemorymanager, System* pSystem);
   void CloseFile(int32_t iHandleIndex);
   
   int FindFile(char* szFileName, int32_t* pHandleIndex);

   byte_t* LoadFile(char* szFileName);

   void TEMP_PrintListAllFiles();

protected:
   void InitGameFiles(Parameters* pParameters);
   void AddGameDirectory(std::string sPAKDirectory);
   void GetFileBaseName(char* szFileName, char* szBaseFileName);

   byte_t* LoadFileToMemory(char* szFileName, Common::LOAD_MEMORY eLoadMemory, Cache::CacheData* pData = nullptr, int iStackSize = 0, byte_t* pStack = nullptr);

   Pack* LoadPackFile(std::string& sPackFileName);

   MemoryManager* m_pMemorymanager;
   SearchPath* m_pHeadSearchList;
   System* m_pSystem;
   
};
