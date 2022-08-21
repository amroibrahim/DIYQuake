#include "Screen.h"

#include "Video.h"
#include "View.h"

Screen::Screen() : m_pVideo(nullptr), m_pView(nullptr)
{
}

void Screen::Init(Video* pVideo, View* pView)
{
   m_pVideo = pVideo;
   m_pView = pView;
}

void Screen::UpdateScreen()
{
   //TEMP_DrawPalette();
   //TEMP_DrawPalette2();

   m_pView->RenderView();
   UpdateRect rect{ 0, 0, m_pVideo->GetWidth(), m_pVideo->GetHeight() };
   m_pVideo->Update(&rect);
}

void Screen::TEMP_DrawPalette()
{
   byte_t* pScreenBuffer = m_pVideo->GetFrameBuffer();
   for (int i = 0; i < 256; ++i)
   {
      for (int j = 0; j < m_pVideo->GetHeight(); ++j)
      {
         pScreenBuffer[j * m_pVideo->GetWidth() + i * 2] = i;
         pScreenBuffer[j * m_pVideo->GetWidth() + i * 2 + 1] = i;
      }
   }
}

void Screen::TEMP_DrawPalette2()
{
   int PaletteWidth = m_pVideo->GetWidth() / 16;
   int PaletteHight = m_pVideo->GetHeight() / 16;

   byte_t* pScreenBuffer = m_pVideo->GetFrameBuffer();

   for (int X = 0; X < m_pVideo->GetWidth(); ++X)
   {
      for (int Y = 0; Y < m_pVideo->GetHeight(); ++Y)
      {
         pScreenBuffer[Y * m_pVideo->GetWidth() + X] = (Y / PaletteHight) * 16 + (X / PaletteWidth);
      }
   }
}
