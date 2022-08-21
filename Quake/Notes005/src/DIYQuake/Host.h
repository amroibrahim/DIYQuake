#pragma once

#include "Common.h"
#include "MemoryManager.h"
#include "View.h"
#include "Render.h"
#include "Parameters.h"
#include "System.h"
#include "Video.h"
#include "Screen.h"
#include "ModelManager.h"
#include "Client.h"

class Host
{
public:
   Host();
   ~Host();
   void Init(Parameters* pParameters, System* pSystem);

   void Frame(float fTime);

protected:
   void GetWorkingDirectory();

   bool FilterTime(float fTime);

   Parameters* m_pParameters;
   System* m_pSystem;

   MemoryManager m_Memorymanager;
   ModelManager m_ModelManager;
   Common m_Common;
   Video m_Video;
   Screen m_Screen;
   Client m_Client;
   View m_View;
   Render m_Render;

   double dRealTime;
   double dOldrealTime;
   double dFrametime;
};
