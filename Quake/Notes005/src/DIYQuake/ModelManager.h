#pragma once

#include "Model.h"
#include "MemoryManager.h"

#include "Common.h"


class ModelManager
{
public:
   void Init(MemoryManager* pMemorymanager, Common* pCommon);
   void LoadHeader(char* szName);

   void LoadAliasModel(ModelData* pModel, byte_t* pBuffer, char* szHunkName);

   ModelData* Load(char* szName);
   ModelData* Load(ModelData* pModel);

   ModelData* Find(char* szName);

   void* ExtraData(ModelData* pModel);

protected:
   void* LoadAliasSkin(void* pTempModel, int32_t* pSkinOffset, int32_t iSkinSize, AliasModelHeader* pHeader, std::string& sHunkName);
   void* LoadAliasSkinGroup(void* pTempModel, int32_t* pSkinOffset, int32_t iSkinSize, AliasModelHeader* pHeader, std::string& sHunkName);


   MemoryManager* m_pMemorymanager;
   Common* m_pCommon;

   ModelData m_pKnownModels[MAX_KNOWN_MODEL];
   int32_t m_iKnownModelCount;
};

