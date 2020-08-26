#include "boids.h"

float Boid::Range_FAttract = 30.0f;
float Boid::Range_FRepel = 20.0f;
float Boid::Range_FAlign = 5.0f;
float Boid::FAttract_Vmax = 5.0f;
float Boid::FAttract_Factor = 4.0f;
float Boid::FRepel_Factor = 2.0f;
float Boid::FAlign_Factor = 2.0f;



void Boid::Initialise(ResourceCache* pRes, Scene* pScene)
{
	pNode = pScene->CreateChild("Boid");
	pNode->SetPosition(Vector3(Random(40.0f) - 20.0f, 0.0f, Random(40.0f) - 20.0f));
	pNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
	pNode->SetScale(2.0f + Random(5.0f));
	pObject = pNode->CreateComponent<StaticModel>();
	pObject->SetModel(pRes->GetResource<Model>("Models/Cone.mdl"));
	pObject->SetMaterial(pRes->GetResource<Material>("Materials/Stone.xml"));
	pObject->SetCastShadows(true);
	pRigidBody = pNode->CreateComponent<RigidBody>();
	pRigidBody->SetCollisionLayer(2);
	pRigidBody->SetMass(1.0f);
	pRigidBody->SetUseGravity(false);
	pRigidBody->SetPosition(Vector3(Random(40.0f) - 20.0f, 0.0f, Random(40.0f) - 20.0f));
	pCollisionShape = pNode->CreateComponent<CollisionShape>();
	pCollisionShape->SetTriangleMesh(pObject->GetModel(), 0);
}

void Boid::ComputeForce(Boid* pBoid, bool hasRun)
{
	Vector3 CoM; //centre of mass, accumulated total
	int n = 0; //count number of neigbours
	//set the force member variable to zero
	force = Vector3(0, 0, 0);
	//Search Neighbourhood
	if (!hasRun)
	{
		for (int i = 0; i < (NumBoids / 2); i++)
		{
			//the current boid?
			if (this == &pBoid[i]) continue;
			//sep = vector position of this boid from current boid
			Vector3 sep = pRigidBody->GetPosition() - pBoid[i].pRigidBody->GetPosition();
			float d = sep.Length(); //distance of boid
			if (d < Range_FAttract)
			{
				//with range, so is a neighbour
				CoM += pBoid[i].pRigidBody->GetPosition();
				n++;
			}
		}
	}
	if (hasRun)
	{
		for (int i = (NumBoids / 2); i < NumBoids; i++)
		{
			//the current boid?
			if (this == &pBoid[i]) continue;
			//sep = vector position of this boid from current boid
			Vector3 sep = pRigidBody->GetPosition() - pBoid[i].pRigidBody->GetPosition();
			float d = sep.Length(); //distance of boid
			if (d < Range_FAttract)
			{
				//with range, so is a neighbour
				CoM += pBoid[i].pRigidBody->GetPosition();
				n++;
			}
		}
	}
	//Attractive force component
	if (n > 0)
	{
		//find average position = centre of mass
		CoM /= n;
		Vector3 dir = (CoM - pRigidBody->GetPosition()).Normalized();
		Vector3 vDesired = dir * FAttract_Vmax;
		force += (vDesired - pRigidBody->GetLinearVelocity()) * FAttract_Factor;
	}


	n = 0;
	Vector3 temp = Vector3(0, 0, 0);
	if (!hasRun)
	{
		for (int i = 0; i < (NumBoids / 2); i++)
		{
			//the current boid?
			if (this == &pBoid[i]) continue;
			//sep = vector position of this boid from current boid
			Vector3 sep = pRigidBody->GetPosition() - pBoid[i].pRigidBody->GetPosition();
			float d = sep.Length(); //distance of boid
			if (d < Range_FAttract)
			{
				temp += pBoid[i].pRigidBody->GetLinearVelocity();
				n++;
			}
		}
	}
	if (hasRun)
	{
		for (int i = (NumBoids / 2); i < NumBoids; i++)
		{
			//the current boid?
			if (this == &pBoid[i]) continue;
			//sep = vector position of this boid from current boid
			Vector3 sep = pRigidBody->GetPosition() - pBoid[i].pRigidBody->GetPosition();
			float d = sep.Length(); //distance of boid
			if (d < Range_FAttract)
			{
				temp += pBoid[i].pRigidBody->GetLinearVelocity();
				n++;
			}
		}
	}
	if (n > 0)
	{
		temp /= n;
		temp.Normalize();
		temp* FAlign_Factor;

		force += temp - pRigidBody->GetLinearVelocity();
	}


	n = 0;
	if (!hasRun)
	{
		for (int i = 0; i < (NumBoids / 2); i++)
		{
			//the current boid?
			if (this == &pBoid[i]) continue;
			//sep = vector position of this boid from current boid
			Vector3 sep = pRigidBody->GetPosition() - pBoid[i].pRigidBody->GetPosition();
			float d = sep.Length(); //distance of boid
			if (d < Range_FRepel)
			{
				if (d > 0 && d < 100)
				{
					sep.Normalize();
					force += sep * FRepel_Factor;
				}
			}
		}
	}
	if (hasRun)
	{
		for (int i = (NumBoids / 2); i < NumBoids; i++)
		{
			//the current boid?
			if (this == &pBoid[i]) continue;
			//sep = vector position of this boid from current boid
			Vector3 sep = pRigidBody->GetPosition() - pBoid[i].pRigidBody->GetPosition();
			float d = sep.Length(); //distance of boid
			if (d < Range_FRepel)
			{
				if (d > 0 && d < 100)
				{
					sep.Normalize();
					force += sep * FRepel_Factor;
				}
			}
		}
	}
}

void Boid::Update(float timeStep)
{
	pRigidBody->ApplyForce(force);

	Vector3 vel = pRigidBody->GetLinearVelocity();
	float d = vel.Length();
	if (d < 10.0f)
	{
		d = 10.0f;
		pRigidBody->SetLinearVelocity(vel.Normalized() * d);
	}
	else if (d > 50.0f)
	{
		d = 50.0f;
		pRigidBody->SetLinearVelocity(vel.Normalized() * d);
	}

	Vector3 vn = vel.Normalized();
	Vector3 cp = -vn.CrossProduct(Vector3(0.0f, 1.0f, 0.0f));
	float dp = cp.DotProduct(vn);
	pRigidBody->SetRotation(Quaternion(Acos(dp), cp));

	Vector3 p = pRigidBody->GetPosition();
	if (p.y_ < 10.0f)
	{
		p.y_ = 10.0f;
		pRigidBody->SetPosition(p);
	}
	else if (p.y_ > 50.0f)
	{
		p.y_ = 50.0f;
		pRigidBody->SetPosition(p);
	}
}

void BoidSet::Initialise(ResourceCache* pRes, Scene* pScene)
{
	for (int i = 0; i < NumBoids; i++)
	{
		boidList[i].Initialise(pRes, pScene);
	}
}

void BoidSet::Update(float tm)
{
	/*for (int i = 0; i < NumBoids; i++)
	{
		boidList[i].ComputeForce(&boidList[0]);
		boidList[i].Update(tm);
	}*/

	if (!hasRun)
	{
		for (int i = 0; i < (NumBoids / 2); i++)
		{
			boidList[i].ComputeForce(&boidList[0], hasRun);
			boidList[i].Update(tm);
		}
		hasRun = !hasRun;
	}
	if (hasRun)
	{
		for (int i = (NumBoids / 2); i < NumBoids; i++)
		{
			boidList[i].ComputeForce(&boidList[0], hasRun);
			boidList[i].Update(tm);
		}
		hasRun = !hasRun;
	}
}