#include "Client.h"
#include "Render.h"

void Client::Init(ModelManager* pModelManager)
{
   m_pModelManager = pModelManager;
}

void Client::TEMP_LoadPlayerModel(void)
{
   char szPlayerMdl[32];
   strcpy(szPlayerMdl, "progs/player.mdl");
   //strcpy(szPlayerMdl, "progs/soldier.mdl");
   //strcpy(szPlayerMdl, "progs/ogre.mdl");
   //strcpy(szPlayerMdl, "progs/hknight.mdl"); 
   //strcpy(szPlayerMdl, "progs/wizard.mdl");
   //strcpy(szPlayerMdl, "progs/zombie.mdl");
   //strcpy(szPlayerMdl, "progs/fish.mdl");
   //strcpy(szPlayerMdl, "progs/oldone.mdl");
   //strcpy(szPlayerMdl, "progs/boss.mdl");
   //strcpy(szPlayerMdl, "progs/armor.mdl");
   //strcpy(szPlayerMdl, "progs/g_shot.mdl");

   m_ClientData.pModels[0] = m_pModelManager->Load(szPlayerMdl);
   Entities[0].model = m_ClientData.pModels[0];
}
