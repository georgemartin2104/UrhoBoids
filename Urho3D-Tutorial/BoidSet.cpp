/*#include "BoidSet.h"

void BoidSet::Initialise(ResourceCache* pRes, Scene* pScene)
{
	for (int i = 0; i < NumBoids; i++)
	{
		boidList[i].Initialise(pRes, pScene);
	}
}

void BoidSet::Update(float tm)
{
	for (int i = 0; i < NumBoids; i++)
	{
		boidList[i].ComputeForce(&boidList[0]);
		boidList[i].Update(tm);
	}
}*/