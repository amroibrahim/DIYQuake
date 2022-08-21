#include "ModelManager.h"

#include <string>

void ModelManager::Init(MemoryManager* pMemorymanager, Common* pCommon)
{
   m_pMemorymanager = pMemorymanager;
   m_pCommon = pCommon;
}

ModelData* ModelManager::Load(char* szName)
{
   ModelData* pModelData = nullptr;

   pModelData = Find(szName);

   return Load(pModelData);
}

//Load header to Known list
void ModelManager::LoadHeader(char* szName)
{
   ModelData* pModelData = Find(szName);

   if (pModelData->eLoadStatus == MODELLOADSTATUS::PRESENT)
   {
      if (pModelData->eType == MODELTYPE::ALIAS)
      {
         m_pMemorymanager->Check(&pModelData->pCachData);
      }
   }
}

void ModelManager::LoadAliasModel(ModelData* pModel, byte_t* pBuffer, char* szHunkName)
{
   int32_t iMemoryStartOffset = m_pMemorymanager->GetLowUsed();

   ModelHeader* pTempModelHeader = (ModelHeader*)pBuffer;

   int32_t iVersion = pTempModelHeader->iVersion;
   int32_t iSize = sizeof(AliasModelHeader) + sizeof(ModelHeader);

   std::string sloadName(szHunkName);
   AliasModelHeader* pAliasModelHeader = (AliasModelHeader*)m_pMemorymanager->NewLowEndNamed(iSize, sloadName);

   ModelHeader* pModelHeader = (ModelHeader*)((byte_t*)&pAliasModelHeader[1]);

   // pModel->iFlags = pTempModelHeader->iFlags;

   pModelHeader->iNumSkins = pTempModelHeader->iNumSkins;
   pModelHeader->iSkinWidth = pTempModelHeader->iSkinWidth;
   pModelHeader->iSkinHeight = pTempModelHeader->iSkinHeight;

   int32_t iNumberOfSkins = pModelHeader->iNumSkins;

   // Calculate the skin size in bytes
   int32_t iSkinSize = pModelHeader->iSkinHeight * pModelHeader->iSkinWidth;

   AliasSkinType* pSkinType = (AliasSkinType*)&pTempModelHeader[1];

   AliasSkinDesc* pSkinDesc = (AliasSkinDesc*)m_pMemorymanager->NewLowEndNamed(iNumberOfSkins * sizeof(AliasSkinDesc), sloadName);

   pAliasModelHeader->SkinDescOffset = (byte_t*)pSkinDesc - (byte_t*)pAliasModelHeader;

   for (int i = 0; i < iNumberOfSkins; i++)
   {
      pSkinDesc[i].eSkinType = pSkinType->eSkinType;

      if (pSkinType->eSkinType == ALIAS_SKIN_SINGLE)
      {
         pSkinType = (AliasSkinType*)LoadAliasSkin(pSkinType + 1, &pSkinDesc[i].skin, iSkinSize, pAliasModelHeader, sloadName);
      }
      else
      {
         pSkinType = (AliasSkinType*)LoadAliasSkinGroup(pSkinType + 1, &pSkinDesc[i].skin, iSkinSize, pAliasModelHeader, sloadName);
      }
   }

   pModel->eType = MODELTYPE::ALIAS;

   int32_t iMemoryEndOffset = m_pMemorymanager->GetLowUsed();
   int32_t iTotalMemorySize = iMemoryEndOffset - iMemoryStartOffset;

   m_pMemorymanager->m_Cache.NewNamed(&pModel->pCachData, iTotalMemorySize, sloadName);

   if (!pModel->pCachData.pData)
      return;

   memcpy(pModel->pCachData.pData, pAliasModelHeader, iTotalMemorySize);

   m_pMemorymanager->DeleteToLowMark(iMemoryStartOffset);
}

ModelData* ModelManager::Find(char* szName)
{
   ModelData* pAvailable = NULL;
   int  iCurrentModelIndex = 0;

   // Is the model already loaded?
   ModelData* pCurrentModel = m_pKnownModels;

   while (iCurrentModelIndex < m_iKnownModelCount)
   {
      if (!strcmp(pCurrentModel->szName, szName))
      {
         break;
      }

      if ((pCurrentModel->eLoadStatus == MODELLOADSTATUS::UNREFERENCED) && (!pAvailable || pCurrentModel->eType != MODELTYPE::ALIAS))
      {
         pAvailable = pCurrentModel;
      }

      ++iCurrentModelIndex;
      ++pCurrentModel;
   }

   if (iCurrentModelIndex == m_iKnownModelCount)
   {
      if (m_iKnownModelCount == MAX_KNOWN_MODEL)
      {
         if (pAvailable)
         {
            pCurrentModel = pAvailable;
            if (pCurrentModel->eType == MODELTYPE::ALIAS && m_pMemorymanager->Check(&pCurrentModel->pCachData))
            {
               m_pMemorymanager->CacheEvict(&pCurrentModel->pCachData);
            }
         }
      }
      else
      {
         ++m_iKnownModelCount;
      }
      strcpy(pCurrentModel->szName, szName);
      pCurrentModel->eLoadStatus = MODELLOADSTATUS::NEEDS_LOADING;
   }

   return pCurrentModel;
}

void* ModelManager::ExtraData(ModelData* pModel)
{
   void* pData;

   pData = m_pMemorymanager->m_Cache.Check(&pModel->pCachData);
   if (pData)
      return pData;

   Load(pModel);

   return pModel->pCachData.pData;
}

ModelData* ModelManager::Load(ModelData* pModel)
{
   if (pModel->eType == MODELTYPE::ALIAS)
   {
      if (m_pMemorymanager->Check(&pModel->pCachData))
      {
         pModel->eLoadStatus = MODELLOADSTATUS::PRESENT;
         return pModel;
      }
   }
   else
   {
      if (pModel->eLoadStatus == MODELLOADSTATUS::PRESENT)
      {
         return pModel;
      }
   }

   byte_t TempBuffer[1024]; // 1K on stack! Load the model temporary on stack (if they fit)
   byte_t* pBuffer = m_pCommon->LoadFile(pModel->szName, TempBuffer, sizeof(TempBuffer));

   char szHunkName[32] = { 0 };
   m_pCommon->GetFileBaseName(pModel->szName, szHunkName);

   //TODO: 
   //loadmodel = pModel;

   pModel->eLoadStatus = MODELLOADSTATUS::PRESENT;

   switch (*(uint32_t*)pBuffer)
   {
   case IDPOLYHEADER:
      LoadAliasModel(pModel, (byte_t*)pBuffer, szHunkName);
      break;

      //case IDSPRITEHEADER:
      //   LoadSpriteModel(pModel, pBuffer);
      //   break;

      //default:
      //   LoadBrushModel(pModel, pBuffer);
      //   break;
   }


   return pModel;
}

void* ModelManager::LoadAliasSkin(void* pTempModel, int32_t* pSkinOffset, int32_t iSkinSize, AliasModelHeader* pHeader, std::string& sHunkName)
{
   byte_t* pSkin = (byte_t*)m_pMemorymanager->NewLowEndNamed(iSkinSize, sHunkName);
   byte_t* pSkinInTemp = (byte_t*)pTempModel;

   *pSkinOffset = (byte_t*)pSkin - (byte_t*)pHeader;

   memcpy(pSkin, pSkinInTemp, iSkinSize);

   pSkinInTemp += iSkinSize;

   return  ((void*)pSkinInTemp);
}

void* ModelManager::LoadAliasSkinGroup(void* pTempModel, int32_t* pSkinOffset, int32_t iSkinSize, AliasModelHeader* pHeader, std::string& sHunkName)
{
   return nullptr;
}
