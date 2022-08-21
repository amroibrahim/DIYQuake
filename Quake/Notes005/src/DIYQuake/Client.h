#pragma once

#include "Model.h"
#include "ModelManager.h"
#include "Render.h"

struct ClientData
{
   ModelData* pModels[MAX_KNOWN_MODEL];
};

class Client
{
public:
   void Init(ModelManager* pModelManager);

   void TEMP_LoadPlayerModel(void);


   ModelManager* m_pModelManager;

   ClientData m_ClientData;

   int cl_numvisedicts;
   Entity Entities[256];
};

