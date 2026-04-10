#include "stdafx.h"

#if defined(__ANDROID__)

#include "ZzzLodTerrain.h"

vec3_t PrimaryTerrainLight[TERRAIN_SIZE * TERRAIN_SIZE] = {};
vec3_t BackTerrainLight[TERRAIN_SIZE * TERRAIN_SIZE] = {};
vec3_t TerrainLight[TERRAIN_SIZE * TERRAIN_SIZE] = {};
float PrimaryTerrainHeight[TERRAIN_SIZE * TERRAIN_SIZE] = {};
float BackTerrainHeight[TERRAIN_SIZE * TERRAIN_SIZE] = {};
unsigned char TerrainMappingLayer1[TERRAIN_SIZE * TERRAIN_SIZE] = {};
unsigned char TerrainMappingLayer2[TERRAIN_SIZE * TERRAIN_SIZE] = {};
float TerrainMappingAlpha[TERRAIN_SIZE * TERRAIN_SIZE] = {};
WORD TerrainWall[TERRAIN_SIZE * TERRAIN_SIZE] = {};

float SelectXF = 0.0f;
float SelectYF = 0.0f;
int CurrentLayer = 0;

const float g_fMinHeight = 0.0f;
const float g_fMaxHeight = 0.0f;

void CreateFrustrum(float Aspect, vec3_t position)
{
	(void)Aspect;
	(void)position;
}

float RequestTerrainHeight(float xf, float yf)
{
	(void)xf;
	(void)yf;
	return 0.0f;
}

void AddTerrainLight(float xf,float yf,vec3_t Light,int Range,vec3_t *Buffer)
{
	(void)xf;
	(void)yf;
	(void)Light;
	(void)Range;
	(void)Buffer;
}

bool TestFrustrum(vec3_t Position,float Range)
{
	(void)Position;
	(void)Range;
	return true;
}

bool TestFrustrum2D(float x,float y,float Range)
{
	(void)x;
	(void)y;
	(void)Range;
	return true;
}

#endif
