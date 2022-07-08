#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "StateGameObject.h"
#include "..//CSC8503Common/PushdownMachine.h"
#include "..//CSC8503Common/PushdownState.h"
#include "..//CSC8503Common/StateMachine.h"
#include "..//CSC8503Common/StateTransition.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);
			Vector3 playerPos();
			Vector3 enemyPos();
			bool world1;
			void moveEnemy(Vector3 start, Vector3 next);
			GameObject* enemy;
			StateMachine* enemyMachine;
			bool chase;
			Vector3 chaseStart;
			Vector3 chaseEnd;
		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();
			void InitWorld2();
			void InitMenu();
			void InitDemo();

			//bool world1;

			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloor();
			void BridgeConstraintTest();
	
			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			float degrees;

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddTiltedFloorToWorld(const Vector3& position, float degrees);
			GameObject* AddOBBToWorld(const Vector3& position, const Vector3& size, float inverseMass);
			GameObject* AddAABBToWorld(const Vector3& position, const Vector3& size, float inverseMass);

			GameObject* AddWallsToFloor(const Vector3& position, const Vector3& size);

			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, bool b = true);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			GameObject* player;
			GameObject* floor;
			GameObject* wallS;
			GameObject* wallE;
			GameObject* wallN;
			GameObject* wallW;
			GameObject* testCube;
			GameObject* rotateSphere;
			GameObject* rotateCube;
			GameObject* spinAnc;
			GameObject* rope;
			GameObject* ropeBall;
			GameObject* ice;
			GameObject* jelly;
			GameObject* finish;

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh*	capsuleMesh = nullptr;
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	charMeshA	= nullptr;
			OGLMesh*	charMeshB	= nullptr;
			OGLMesh*	enemyMesh	= nullptr;
			OGLMesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject = nullptr;
			GameObject* stateAnc;

			float timer;

			GameObject* player2;
			GameObject* floor2;
			float wintimer;
			float deadtimer;
			bool won;
			bool lost;

			bool menu;
		};
	}
}

