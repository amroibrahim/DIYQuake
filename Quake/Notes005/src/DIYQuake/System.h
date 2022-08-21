#pragma once

#include <string>
#include <SDL.h>

#define MAX_FILE_HANDLES 10

class System
{
public:
   System();
   ~System();

   void Init();
   void GetKeyEvents();
   void Quit();


   void FileRead(int32_t iHandleIndex, void* pDistnation, size_t iSize, size_t iObjectsCount = 1);
   void FileClose(int32_t iHandleIndex);

   void Message(const std::string& sInfoMsg);
   void InfoMessage(const std::string& sInfoMsg);
   void ErrorMessage(const std::string& sErrorMsg);

   int32_t FileOpen(std::string& sFileName);
   int32_t FileSeek(int32_t iHandleIndex, int32_t iOffset);

   uint64_t GetTime();
   uint64_t GetPerformanceFrequency();

   bool GetIsRunning();
   bool FileOpenEx(std::string& sFileName);
   bool FileSeekEx(int32_t iOffset);

protected:
   void SDLInit();
   int FindEmptyHandle();

   bool m_bIsRunning;
   SDL_RWops* m_FileHandleList[MAX_FILE_HANDLES];
   SDL_RWops* m_hFileHandleEx;

};
