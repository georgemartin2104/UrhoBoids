#include "Missile.h"

void Missile::Initialise(ResourceCache* pRes, Scene* pScene)
{
	pNode = pScene->CreateChild("Missile");
	pNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	pNode->SetRotation(Quaternion(0.0f, 0.0f, 0.0f));
	pNode->SetScale(1.0f);
	pObject = pNode->CreateComponent<StaticModel>();
	pObject->SetModel(pRes->GetResource<Model>("Models/Cone.mdl"));
	pObject->SetMaterial(pRes->GetResource<Material>("Materials/Stone.xml"));
	pObject->SetCastShadows(true);
	pRigidBody = pNode->CreateComponent<RigidBody>();
	pRigidBody->SetCollisionLayer(2);
	pRigidBody->SetMass(1.0f);
	pRigidBody->SetUseGravity(false);
	pRigidBody->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	pObject->SetEnabled(false);
	isActive = false;
}

void Missile::Activate(float timeStep, Node* cameraNode)
{
	timer = currentTime + delay;
	isActive = true;
	pObject->SetEnabled(true);
	pRigidBody->SetPosition(cameraNode->GetPosition());
	pRigidBody->SetLinearVelocity(cameraNode->GetDirection().Normalized() * 20.0f);
}

bool Missile::CheckActive(bool returnActive)
{
	if (isActive == true)
	{
		returnActive = true;
	}
	if (isActive == false)
	{
		returnActive = false;
	}
	return returnActive;
}

void Missile::Update(float timeStep)
{
	currentTime = currentTime + timeStep;
	if (currentTime > timer)
	{
		pObject->SetEnabled(false);
		isActive = false;
	}
	if (isActive == true)
	{
		pRigidBody->ApplyForce(Vector3(1, 0, 0));
	}
}

void MissileSet::Initialise(ResourceCache* pRes, Scene* pScene)
{
	for (int i = 0; i < MaxMissiles; i++)
	{
		MissileList[i].Initialise(pRes, pScene);
	}
}

void MissileSet::ActivateMissile(float timestep, Node* cameraNode)
{
	bool newMissile = false;
	for (int i = 0; i < MaxMissiles; i++)
	{
		bool check = false;
		bool temp = MissileList[i].CheckActive(check);
		if (temp == false && newMissile == false)
		{
			MissileList[i].Activate(timestep, cameraNode);
			newMissile = true;
		}
	}
}

void MissileSet::Update(float tm)
{
	for (int i = 0; i < MaxMissiles; i++)
	{
		MissileList[i].Update(tm);
	}
}