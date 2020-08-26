//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Urho3D/Core/CoreEvents.h>
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
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Physics/PhysicsEvents.h>

#include "Character.h"
#include "CharacterDemo.h"
#include "Touch.h"
#include "boids.h"
#include "Missile.h"


#include <Urho3D/DebugNew.h>

URHO3D_DEFINE_APPLICATION_MAIN(CharacterDemo)

// Custom remote event we use to tell the client which object they control
static const StringHash E_CLIENTOBJECTAUTHORITY("ClientObjectAuthority");

// Identifier for the node ID parameter in the event data
static const StringHash PLAYER_ID("IDENTITY");

// Custom event on server, client has pressed button that it wants to start game
static const StringHash E_CLIENTISREADY("ClientReadyToStart");

CharacterDemo::CharacterDemo(Context* context) :
    Sample(context),
    firstPerson_(false)
{

	//TUTORIAL: TODO
}

CharacterDemo::~CharacterDemo()
{
}

void CharacterDemo::Start()
{
    // Execute base class startup
    Sample::Start();
    if (touchEnabled_)
        touch_ = new Touch(context_, TOUCH_SENSITIVITY);

	CreateMainMenu();

	// Create static scene content
	CreateScene();

	// Subscribe to necessary events
	SubscribeToEvents();

	// Set the mouse mode to use in the sample
	Sample::InitMouseMode(MM_RELATIVE);

	//TUTORIAL: TODO
}

void CharacterDemo::CreateScene()
{
	//so we can access resources
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	scene_ = new Scene(context_);
	// Create scene subsystem components
	scene_->CreateComponent<Octree>();
	scene_->CreateComponent<PhysicsWorld>();

	//Create camera node and component
	cameraNode_ = new Node(context_);
	Camera* camera = cameraNode_->CreateComponent<Camera>();
	cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
	camera->SetFarClip(300.0f);

	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));

	// Create static scene content. First create a zone for ambient lighting and fog control
	Node* zoneNode = scene_->CreateChild("Zone");
	Zone* zone = zoneNode->CreateComponent<Zone>();
	zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
	zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
	zone->SetFogStart(100.0f);
	zone->SetFogEnd(300.0f);
	zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

	// Create a directional light with cascaded shadow mapping
	Node* lightNode = scene_->CreateChild("DirectionalLight");
	lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
	Light* light = lightNode->CreateComponent<Light>();
	light->SetLightType(LIGHT_DIRECTIONAL);
	light->SetCastShadows(true);
	light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
	light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
	light->SetSpecularIntensity(0.5f);

	// Create the floor object
	Node* floorNode = scene_->CreateChild("Floor");
	floorNode->SetPosition(Vector3(0.0f, -0.5f, 0.0f));
	floorNode->SetScale(Vector3(200.0f, 1.0f, 200.0f));
	StaticModel* object = floorNode->CreateComponent<StaticModel>();
	object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	object->SetMaterial(cache -> GetResource<Material>("Materials/Stone.xml"));
	RigidBody* body = floorNode->CreateComponent<RigidBody>();
	// Use collision layer bit 2 to mark world scenery. This is what we will raycast against to prevent camera from going inside geometry
	body->SetCollisionLayer(2);
	CollisionShape* shape = floorNode->CreateComponent<CollisionShape>();
	shape->SetBox(Vector3::ONE);

	// Initialise Boids
	boids.Initialise(cache, scene_);

	// Initialise Missiles
	missile.Initialise(cache, scene_);

	// Create mushrooms of varying sizes
	const unsigned NUM_MUSHROOMS = 60;
	for (unsigned i = 0; i < NUM_MUSHROOMS; ++i)
	{
		Node* objectNode = scene_->CreateChild("Mushroom");
		objectNode->SetPosition(Vector3(Random(180.0f) - 90.0f, 0.0f,
			Random(180.0f) - 90.0f));
		objectNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
		objectNode->SetScale(2.0f + Random(5.0f));
		StaticModel* object = objectNode->CreateComponent<StaticModel>();
		object->SetModel(cache -> GetResource<Model>("Models/Mushroom.mdl"));
		object->SetMaterial(cache -> GetResource<Material>("Materials/Mushroom.xml"));
		object->SetCastShadows(true);
		RigidBody* body = objectNode->CreateComponent<RigidBody>();
		body->SetCollisionLayer(2);
		CollisionShape* shape = objectNode -> CreateComponent<CollisionShape>();
		shape->SetTriangleMesh(object->GetModel(), 0);
	}

	const unsigned NUM_BOXES = 100;
	for (unsigned i = 0; i < NUM_BOXES; ++i)
	{
		float scale = Random(2.0f) + 0.5f;
		Node* objectNode = scene_->CreateChild("Box");
		objectNode->SetPosition(Vector3(Random(180.0f) - 90.0f,
			Random(10.0f) + 10.0f, Random(180.0f) - 90.0f));
		objectNode->SetRotation(Quaternion(Random(360.0f),
			Random(360.0f), Random(360.0f)));
		objectNode->SetScale(scale);
		StaticModel* object = objectNode->CreateComponent<StaticModel>();
		object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
		object->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
		object->SetCastShadows(true);
		RigidBody* body = objectNode->CreateComponent<RigidBody>();
		body->SetCollisionLayer(2);
		// Bigger boxes will be heavier and harder to move
		body->SetMass(scale * 2.0f);
		CollisionShape* shape = objectNode->CreateComponent<CollisionShape>();
		shape->SetBox(Vector3::ONE);
	}

	//TUTORIAL: TODO

}

void CharacterDemo::CreateCharacter()
{
	//TUTORIAL: TODO
}

void CharacterDemo::CreateInstructions()
{
	//TUTORIAL: TODO
}

void CharacterDemo::SubscribeToEvents()
{

	// Subscribe to Update event for setting the character controls
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CharacterDemo, HandleUpdate));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(CharacterDemo, HandlePostUpdate));
	SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(CharacterDemo, HandleClientConnected));
	SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(CharacterDemo, HandleClientDisconnected));
	SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(CharacterDemo, HandlePhysicsPreStep));
	SubscribeToEvent(E_CLIENTSCENELOADED, URHO3D_HANDLER(CharacterDemo, HandleClientFinishedLoading));
	SubscribeToEvent(E_CLIENTISREADY, URHO3D_HANDLER(CharacterDemo, HandleClientToServerReadyToStart));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTISREADY);
	SubscribeToEvent(E_CLIENTOBJECTAUTHORITY, URHO3D_HANDLER(CharacterDemo, HandleServerToClientObjectID));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTOBJECTAUTHORITY);
	//TUTORIAL: TODO
}

void CharacterDemo::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;
	// Take the frame time step, which is stored as a float
	float timeStep = eventData[P_TIMESTEP].GetFloat();
	// Do not move if the UI has a focused element (the console)
	if (GetSubsystem<UI>()->GetFocusElement()) return;
	Input* input = GetSubsystem<Input>();
	// Movement speed as world units per second
	const float MOVE_SPEED = 20.0f;
	// Mouse sensitivity as degrees per pixel
	const float MOUSE_SENSITIVITY = 0.1f;
	// Use this frame's mouse motion to adjust camera node yaw and pitch.
	// Clamp the pitch between -90 and 90 degrees
	IntVector2 mouseMove = input->GetMouseMove();
	yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
	pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
	pitch_ = Clamp(pitch_, -90.0f, 90.0f);
	// Construct new orientation for the camera scene node from
	// yaw and pitch. Roll is fixed to zero
	cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
	// Read WASD keys and move the camera scene node to the corresponding
	// direction if they are pressed, use the Translate() function
	// (default local space) to move relative to the node's orientation.
	if (controlsActive)
	{
		if (input->GetKeyDown(KEY_W))
			cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
		if (input->GetKeyDown(KEY_S))
			cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
		if (input->GetKeyDown(KEY_A))
			cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
		if (input->GetKeyDown(KEY_D))
			cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
		if (input->GetKeyPress(KEY_SPACE))
			missile.ActivateMissile(timeStep, cameraNode_);
	}
	if (input->GetKeyPress(KEY_M))
		menuVisible = !menuVisible;

	// Only move the camera if we have a controllable object
	if (clientObjectID_)
	{
		Node* ballNode = this->scene_->GetNode(clientObjectID_);
		if (ballNode)
		{
			const float CAMERA_DISTANCE = 5.0f;
			cameraNode_->SetPosition(ballNode->GetPosition() + cameraNode_->GetRotation()
				* Vector3::BACK * CAMERA_DISTANCE);
		}
	}


	// Update Boids
	boids.Update(timeStep);
	missile.Update(timeStep);
	//TUTORIAL: TODO
}

void CharacterDemo::CreateMainMenu()
{
	// Set the mouse mode to use in the sample
	InitMouseMode(MM_RELATIVE);
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI* ui = GetSubsystem<UI>();
	UIElement* root = ui->GetRoot();
	XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	root->SetDefaultStyle(uiStyle); //need to set default ui style
	/*Create a Cursor UI element.We want to be able to hide and show the main menu
	at will.When hidden, the mouse will control the camera, and when visible, the
	mouse will be able to interact with the GUI.*/
	SharedPtr<Cursor> cursor(new Cursor(context_));
	cursor->SetStyleAuto(uiStyle);
	ui->SetCursor(cursor);
	// Create the Window and add it to the UI's root node
	window_ = new Window(context_);
	root->AddChild(window_);
	// Set Window size and layout settings
	window_->SetMinWidth(384);
	window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	window_->SetAlignment(HA_CENTER, VA_CENTER);
	window_->SetName("Window");
	window_->SetStyleAuto();

	Button* connectButton_ = CreateButton("CONNECT", 24, window_);
	serverAddressLineEdit = CreateLineEdit("Insert Server Address", 24, window_);
	Button* disconnectButton_ = CreateButton("DISCONNECT", 24, window_);
	Button* startServerButton_ = CreateButton("START SERVER", 24, window_);
	Button* startClientGameButton_ = CreateButton("Client: Start Game", 24, window_);
	Button* quitButton_ = CreateButton("QUIT", 48, window_);
	SubscribeToEvent(connectButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleConnect));
	SubscribeToEvent(disconnectButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleDisconnect));
	SubscribeToEvent(startServerButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleStartServer));
	SubscribeToEvent(startClientGameButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleClientStartGame));
	SubscribeToEvent(quitButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleQuit));
}

void CharacterDemo::HandleClientConnected(StringHash eventType, VariantMap& eventData)
{
	Log::WriteRaw("(HandleClientConnected) A client has connected!");
	using namespace ClientConnected;

	// When a client connects, assign to a scene
	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
	newConnection->SetScene(scene_);

}

void CharacterDemo::HandleClientDisconnected(StringHash eventType, VariantMap& eventData)
{
	
}

void CharacterDemo::HandleConnect(StringHash eventType, VariantMap& eventData)
{
	// Clears the scene, prepares to receive scene
	CreateClientScene();

	Network* network = GetSubsystem<Network>();
	String address = serverAddressLineEdit->GetText().Trimmed();
	if (address.Empty()) { address = "localhost"; }

	// Specify scene to use as a client for replication
	network->Connect(address, SERVER_PORT, scene_);
}

void CharacterDemo::HandleDisconnect(StringHash eventType, VariantMap& eventData)
{
	Log::WriteRaw("HandleDisconnect has been pressed. \n");
	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();
	// Running as Client
	if (serverConnection)
	{
		serverConnection->Disconnect();
		scene_->Clear(true, false);
		clientObjectID_ = 0;
	}
	// Running as a server, stop it
	else if (network->IsServerRunning())
	{
		network->StopServer();
		scene_->Clear(true, false);
	}
}


Controls CharacterDemo::FromClientToServerControls()
{
	Input* input = GetSubsystem<Input>();
	Controls controls;

	// Check which button has been pressed, keep track
	controls.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
	controls.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
	controls.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
	controls.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
	controls.Set(1024, input->GetKeyDown(KEY_E));

	// Mouse yaw to server
	controls.yaw_ = yaw_;

	return controls;
}

void CharacterDemo::ProcessClientControls()
{
	Network* network = GetSubsystem<Network>();
	const Vector<SharedPtr<Connection> >& connections = network->GetClientConnections();
	//Server: go through every client connected
	for (unsigned i = 0; i < connections.Size(); ++i)
	{
		Connection* connection = connections[i];
		// Get the object this connection is controlling
		Node* ballNode = serverObjects_[connection];
		// Client has no item connected
		if (!ballNode) continue;
		RigidBody* body = ballNode->GetComponent<RigidBody>();
		// Get the last controls sent by the client
		const Controls& controls = connection->GetControls();
		// Torque is relative to the forward vector
		Quaternion rotation(0.0f, controls.yaw_, 0.0f);
		const float MOVE_TORQUE = 5.0f;
		if (controls.buttons_ & CTRL_FORWARD)
			body->ApplyTorque(rotation * Vector3::RIGHT * MOVE_TORQUE);
		if (controls.buttons_ & CTRL_BACK)
			body->ApplyTorque(rotation * Vector3::LEFT * MOVE_TORQUE);
		if (controls.buttons_ & CTRL_LEFT)
			body->ApplyTorque(rotation * Vector3::FORWARD * MOVE_TORQUE);
		if (controls.buttons_ & CTRL_RIGHT)
			body->ApplyTorque(rotation * Vector3::BACK * MOVE_TORQUE);
	}
}

void CharacterDemo::HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData)
{
	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	// Client: collect controls
	if (serverConnection)
	{
		serverConnection->SetPosition(cameraNode_->GetPosition()); // send camera position
		serverConnection->SetControls(FromClientToServerControls()); // send controls to server
	}

	// Server: Read Controls, Apply them if needed
	else if (network->IsServerRunning())
	{
		ProcessClientControls(); // take data from clients, process it
	}
}

void CharacterDemo::HandleClientFinishedLoading(StringHash eventType, VariantMap& eventData)
{
	printf("Client has finished loading up the scene from the server \n");
}


void CharacterDemo::CreateClientScene()
{

}

void CharacterDemo::CreateServerScene()
{

}

void CharacterDemo::HandleServerToClientObjectID(StringHash eventType, VariantMap& eventData)
{
	clientObjectID_ = eventData[PLAYER_ID].GetUInt();
	printf("Client ID : %i \n", clientObjectID_);
}

void CharacterDemo::HandleClientToServerReadyToStart(StringHash eventType, VariantMap& eventData)
{
	printf("Event sent by the Client and running on Server: Client is ready to start the game \n");
	using namespace ClientConnected;
	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
	// Create a controllable object for that client
	Node* newObject = CreateControllableObject();
	serverObjects_[newConnection] = newObject;
	// Finally send the object's node ID using a remote event
	VariantMap remoteEventData;
	remoteEventData[PLAYER_ID] = newObject->GetID();
	newConnection->SendRemoteEvent(E_CLIENTOBJECTAUTHORITY, true, remoteEventData);
}

void CharacterDemo::HandleClientStartGame(StringHash eventType, VariantMap& eventData)
{
	printf("Client has pressed START GAME \n");
	if (clientObjectID_ == 0) // Client is still observer
	{
		Network* network = GetSubsystem<Network>();
		Connection* serverConnection = network->GetServerConnection();
		if (serverConnection)
		{
			VariantMap remoteEventData;
			remoteEventData[PLAYER_ID] = 0;
			serverConnection->SendRemoteEvent(E_CLIENTISREADY, true, remoteEventData);
		}
	}
}

Node* CharacterDemo::CreateControllableObject()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	// Create the scene node & visual representation. This will be a replicated object
	Node* ballNode = scene_->CreateChild("AClientBall");
	ballNode->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
	ballNode->SetScale(2.5f);
	StaticModel* ballObject = ballNode->CreateComponent<StaticModel>();
	ballObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	ballObject->SetMaterial(cache->GetResource<Material>("Materials/StoneSmall.xml"));

	// Create the physics components
	RigidBody* body = ballNode->CreateComponent<RigidBody>();
	body->SetMass(1.0f);
	body->SetFriction(1.0f);
	// motion damping so that the ball can not accelerate limitlessly
	body->SetLinearDamping(0.25f);
	body->SetAngularDamping(0.25f);
	CollisionShape* shape = ballNode->CreateComponent<CollisionShape>();
	shape->SetSphere(1.0f);
	return ballNode;
}


void CharacterDemo::HandleStartServer(StringHash eventType, VariantMap& eventData)
{
	Log::WriteRaw("(HandleStartServer called) Server is started!");

	Network* network = GetSubsystem<Network>();
	network->StartServer(SERVER_PORT);

	// Makes menu disappear
	menuVisible = !menuVisible;
}

void CharacterDemo::HandleQuit(StringHash eventType, VariantMap& eventData)
{
	engine_->Exit();
}

void CharacterDemo::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
	UI* ui = GetSubsystem<UI>();
	Input* input = GetSubsystem<Input>();
	ui->GetCursor()->SetVisible(menuVisible);
	window_->SetVisible(menuVisible);

	if (!ui->GetCursor()->IsVisible())
	{
		controlsActive = true;
	}
	else if (ui->GetCursor()->IsVisible())
	{
		controlsActive = false;
	}
	//TUTORIAL: TODO
}
