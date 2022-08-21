#include "Render.h"
#include "Client.h"
#include "Video.h"

void Render::Init(Video* pVideo, Client* pClient, MemoryManager* pMemorymanager)
{
   m_pVideo = pVideo;
   m_pClient = pClient;
   m_pMemorymanager = pMemorymanager;
}

void Render::RenderView(void)
{
   DrawEntitiesOnList();
}

void Render::DrawEntitiesOnList(void)
{
   Entity* pCurrentEntity = m_pClient->Entities;

   switch (pCurrentEntity->model->eType)
   {

   case ALIAS:
      AliasDrawModel(pCurrentEntity);
      break;
   }
}

void Render::AliasDrawModel(Entity* pCurrentEntity)
{
   AliasModelHeader* pAliasModelHeader = (AliasModelHeader*)m_pClient->m_pModelManager->ExtraData(pCurrentEntity->model);
   ModelHeader* pModel = (ModelHeader*)&pAliasModelHeader[1];
   AliasSkinDesc* pAliasSkinDesc = (AliasSkinDesc*)((byte_t*)pAliasModelHeader + pAliasModelHeader->SkinDescOffset);

   byte_t* pSkin = (byte_t*)pAliasModelHeader + pAliasSkinDesc->skin;
   byte_t* pScreenBuffer = m_pVideo->GetFrameBuffer();

   if (pAliasSkinDesc->eSkinType == ALIAS_SKIN_SINGLE)
   {

      for (int y = 0; y < pModel->iSkinHeight; ++y)
      {
         for (int x = 0; x < pModel->iSkinWidth; ++x)
         {
            pScreenBuffer[y * m_pVideo->GetWidth() + x] = *pSkin;
            pSkin++;
         }
      }
   }
}
