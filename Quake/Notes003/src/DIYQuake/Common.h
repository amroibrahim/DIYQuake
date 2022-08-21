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
   Common();
   ~Common();

   void SetSubSystems(MemoryManager* pMemorymanager, System* pSystem);
   void InitGameFiles(Parameters* pParameters);
   int FindFile(char* szFileName, int* pHandleIndex);

   void TEMP_PrintListAllFiles();

protected:
   void AddGameDirectory(std::string sPAKDirectory);

   Pack* LoadPackFile(std::string& sPackFileName);

   MemoryManager* m_pMemorymanager;
   SearchPath* m_pHeadSearchList;
   System* m_pSystem;
};
