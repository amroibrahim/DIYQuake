#include "System.h"

#include <iostream>
#include <SDL.h>

#define READ_BINARY "rb"

System::System()
{
   for (int i = 0; i < MAX_FILE_HANDLES; ++i)
   {
      m_FileHandleList[i] = nullptr;
   }
}

System::~System()
{
   SDL_Quit();
}

void System::Init()
{
   SDLInit();
}

void System::SDLInit()
{
   //Initialize SDL
   if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
   {
      std::cout << "ERROR: SDL failed to initialize! SDL_Error: " << SDL_GetError() << std::endl;
   }
}

int System::FindEmptyHandle()
{
   for (int i = 0; i < MAX_FILE_HANDLES; ++i)
   {
      if (!m_FileHandleList[i])
         return i;
   }

   return -1;
}

void System::FileRead(int32_t iHandleIndex, void* pDistnation, size_t iSize, size_t iObjectsCount)
{
   SDL_RWops* hFile = m_FileHandleList[iHandleIndex];
   SDL_RWread(hFile, pDistnation, iSize, iObjectsCount);
}

void System::FileClose(int32_t iHandleIndex)
{
   SDL_RWops* hFile = m_FileHandleList[iHandleIndex];
   SDL_RWclose(hFile);
   m_FileHandleList[iHandleIndex] = nullptr;
}

void System::Message(const std::string& sInfoMsg)
{
   std::cout << sInfoMsg << std::endl;
}

void System::InfoMessage(const std::string& sInfoMsg)
{
   std::cout << "Info :" << sInfoMsg << std::endl;
}

void System::ErrorMessage(const std::string& sError)
{
   std::cout << "Error :" << sError << std::endl;
}

int32_t System::OpenFile(std::string& sFileName)
{
   SDL_RWops* hFile = SDL_RWFromFile(sFileName.c_str(), READ_BINARY);
   if (!hFile)
   {
      return -1;
   }

   int32_t iHandleIndex = FindEmptyHandle();
   m_FileHandleList[iHandleIndex] = hFile;
   return iHandleIndex;
}

int32_t System::FileSeek(int32_t iHandleIndex, int32_t iOffset)
{
   return (int)SDL_RWseek(m_FileHandleList[iHandleIndex], iOffset, RW_SEEK_SET);
}

bool System::OpenFileEx(std::string& sFileName)
{
   m_hFileHandleEx = SDL_RWFromFile(sFileName.c_str(), READ_BINARY);
   if (!m_hFileHandleEx)
   {
      m_hFileHandleEx = nullptr;
      return false;
   }

   return true;
}

bool System::FileSeekEx(int32_t iOffset)
{
   SDL_RWseek(m_hFileHandleEx, iOffset, RW_SEEK_SET);
   return true;
}
