#include "View.h"

void View::Init(Render* pRender)
{
   m_pRender = pRender;
}

void View::RenderView(void)
{
   m_pRender->RenderView();
}
