#pragma once

class Video;
class View;

class Screen
{
public:
   Screen();
   void Init(Video* pVideo, View* pView);
   void UpdateScreen();

   void TEMP_DrawPalette();
   void TEMP_DrawPalette2();

protected:
   Video* m_pVideo;
   View* m_pView;
};

