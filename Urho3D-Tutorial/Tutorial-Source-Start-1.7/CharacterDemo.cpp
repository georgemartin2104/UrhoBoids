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

#include "Character.h"
#include "CharacterDemo.h"
#include "Touch.h"

#include <Urho3D/DebugNew.h>

URHO3D_DEFINE_APPLICATION_MAIN(CharacterDemo)

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

	//TUTORIAL: TODO
}

void CharacterDemo::CreateScene()
{
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
	//TUTORIAL: TODO
}

void CharacterDemo::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	//TUTORIAL: TODO
}

void CharacterDemo::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
	//TUTORIAL: TODO
}
