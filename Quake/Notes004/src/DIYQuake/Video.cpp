#include "Video.h"

#include <vector>

constexpr uint32_t PALETTE_SIZE = 256;
constexpr uint32_t SCREEN_WIDTH = 640;
constexpr uint32_t SCREEN_HEIGHT = 480;

Video::Video() : m_pWindow(nullptr), m_pPalette(nullptr), m_pWindowSurface(nullptr), m_pScreenBuffer(nullptr),
m_pFrameBuffer(nullptr), m_pFrameBufferDirect(nullptr), m_pFrameBufferConsole(nullptr),
m_iHeight(0), m_iWidth(0), m_fAspectRatio(0)
{
}

void Video::Init(byte_t* pPalette)
{
   m_iWidth = SCREEN_WIDTH;
   m_iHeight = SCREEN_HEIGHT;

   m_pWindow = SDL_CreateWindow("DIYQuake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_iWidth, m_iHeight, SDL_WINDOW_SHOWN);
   if (m_pWindow == nullptr)
   {
      return;
   }

   m_pWindowSurface = SDL_GetWindowSurface(m_pWindow);
   if (m_pWindowSurface == nullptr)
   {
      return;
   }

   // Create 8-bit screen buffer (that use palette)
   m_pScreenBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, m_iWidth, m_iHeight, 8, 0, 0, 0, 0);
   if (m_pScreenBuffer == nullptr)
   {
      return;
   }

   SDL_FillRect(m_pScreenBuffer, NULL, 0);

   m_pFrameBuffer = (byte_t*)m_pScreenBuffer->pixels;
   m_pFrameBufferDirect = (byte_t*)m_pScreenBuffer->pixels;
   m_pFrameBufferConsole = (byte_t*)m_pScreenBuffer->pixels;

   SetPalette(pPalette);
}

void Video::SetPalette(byte_t* pPalette)
{
   if (m_pPalette)
   {
      delete[] m_pPalette;
   }

   m_pPalette = new SDL_Color[PALETTE_SIZE];

   for (int i = 0; i < PALETTE_SIZE; ++i)
   {
      m_pPalette[i].r = *pPalette++;
      m_pPalette[i].g = *pPalette++;
      m_pPalette[i].b = *pPalette++;
   }

   SDL_SetPaletteColors(m_pScreenBuffer->format->palette, m_pPalette, 0, PALETTE_SIZE);
}

void Video::Update(UpdateRect* pRectsList)
{

   std::vector<SDL_Rect> vRects;
   UpdateRect* pCurrentRect = pRectsList;
   SDL_Rect vConvertRect;
   while (pRectsList)
   {
      vConvertRect.x = pRectsList->iX;
      vConvertRect.y = pRectsList->iY;
      vConvertRect.w = pRectsList->iWidth;
      vConvertRect.h = pRectsList->iHeight;

      vRects.push_back(vConvertRect);
      pRectsList = pRectsList->pNext;
   }

   if (m_pScreenBuffer == nullptr || m_pWindowSurface == nullptr)
   {
      return;
      }

   for (SDL_Rect& vRect : vRects)
   {
      SDL_BlitSurface(m_pScreenBuffer, &vRect, m_pWindowSurface, &vRect);
   }

   SDL_UpdateWindowSurface(m_pWindow);
}

void Video::BeginDirectUpdate(UpdateRect vUpdateRect, byte_t* pBitMapData)
{
   if (m_pFrameBufferDirect == nullptr)
   {
      return;
   }

   // Check if iX Value is not negative
   if (vUpdateRect.iX < 0)
   {
      // Offset iX to be in range
      vUpdateRect.iX = m_pScreenBuffer->w + vUpdateRect.iX - 1;
   }

   byte_t* pOffsetLocation = m_pFrameBufferDirect + vUpdateRect.iY * m_pScreenBuffer->pitch + vUpdateRect.iX;

   for (int i = 0; i < vUpdateRect.iHeight; ++i)
   {
      memcpy(pOffsetLocation, pBitMapData, vUpdateRect.iWidth);
      pOffsetLocation += m_pScreenBuffer->pitch;
      pBitMapData += vUpdateRect.iWidth;
   }
}

void Video::EndDirectUpdate(/*UpdateRect vUpdateRect*/)
{
   if (m_pFrameBufferDirect == nullptr)
   {
      return;
   }

   SDL_UpdateWindowSurface(m_pWindow);
}

int32_t Video::GetWidth()
{
   return m_iWidth;
}

int32_t Video::GetHeight()
{
   return m_iHeight;
}

byte_t* Video::GetFrameBuffer()
{
   return m_pFrameBuffer;
}

byte_t* Video::GetFrameBufferDirect()
{
   return m_pFrameBufferDirect;
}

byte_t* Video::GetFrameBufferConsole()
{
   return m_pFrameBufferConsole;
}

void Video::Shutdown(void)
{
   SDL_FreeSurface(m_pScreenBuffer);
   m_pScreenBuffer = nullptr;

   SDL_FreeSurface(m_pWindowSurface);
   m_pWindowSurface = nullptr;

   SDL_DestroyWindow(m_pWindow);
   m_pWindow = nullptr;

   if (m_pPalette)
   {
      delete[] m_pPalette;
   }
}