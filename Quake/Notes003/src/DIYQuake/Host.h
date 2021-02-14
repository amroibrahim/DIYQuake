#pragma once

#include "Common.h"
#include "MemoryManager.h"
#include "Parameters.h"
#include "System.h"

class Host
{
public:
   Host();
   ~Host();
   void Init(Parameters* pParameters, System* pSystem);

protected:
   void GetWorkingDirectory();
   Parameters* m_pParameters;
   MemoryManager m_Memorymanager;
   Common m_Common;

};
