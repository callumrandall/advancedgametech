#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../CSC8503Common/PositionConstraint.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);
	degrees = 0;
	timer = 0;
	wintimer = 0;
	deadtimer = 0;
	forceMagnitude	= 10.0f;
	useGravity		= true;
	inSelectionMode = false;
	won = false;
	lost = false;
	chase = false;
	menu = false;

	//state
	enemyMachine = new StateMachine();
	State* Chase = new State([&](float dt)->void {
		//std::cout << "Chasing" << std::endl;
		moveEnemy(chaseStart, chaseEnd);
		}
	);

	State* Collect = new State([&](float dt)->void {
		//std::cout << "Collecting" << std::endl;
		moveEnemy(chaseStart, chaseEnd);
		}
	);

	StateTransition* ChaseToCollect = new StateTransition(Chase, Collect, [&](void)->bool {
		if (!chase) {
			return true;
		}
		else {
			return false;
		}
		}
	);

	StateTransition* CollectToChase = new StateTransition(Collect, Chase, [&](void)->bool {
		if (chase) {
			return true;
		}
		else {
			return false;
		}
		}
	);

	enemyMachine->AddState(Chase);
	enemyMachine->AddState(Collect);
	enemyMachine->AddTransition(ChaseToCollect);
	enemyMachine->AddTransition(CollectToChase);


	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("Male1.msh"	 , &charMeshA);
	loadFunc("courier.msh"	 , &charMeshB);
	loadFunc("security.msh"	 , &enemyMesh);
	loadFunc("coin.msh"		 , &bonusMesh);
	loadFunc("capsule.msh"	 , &capsuleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	//InitWorld();
	InitMenu();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}

	UpdateKeys();

	if (world1 && !menu) {
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			floor->GetPhysicsObject()->SetAngularVelocity(Vector3(-1, 0, 0) * 0.4);
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			floor->GetPhysicsObject()->SetAngularVelocity(Vector3(1, 0, 0) * 0.4);
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			floor->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, -1) * 0.4);
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			floor->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 1) * 0.4);
		}

		if (floor->GetTransform().GetOrientation().x < -0.15) {
			Quaternion reset = floor->GetTransform().GetOrientation();
			reset.x = -0.15;
			floor->GetTransform().SetOrientation(reset);
		}

		if (floor->GetTransform().GetOrientation().x > 0.15) {
			Quaternion reset = floor->GetTransform().GetOrientation();
			reset.x = 0.15;
			floor->GetTransform().SetOrientation(reset);
		}

		if (floor->GetTransform().GetOrientation().z < -0.15) {
			Quaternion reset = floor->GetTransform().GetOrientation();
			reset.z = -0.15;
			floor->GetTransform().SetOrientation(reset);
		}

		if (floor->GetTransform().GetOrientation().z > 0.15) {
			Quaternion reset = floor->GetTransform().GetOrientation();
			reset.z = 0.15;
			floor->GetTransform().SetOrientation(reset);
		}

		Quaternion resetY = floor->GetTransform().GetOrientation();
		resetY.y = 0;
		floor->GetTransform().SetOrientation(resetY);


		rotateSphere->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, -2));
		rotateCube->GetTransform().SetPosition((rotateCube->GetTransform().GetOriginalPosition()) + spinAnc->GetTransform().GetPosition());
		rotateCube->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 5, 0));
		if (testStateObject) {
			testStateObject->Update(dt);
			testStateObject->GetTransform().SetOrientation(resetY);
			//testStateObject->GetTransform().SetPosition((testStateObject->GetTransform().GetPosition()) + stateAnc->GetTransform().GetPosition());
		}

		if (player->game1fin) {
			renderer->DrawString("You won!", Vector2(40, 5), Vector4(1, 1, 1, 1), 40);
			wintimer += dt;
			if (wintimer > 10) {
				wintimer = 0;
				player->game1fin = false;
				InitWorld();
			}
		}
	}

	if (!world1 && !menu) {
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			player2->GetPhysicsObject()->AddForce(Vector3(0, 0, -7));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			player2->GetPhysicsObject()->AddForce(Vector3(0, 0, 7));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			player2->GetPhysicsObject()->AddForce(Vector3(7, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			player2->GetPhysicsObject()->AddForce(Vector3(-7, 0, 0));
		}

		renderer->DrawString("Points:" + std::to_string(player2->points), Vector2(10, 15));
		renderer->DrawString("Enemy points:" + std::to_string(enemy->points), Vector2(10, 20));

		if ((player2->points + enemy->points) >= 11) {
			if (player2->points > enemy->points) {
				//renderer->DrawString("You won!", Vector2(40, 5), Vector4(1, 1, 1, 1), 40);
				won = true;
			}
			if (enemy->points > player2->points) {
				//renderer->DrawString("You lost!", Vector2(40, 15), Vector4(1, 1, 1, 1), 40);
				lost = true;
			}
		}

		if (!won && player2->dead) {
			lost = true;
			//renderer->DrawString("You lost!", Vector2(40, 15), Vector4(1, 1, 1, 1), 40);
		}

		if (won) {
			wintimer += dt;
			renderer->DrawString("You won!", Vector2(40, 5), Vector4(1, 1, 1, 1), 40);
			if (wintimer > 10) {
				won = false;
				lost = false;
				wintimer = 0;
				player2->points = 0;
				enemy->points = 0;
				InitWorld2();
			}
		}

		if (lost) {
			deadtimer += dt;
			renderer->DrawString("You lost!", Vector2(40, 15), Vector4(1, 1, 1, 1), 40);
			if (deadtimer > 10) {
				won = false;
				lost = false;
				deadtimer = 0;
				player2->points = 0;
				enemy->points = 0;
				InitWorld2();
			}
		}

		enemyMachine->Update(dt);

		/*if (player2->points >= 10) {
			renderer->DrawString("You won!", Vector2(40, 5), Vector4(1, 1, 1, 1), 40);
			wintimer += dt;
			if (wintimer > 10) {
				wintimer = 0;
				player2->points = 0;
				InitWorld2();
			}
		}

		if (player2->dead && player2->points < 10) {
			renderer->DrawString("You died!", Vector2(40, 15), Vector4(1, 1, 1, 1), 40);
			deadtimer += dt;
			if (deadtimer > 10) {
				deadtimer = 0;
				player2->points = 0;
				player2->dead = false;
				InitWorld2();
			}
		}*/
	}
	
	floor->Update();

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95));
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95));
	}

	timer += dt;
	//float seconds = timer

	if (!menu) {
		renderer->DrawString("Timer:" + std::to_string(timer), Vector2(10, 10));
	}

	if (menu) {
		Debug::Print("F1 to start/reset level one", Vector2(25, 45));
		Debug::Print("F2 to start/reset level two", Vector2(25, 50));
		Debug::Print("F3 to return to menu", Vector2(25, 55));
		Debug::Print("P to see demo scene", Vector2(25, 60));
	}

	SelectObject();
	MoveSelectedObject();

	/*if (testStateObject) {
		testStateObject->Update(dt);
	}*/

	physics->Update(dt);

	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0,1,0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);

		Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
	}

	//ice->GetRenderObject()->SetColour(Vector4(0, 0, 1, 0));
	
	world->UpdateWorld(dt);
	renderer->Update(dt);

	Debug::FlushRenderables(dt);
	renderer->Render();
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject	= nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitWorld2(); //We can reset the simulation2 at any time with F2
		selectionObject = nullptr;
		lockedObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F3)) {
		InitMenu(); //We can reset to menu at any time with F3
		selectionObject = nullptr;
		lockedObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
		InitDemo(); //We can reset to demo at any time with p
		selectionObject = nullptr;
		lockedObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F4)) {
		InitCamera(); //F4 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}

	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward  = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 100.0f;

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		Vector3 worldPos = selectionObject->GetTransform().GetPosition();
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}

}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world1 = true;
	menu = false;
	timer = 0;
	world->ClearAndErase();
	physics->Clear();
	physics->UseGravity(true);

	world->GetMainCamera()->SetPosition(Vector3(0, 350, 0));
	world->GetMainCamera()->SetPitch(-90.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	floor = AddOBBToWorld(Vector3(0, 0, 0), Vector3(100, 1, 100), 0);
	GameObject* wall1 = AddOBBToWorld(Vector3(0, 20, 100), Vector3(100, 20, 1), 0);
	GameObject* wall2 = AddOBBToWorld(Vector3(100, 20, 0), Vector3(1, 20, 100), 0);
	GameObject* wall3 = AddOBBToWorld(Vector3(0, 20, -100), Vector3(100, 20, 1), 0);
	GameObject* wall4 = AddOBBToWorld(Vector3(-100, 20, 0), Vector3(1, 20, 100), 0);

	GameObject* wall5 = AddOBBToWorld(Vector3(20, 20, 50), Vector3(80, 20, 1), 0);
	GameObject* wall6 = AddOBBToWorld(Vector3(-20, 20, 0), Vector3(80, 20, 1), 0);
	GameObject* wall7 = AddOBBToWorld(Vector3(20, 20, -50), Vector3(80, 20, 1), 0);


	rotateSphere = AddSphereToWorld(Vector3(0, -22, 75), 27, 0.1, false);
	rotateSphere->GetPhysicsObject()->SetElasticity(1);

	GameObject* sphereAnc = AddOBBToWorld(Vector3(0, -15, 75), Vector3(1, 1, 1), 0);
	PositionConstraint* constr = new PositionConstraint(sphereAnc, rotateSphere, 0);
	world->AddConstraint(constr);

	rotateCube = AddOBBToWorld(Vector3(0, 0, 0), Vector3(2, 20, 20), 0.001);
	rotateCube->GetPhysicsObject()->SetFriction(0);
	spinAnc = AddOBBToWorld(Vector3(0, 6, 25), Vector3(1, 1, 1), 0);

	rope = AddOBBToWorld(Vector3(0, 70, -25), Vector3(2, 2, 2), 0);
	ropeBall = AddSphereToWorld(Vector3(0, 70, -25), 20, 2);
	PositionConstraint* ropeconstr = new PositionConstraint(rope, ropeBall, 49);

	ice = AddOBBToWorld(Vector3(75, 0, 0), Vector3(25, 2, 100), 0);
	ice->GetRenderObject()->SetColour(Vector4(0, 0, 1, 0));
	ice->GetPhysicsObject()->SetFriction(0.01);
	ice->GetPhysicsObject()->SetElasticity(0.1);

	jelly = AddOBBToWorld(Vector3(-75, 0, 0), Vector3(25, 2, 100), 0);
	jelly->GetRenderObject()->SetColour(Vector4(1, 0, 0, 0));
	jelly->GetPhysicsObject()->SetFriction(0.9);
	jelly->GetPhysicsObject()->SetElasticity(1);

	testStateObject = AddStateObjectToWorld(Vector3(0, 50, -75));
	stateAnc = AddOBBToWorld(Vector3(0, 0, -75), Vector3(0.5, 0.5, 0.5), 0);

	finish = AddSphereToWorld(Vector3(75, 10, -75), 10, 0);
	finish->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	finish->SetName("Finish");

	floor->AddChild(wall1);
	floor->AddChild(wall2);
	floor->AddChild(wall3);
	floor->AddChild(wall4);
	floor->AddChild(wall5);
	floor->AddChild(wall6);
	floor->AddChild(wall7);
	floor->AddChild(sphereAnc);
	floor->AddChild(spinAnc);
	floor->AddChild(rope);
	floor->AddChild(ice);
	floor->AddChild(jelly);
	floor->AddChild(stateAnc);
	floor->AddChild(finish);

	player = AddSphereToWorld(Vector3(75, 30, 75), 5, 10);
	player->GetPhysicsObject()->SetElasticity(0.6);
	player->SetName("Player");

	world->AddConstraint(ropeconstr);
}

void TutorialGame::InitWorld2() {
	world1 = false;
	menu = false;
	timer = 0;
	world->ClearAndErase();
	physics->Clear();
	physics->UseGravity(true);

	world->GetMainCamera()->SetPosition(Vector3(100, 350, 100));
	world->GetMainCamera()->SetPitch(-90.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	floor = AddAABBToWorld(Vector3(100, -5, 100), Vector3(100, 1, 100), 0);
	GameObject* wall1 = AddAABBToWorld(Vector3(0, 20, -95), Vector3(90, 20, 10), 0);
	GameObject* wall2 = AddAABBToWorld(Vector3(0, 20, 95), Vector3(90, 20, 10), 0);
	GameObject* wall3 = AddAABBToWorld(Vector3(-95, 20, 0), Vector3(10, 20, 100), 0);
	GameObject* wall4 = AddAABBToWorld(Vector3(95, 20, 0), Vector3(10, 20, 100), 0);

	GameObject* wall5 = AddAABBToWorld(Vector3(-50, 20, 0), Vector3(10, 20, 60), 0);
	GameObject* wall6 = AddAABBToWorld(Vector3(0, 20, 0), Vector3(20, 20, 20), 0);
	GameObject* wall7 = AddAABBToWorld(Vector3(50, 20, 0), Vector3(10, 20, 60), 0);
	GameObject* wall8 = AddAABBToWorld(Vector3(0, 20, 65), Vector3(20, 20, 20), 0);

	floor->AddChild(wall1);
	floor->AddChild(wall2);
	floor->AddChild(wall3);
	floor->AddChild(wall4);
	floor->AddChild(wall5);
	floor->AddChild(wall6);
	floor->AddChild(wall7);
	floor->AddChild(wall8);

	player2 = AddSphereToWorld(Vector3(170, 4, 100), 7, 10);
	player2->SetName("Player");
	player2->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	enemy = AddSphereToWorld(Vector3(20, 4, 100), 7, 10);
	enemy->SetName("Enemy");
	enemy->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	enemy->Chase = true;

	GameObject* pickup = AddSphereToWorld(Vector3(90, 5, 30), 5, 0);
	pickup->SetName("Pickup");
	pickup->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	GameObject* pickup2 = AddSphereToWorld(Vector3(110, 5, 30), 5, 0);
	pickup2->SetName("Pickup");
	pickup2->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	GameObject* pickup3 = AddSphereToWorld(Vector3(30, 5, 30), 5, 0);
	pickup3->SetName("Pickup");
	pickup3->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	GameObject* pickup4 = AddSphereToWorld(Vector3(170, 5, 30), 5, 0);
	pickup4->SetName("Pickup");
	pickup4->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	GameObject* pickup5 = AddSphereToWorld(Vector3(70, 5, 70), 5, 0);
	pickup5->SetName("Pickup");
	pickup5->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	GameObject* pickup6 = AddSphereToWorld(Vector3(130, 5, 70), 5, 0);
	pickup6->SetName("Pickup");
	pickup6->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	GameObject* pickup7 = AddSphereToWorld(Vector3(70, 5, 130), 5, 0);
	pickup7->SetName("Pickup");
	pickup7->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	GameObject* pickup8 = AddSphereToWorld(Vector3(130, 5, 130), 5, 0);
	pickup8->SetName("Pickup");
	pickup8->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	GameObject* pickup9 = AddSphereToWorld(Vector3(30, 5, 170), 5, 0);
	pickup9->SetName("Pickup");
	pickup9->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	GameObject* pickup10 = AddSphereToWorld(Vector3(170, 5, 170), 5, 0);
	pickup10->SetName("Pickup");
	pickup10->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	GameObject* pickup11 = AddSphereToWorld(Vector3(100, 5, 130), 5, 0);
	pickup11->SetName("Pickup");
	pickup11->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
}

void TutorialGame::InitMenu() {
	timer = 0;
	menu = true;
	world1 = false;
	world->ClearAndErase();
	physics->Clear();
	world->GetMainCamera()->SetPosition(Vector3(100, 350, 100));
	world->GetMainCamera()->SetPitch(-90.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	floor = AddOBBToWorld(Vector3(0, 0, 0), Vector3(0, 0, 0), 0);
}

void TutorialGame::InitDemo() {
	timer = 0;
	menu = false;
	world1 = false;
	world->ClearAndErase();
	physics->Clear();
	physics->UseGravity(true);

	world->GetMainCamera()->SetPosition(Vector3(100, 50, 400));
	world->GetMainCamera()->SetPitch(0.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	floor = AddAABBToWorld(Vector3(100, -5, 100), Vector3(200, 1, 200), 0);
	GameObject* cap1 = AddCapsuleToWorld(Vector3(80, 100, 80), 10, 5);
	GameObject* obb = AddOBBToWorld(Vector3(80, 10, 80), Vector3(40, 2, 40), 0);

	GameObject* cap2 = AddCapsuleToWorld(Vector3(160, 100, 160), 10, 5);
	GameObject* sphere = AddSphereToWorld(Vector3(160, 20, 160), 20, 1);

	Quaternion rotate = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), 35);
	obb->GetTransform().SetOrientation(rotate);
	cap1->GetTransform().SetOrientation(-rotate);
	cap2->GetTransform().SetOrientation(rotate);



	player2 = AddSphereToWorld(Vector3(2700, 4, 100), 7, 10);
	player2->SetName("Player");
	player2->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	enemy = AddSphereToWorld(Vector3(2200, 4, 100), 7, 10);
	enemy->SetName("Enemy");
	enemy->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	enemy->Chase = true;

}



GameObject* TutorialGame::AddOBBToWorld(const Vector3& position, const Vector3& size, float inverseMass) {

	GameObject* cube = new GameObject();
	OBBVolume* volume = new OBBVolume(size);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetOriginalPosition(position);
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(size * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddAABBToWorld(const Vector3& position, const Vector3& size, float inverseMass) {

	GameObject* cube = new GameObject();
	AABBVolume* volume = new AABBVolume(size);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetOriginalPosition(position);
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(size * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}




void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 8, 8);
	
	float invCubeMass = 5; // how heavy the middle pieces are
	int numLinks = 10;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 20; // distance between links
	
	Vector3 startPos = Vector3(0, 15, 0);
	
	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);
	
	GameObject * previous = start;
	
	for (int i = 0; i < numLinks; ++i) {
		GameObject * block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint * constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);

}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize	= Vector3(100, 1, 100);
	AABBVolume* volume	= new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddTiltedFloorToWorld(const Vector3& position, float degrees) {
	
	GameObject* floor = new GameObject();
	Vector3 floorSize = Vector3(100, 1, 100);
	OBBVolume* volume = new OBBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetOriginalPosition(position);
	Quaternion rotate = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), degrees);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);
	floor->GetTransform().SetOrientation(rotate);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0.0f);
	floor->GetPhysicsObject()->SetElasticity(0.7f);
	floor->GetPhysicsObject()->SetFriction(0.5f);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	//GameObject* anchor = AddCubeToWorld(Vector3(0, 0, 0), Vector3(0.2, 0.2, 0.2), 0);
	//PositionConstraint* constr = new PositionConstraint(anchor, floor, 0.1);
	//world->AddConstraint(constr);

	return floor;
}

GameObject* TutorialGame::AddWallsToFloor(const Vector3& position, const Vector3& size) {
	GameObject* wall = new GameObject();

	//Vector3 wallSize = Vector3(100, 40, 1);
	AABBVolume* volume = new AABBVolume(size);
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform().SetOriginalPosition(position);
	wall->GetTransform()
		.SetScale(size * 2)
		.SetPosition(position);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0.0f);
	wall->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(wall);
	return wall;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass, bool b) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	volume->setCol(b);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetOriginalPosition(position);
	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();
	sphere->GetPhysicsObject()->SetFriction(0.0f);
	sphere->GetPhysicsObject()->SetElasticity(0.7f);

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius* 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;

}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 3.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 50.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius, 1.0f);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	if (rand() % 2) {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
	}
	else {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
	}
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	//lockedObject = character;

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(0.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* NCL::CSC8503::TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();
	Vector3 size = Vector3(2, 20, 25);
	OBBVolume* volume = new OBBVolume(Vector3(size));
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetOriginalPosition(position);
	apple->GetTransform()
		.SetScale(Vector3(size*2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), cubeMesh, basicTex, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(0.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

Vector3 NCL::CSC8503::TutorialGame::playerPos()
{
	//std::cout << player->GetTransform().GetPosition() << std::endl;
	if (!menu) {
		return player2->GetTransform().GetPosition();
	}
}

Vector3 NCL::CSC8503::TutorialGame::enemyPos()
{
	if (!world1 && !menu) {
		//std::cout << enemy->GetTransform().GetPosition() << std::endl;
		return enemy->GetTransform().GetPosition();
	}
}

void NCL::CSC8503::TutorialGame::moveEnemy(Vector3 start, Vector3 next){
	if (!world1 && !menu) {
		Vector3 direction = (next - start)*0.2;
		direction.y = 0;
		enemy->GetPhysicsObject()->AddForce(direction);
	}
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
				lockedObject	= nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
	}

	if (lockedObject) {
		renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}

	else if(selectionObject){
		renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
		renderer->DrawString("X: " + std::to_string(selectionObject->GetTransform().GetPosition().x), Vector2(75, 5));
		renderer->DrawString("Y: " + std::to_string(selectionObject->GetTransform().GetPosition().y), Vector2(75, 10));
		renderer->DrawString("Z: " + std::to_string(selectionObject->GetTransform().GetPosition().z), Vector2(75, 15));

		renderer->DrawString("X orientation: " + std::to_string(selectionObject->GetTransform().GetOrientation().x), Vector2(55, 20));
		renderer->DrawString("Y orientation: " + std::to_string(selectionObject->GetTransform().GetOrientation().y), Vector2(55, 25));
		renderer->DrawString("Z orientation: " + std::to_string(selectionObject->GetTransform().GetOrientation().z), Vector2(55, 30));

		if (selectionObject->GetName() == "Enemy") {
			if (chase) {
				renderer->DrawString("Enemy chasing", Vector2(70, 35));
			}
			else {
				renderer->DrawString("Enemy wandering", Vector2(70, 35));
			}
		}
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}

	}

	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/
void TutorialGame::MoveSelectedObject() {
	renderer->DrawString("Click Force:" + std::to_string(forceMagnitude),
		Vector2(5, 90)); // Draw debug text at 10 ,20
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;
	
	if (!selectionObject) {
		return;// we haven ’t selected anything !
	}
	// Push the selected object !
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}

		}
	}
}

//class MenuScreen : public PushdownState {
//	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
//		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
//			*newState = new LevelOne();
//			return PushdownResult::Push;
//		}
//		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
//			return PushdownResult::Pop;
//		}
//		return PushdownResult::NoChange;
//	};
//	void OnAwake() override {
//		std::cout << " Welcome to a really awesome game !\n";
//		std::cout << " Press Space To Begin or escape to quit !\n";
//	}
//};
//
//class LevelOne : public PushdownState {
//	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
//		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F3)) {
//			std::cout << " Returning to main menu !\n";
//			return PushdownResult::Pop;
//		}
//		return PushdownResult::NoChange;
//	};
//	void OnAwake() override {
//		std::cout << "Level one!\n";
//		//TutorialGame::InitWorld()
//	}
//protected:
//};
//
//void TestPushdownAutomata(Window* w) {
//	PushdownMachine machine(new MenuScreen());
//	while (w->UpdateWindow()) {
//		float dt = w->GetTimer()->GetTimeDeltaSeconds();
//		if (!machine.Update(dt)) {
//			return;
//		}
//	}
//}

