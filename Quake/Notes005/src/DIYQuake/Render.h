#pragma once
#include "Model.h"

class Client;
class Video;

struct Entity
{
   ModelData* model;
};

class Render
{
public:
   void Init(Video* pVideo, Client* pClient, MemoryManager* pMemorymanager);
   void RenderView(void);

private:
   void DrawEntitiesOnList(void);
   void AliasDrawModel(Entity* pCurrentEntity);

   Client* m_pClient;
   Video* m_pVideo;
   MemoryManager* m_pMemorymanager;
};
