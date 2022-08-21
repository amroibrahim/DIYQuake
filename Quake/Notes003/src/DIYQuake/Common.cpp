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

void Common::SetSubSystems(MemoryManager* pMemorymanager, System* pSystem)
{
   m_pMemorymanager = pMemorymanager;
   m_pSystem = pSystem;
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

int Common::FindFile(char* szFileName, int* pHandleIndex)
{
   SearchPath* pCurrent = m_pHeadSearchList;
   int iFileSize = 0;
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
                  if (m_pSystem->OpenFileEx(sPackFileName))
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
   int32_t iPAKFileHandleIndex = m_pSystem->OpenFile(sPackFileName);

   if (iPAKFileHandleIndex < 0)
   {
      return nullptr;
   }

   PackHeader header;
   // Read Pack header
   m_pSystem->FileRead(iPAKFileHandleIndex, &header, sizeof(PackHeader), 1);

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
   m_pSystem->FileRead(iPAKFileHandleIndex, pPackFileOnDisk, header.iDirectoryLength, 1);

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
