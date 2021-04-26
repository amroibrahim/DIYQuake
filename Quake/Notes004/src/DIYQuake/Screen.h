#pragma once

class Video;

class Screen
{
public:
   Screen();
   void Init(Video* pVideo);
   void UpdateScreen();

   void TEMP_DrawPalette();
   void TEMP_DrawPalette2();

protected:
   Video* m_pVideo;
};

