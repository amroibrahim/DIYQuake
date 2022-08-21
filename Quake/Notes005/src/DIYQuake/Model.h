#pragma once

#include <cstdint>
#include "MathLib.h"
#include "MemoryManager.h"

#define MAX_KNOWN_MODEL	256
#define MAX_MODEL_PATH_LENGTH 64 

#define IDPOLYHEADER	(('O'<<24)+('P'<<16)+('D'<<8)+'I')

enum SYNC_TYPE
{
   SYNC,
   RAND
};

enum MODELTYPE
{
   BRUSH,
   SPRITE,
   ALIAS
};

enum MODELLOADSTATUS
{
   PRESENT,
   NEEDS_LOADING,
   UNREFERENCED
};

enum ALIASSKINTYPE
{
   ALIAS_SKIN_SINGLE,
   ALIAS_SKIN_GROUP
};

struct ModelData
{
   char szName[MAX_MODEL_PATH_LENGTH];
   MODELLOADSTATUS eLoadStatus;
   MODELTYPE eType;

   //int32_t iFlags;

   Cache::CacheData pCachData;
};

struct ModelHeader
{
   int32_t iID;
   int32_t iVersion;
   Vec3 Scale;
   Vec3 ScaleOrigin;
   float fBoundingRadius;
   Vec3 EyePosition;
   int32_t iNumSkins;
   int32_t iSkinWidth;
   int32_t iSkinHeight;
   int32_t iNumVerts;
   int32_t iNumTrianglis;
   int32_t iNumFrames;
   SYNC_TYPE eSyncType;
   int32_t iFlags;
   float fSize;
};

struct AliasModelHeader
{
   int32_t model;
   int32_t stverts;
   int32_t SkinDescOffset;
   int32_t triangles;
};

struct AliasSkinType
{
   ALIASSKINTYPE eSkinType;
};

struct AliasSkinDesc
{
   ALIASSKINTYPE eSkinType;
   int32_t skin;
};