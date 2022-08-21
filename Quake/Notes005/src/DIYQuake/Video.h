#pragma once

#include <SDL.h>

#include "MemoryManager.h"
#include <memory>

struct UpdateRect
{
   int32_t iX;
   int32_t iY;
   int32_t iWidth;
   int32_t iHeight;

   UpdateRect* pNext;
};


class Video
{
public:
   Video();
   void Init(byte_t* pPalette);
   void SetPalette(byte_t* pPalette);
   void Update(UpdateRect* pRectsList);
   void BeginDirectUpdate(UpdateRect vUpdateRect, byte_t* pBitMapData);
   void EndDirectUpdate(/*UpdateRect vUpdateRect*/);
   void Shutdown(void);

   int32_t GetWidth();
   int32_t GetHeight();

   byte_t* GetFrameBuffer();
   byte_t* GetFrameBufferDirect();
   byte_t* GetFrameBufferConsole();


protected:
   SDL_Color* m_pPalette;

   SDL_Window* m_pWindow;

   SDL_Surface* m_pWindowSurface;
   SDL_Surface* m_pScreenBuffer;

   // struct VideoDefinition
   // {
   int32_t m_iWidth;
   int32_t m_iHeight;

   byte_t* m_pFrameBuffer;
   byte_t* m_pFrameBufferDirect;
   byte_t* m_pFrameBufferConsole;

   float m_fAspectRatio;
   // };

   // VideoDefinition m_VideoDefinition;

};

