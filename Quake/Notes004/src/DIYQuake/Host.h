#pragma once

#include "Common.h"
#include "MemoryManager.h"
#include "Parameters.h"
#include "System.h"
#include "Video.h"
#include "Screen.h"

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
   Common m_Common;
   Video m_Video;
   Screen m_Screen;

   double dRealTime;
   double dOldrealTime;
   double dFrametime;
};
