#pragma once
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
namespace Urho3D
{
	class Node;
	class Scene;
	class RigidBody;
	class CollisionShape;
	class ResourceCache;
	const int MaxMissiles = 3;
	

}
using namespace Urho3D;

class Missile
{

public:
	// Constructor
	Missile() { Node* pNode = nullptr; RigidBody* pRigidBody = nullptr; CollisionShape* pCollisionShape = nullptr; StaticModel* pObject = nullptr; };

	Node* pNode;
	RigidBody* pRigidBody;
	CollisionShape* pCollisionShape;
	StaticModel* pObject;
	float delay = 5.0f;
	float timer = 0.0f;
	float currentTime = 0.0f;
	bool isActive = false;
	// Destructor
	~Missile() {};

	void Initialise(ResourceCache* pRes, Scene* pScene);

	void Activate(float timeStep, Node* cameraNode);

	void Update(float timeStep);

	bool CheckActive(bool returnActive);
};

class MissileSet
{
	public:

	Missile MissileList[MaxMissiles];

	MissileSet() {};
	void Initialise(ResourceCache* pRes, Scene* pScene);
	void ActivateMissile(float timeStep, Node* cameraNode);
	void Update(float tm);
};