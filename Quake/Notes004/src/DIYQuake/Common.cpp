#include "Common.h"

#include <iostream>
#include <string>

using namespace std;

#define	QUAKE_ASSET_FOLDER "id1"

std::string sHunkPackFiles = "packfile";

Common::Common() : m_pMemorymanager(nullptr), m_pHeadSearchList(nullptr), m_pSystem(nullptr)
{
}

Common::~Common()
{
}

void Common::Init(Parameters* pParameters)
{
   InitGameFiles(pParameters);
}

void Common::SetSubSystems(MemoryManager* pMemorymanager, System* pSystem)
{
   m_pMemorymanager = pMemorymanager;
   m_pSystem = pSystem;
}

void Common::CloseFile(int32_t iHandleIndex)
{
   SearchPath* pCurrent = m_pHeadSearchList;

   // Check if the handle is for a pack file, if so don't close it (we will need it to load other files)
   while (pCurrent)
   {
      if (pCurrent->pPack && pCurrent->pPack->iFileHandleIndex == iHandleIndex)
      {
         return;
      }

      pCurrent = pCurrent->pNext;
   }

   // Handle if for file not in a pack, close the file to free the handle
   m_pSystem->FileClose(iHandleIndex);
}

void Common::InitGameFiles(Parameters* pParameters)
{
   string sPAKDirectory = pParameters->sCurrentWorkingDirectory;

   // Did the EXE get a specific Pack folder to look into?
   int iArgumentIndex = pParameters->FindArgument(PARAM_BASE_DIR);
   if (iArgumentIndex > 0)
   {
      sPAKDirectory = pParameters->ArgumentsList[iArgumentIndex + 1];
   }

   AddGameDirectory(sPAKDirectory + "/" + QUAKE_ASSET_FOLDER);
}

int32_t Common::FindFile(char* szFileName, int32_t* pHandleIndex)
{
   SearchPath* pCurrent = m_pHeadSearchList;
   int32_t iFileSize = 0;
   while (pCurrent)
   {
      // Is it a valid pack file?
      if (pCurrent->pPack)
      {
         Pack* pPack = pCurrent->pPack;
         for (int iFileIndex = 0; iFileIndex < pPack->iNumberOfFiles; ++iFileIndex)
         {
            if (!strcmp(pPack->pFiles[iFileIndex].szName, szFileName))
            {

               if (pHandleIndex)
               {
                  *pHandleIndex = pPack->iFileHandleIndex;
                  m_pSystem->FileSeek(pPack->iFileHandleIndex, pPack->pFiles[iFileIndex].iFileOffset);
               }
               else
               {
                  // Create a new file handler
                  string sPackFileName = pPack->szFileName;
                  if (m_pSystem->FileOpenEx(sPackFileName))
                  {
                     m_pSystem->FileSeekEx(pPack->pFiles[iFileIndex].iFileOffset);
                  }
               }

               iFileSize = pPack->pFiles[iFileIndex].iFileSize;
               return iFileSize;
            }
         }
      }

      pCurrent = pCurrent->pNext;
   }

   if (pHandleIndex)
   {
      *pHandleIndex = -1;
   }

   return -1;
}

byte_t* Common::LoadFile(char* szFileName)
{
   return LoadFileToMemory(szFileName, LOAD_MEMORY::HUNK_NAMED);
}

void Common::TEMP_PrintListAllFiles()
{
   SearchPath* pCurrent = m_pHeadSearchList;
   while (pCurrent)
   {
      if (!pCurrent->pPack)
      {
         break;
      }

      std::cout << pCurrent->pPack->szFileName << std::endl;
      for (size_t i = 0; i < pCurrent->pPack->iNumberOfFiles; i++)
      {
         std::cout << "[" << i << "] " << pCurrent->pPack->pFiles[i].szName << std::endl;
      }

      pCurrent = pCurrent->pNext;
   }
}

void Common::AddGameDirectory(std::string sPAKDirectory)
{
   SearchPath* pSearchPath = (SearchPath*)m_pMemorymanager->NewLowEnd(sizeof(SearchPath));
   strcpy(pSearchPath->szPathName, sPAKDirectory.c_str());

   pSearchPath->pNext = m_pHeadSearchList;
   m_pHeadSearchList = pSearchPath;

   // Keep searching until we don't find any more pack files
   int iFileNumber = 0;
   while (true)
   {
      string sPakFileName = sPAKDirectory + "/pak" + to_string(iFileNumber) + ".pak";
      Pack* pPack = LoadPackFile(sPakFileName);
      if (!pPack)
         break;

      pSearchPath = (SearchPath*)m_pMemorymanager->NewLowEnd(sizeof(SearchPath));
      pSearchPath->pPack = pPack;
      pSearchPath->pNext = m_pHeadSearchList;
      m_pHeadSearchList = pSearchPath;

      iFileNumber++;
   }
}

Pack* Common::LoadPackFile(string& sPackFileName)
{
   int32_t iPAKFileHandleIndex = m_pSystem->FileOpen(sPackFileName);

   if (iPAKFileHandleIndex < 0)
   {
      return nullptr;
   }

   PackHeader header;
   // Read Pack header
   m_pSystem->FileRead(iPAKFileHandleIndex, &header, sizeof(PackHeader));

   //Validate PAK file header
   if (header.ID[0] != 'P' && header.ID[1] != 'A' && header.ID[2] != 'C' && header.ID[3] != 'K')
   {
      return nullptr;
   }

   int iFilesCount = header.iDirectoryLength / sizeof(PackFileOnDisk);

   // I don't feel okay putting a 2048 or size PackFileOnDisk on stack
   PackFileOnDisk* pPackFileOnDisk = new PackFileOnDisk[2048];

   PackFile* pFiles = (PackFile*)m_pMemorymanager->NewLowEndNamed(iFilesCount * sizeof(PackFile), sHunkPackFiles);
   m_pSystem->FileSeek(iPAKFileHandleIndex, header.iDirectoryOffset);
   m_pSystem->FileRead(iPAKFileHandleIndex, pPackFileOnDisk, header.iDirectoryLength);

   for (int i = 0; i < iFilesCount; i++)
   {
      strcpy(pFiles[i].szName, pPackFileOnDisk[i].szName);
      pFiles[i].iFileOffset = pPackFileOnDisk[i].iFileOffset;
      pFiles[i].iFileSize = pPackFileOnDisk[i].iFileSize;
   }

   Pack* pPack = (Pack*)m_pMemorymanager->NewLowEnd(sizeof(Pack));
   strcpy(pPack->szFileName, sPackFileName.c_str());
   pPack->iFileHandleIndex = iPAKFileHandleIndex;
   pPack->iNumberOfFiles = iFilesCount;
   pPack->pFiles = pFiles;

   return pPack;
}

void Common::GetFileBaseName(char* szFileName, char* szBaseFileName)
{
   int iLen = strlen(szFileName);
   char* pDot = szFileName + iLen - 1;
   while (szFileName < pDot && *pDot != '.')
   {
      --pDot;
   }
   
   char* pStart = pDot;
   while (szFileName <= pStart && *pStart != '/')
   {
      --pStart;
   }

   ++pStart;

   int iCopyCount = pDot - pStart;
   strncpy(szBaseFileName, pStart, iCopyCount);

   szBaseFileName[iCopyCount] = '\0';
}

byte_t* Common::LoadFileToMemory(char* szFileName, Common::LOAD_MEMORY eLoadMemory, Cache::CacheData* pData, int iStackSize, byte_t* pStack)
{
   int32_t iHandleIndex = -1;
   int32_t iFileSize = FindFile(szFileName, &iHandleIndex);
   
   // File not found
   if(iHandleIndex == -1)
      return nullptr;

   char szBaseFileName[32];
   GetFileBaseName(szFileName, szBaseFileName);

   byte_t* pLoadBuffer = nullptr;
   
   std::string sBaseFileName(szBaseFileName);

   switch (eLoadMemory)
   {
   case Common::LOAD_MEMORY::HUNK_NAMED:
      pLoadBuffer = (byte_t*)m_pMemorymanager->NewLowEndNamed(iFileSize + 1, sBaseFileName);
      break;
   case Common::LOAD_MEMORY::HUNK_TEMP:
      pLoadBuffer = (byte_t*)m_pMemorymanager->NewTemp(iFileSize + 1);
      break;
   case Common::LOAD_MEMORY::HUNK_ZONE:
      pLoadBuffer = (byte_t*)m_pMemorymanager->m_Zone.New(iFileSize + 1);
      break;
   case Common::LOAD_MEMORY::HUNK_CACHE:
      pLoadBuffer = (byte_t*)m_pMemorymanager->m_Cache.NewNamed(pData, iFileSize + 1, sBaseFileName);
      break;
   case Common::LOAD_MEMORY::HUNK_TEMP_STACK:
      if (iFileSize + 1 > iStackSize)
         pLoadBuffer = (byte_t*)m_pMemorymanager->NewTemp(iFileSize + 1);
      else
         pLoadBuffer = pStack;
      break;
   }


   // Mark end of file? Do I really need this?
   ((byte_t*)pLoadBuffer)[iFileSize] = 0;


   // TODO: Draw Begin Disc image
   m_pSystem->FileRead(iHandleIndex, pLoadBuffer, iFileSize);
   CloseFile(iHandleIndex);
   // TODO: Draw End Disc image
 
   return pLoadBuffer;
}
