#pragma once

#include "Render.h"

class View
{
public:
   void Init(Render* pRender);
   void RenderView(void);

private:
   Render* m_pRender;
};

