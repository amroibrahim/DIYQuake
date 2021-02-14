#include "Host.h"

#include <SDL.h>
#include <string>
#include <windows.h>

constexpr uint32_t MEMORY_16MB = 16 * 1024 * 1024;

Host::Host()
{
}

Host::~Host()
{
}

void Host::Init(Parameters* pParameters, System* pSystem)
{
   m_pParameters = pParameters;

   GetWorkingDirectory();
   m_Memorymanager.Init(MEMORY_16MB);

   m_Common.SetSubSystems(&m_Memorymanager, pSystem);

   m_Common.InitGameFiles(m_pParameters);

   m_Common.TEMP_PrintListAllFiles();
}

void Host::GetWorkingDirectory()
{
   // We have to free the char* to avoid memory leak
   char* szDirecotry = SDL_GetBasePath();
   m_pParameters->sCurrentWorkingDirectory = szDirecotry;
   SDL_free(szDirecotry);
}
