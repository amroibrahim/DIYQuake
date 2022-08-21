#include "Host.h"

#include <SDL.h>
#include <string>
#include <windows.h>

constexpr uint32_t MEMORY_16MB = 16 * 1024 * 1024;
char szPaletteFileName[] = "gfx/palette.lmp";

Host::Host()
{
}

Host::~Host()
{
   m_Video.Shutdown();
}

void Host::Init(Parameters* pParameters, System* pSystem)
{
   m_pParameters = pParameters;
   m_pSystem = pSystem;

   GetWorkingDirectory();
   m_Memorymanager.Init(MEMORY_16MB);
   m_View.Init(&m_Render);
   m_Common.Init(&m_Memorymanager, pSystem, m_pParameters);

   m_ModelManager.Init(&m_Memorymanager, &m_Common);

   byte_t* pPalette = m_Common.LoadFile(szPaletteFileName);

   m_Video.Init(pPalette);
   m_Render.Init(&m_Video, &m_Client, &m_Memorymanager);
   m_Screen.Init(&m_Video, &m_View);


   m_Client.Init(&m_ModelManager);
   m_Client.TEMP_LoadPlayerModel();
   //m_Common.TEMP_PrintListAllFiles();
}

void Host::GetWorkingDirectory()
{
   // We have to free the char* to avoid memory leak
   char* szDirecotry = SDL_GetBasePath();
   m_pParameters->sCurrentWorkingDirectory = szDirecotry;
   SDL_free(szDirecotry);
}

void Host::Frame(float fTime)
{
   if (!FilterTime(fTime))
      return;

   m_pSystem->GetKeyEvents();

   m_Screen.UpdateScreen();
}

bool Host::FilterTime(float fTime)
{
   dRealTime += fTime; // Keep track for the time

   // TODO: this needed an update when implementing demo play
   if (dRealTime - dOldrealTime < 1.0 / 72.0) // 72 framerate per second
      return false;

   dFrametime = dRealTime - dOldrealTime;
   dOldrealTime = dRealTime;

   // Clamp value between 0.1 and 0.001
   if (dFrametime > 0.1)
      dFrametime = 0.1;
   if (dFrametime < 0.001)
      dFrametime = 0.001;
   return true;
}
