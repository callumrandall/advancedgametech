#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

#include "PhysicsObject.h"
#include "RenderObject.h"

#include <vector>

using std::vector;

namespace NCL {
	namespace CSC8503 {

		class GameObject	{
		public:
			GameObject(string name = "");
			~GameObject();

			void SetBoundingVolume(CollisionVolume* vol) {
				boundingVolume = vol;
			}

			const CollisionVolume* GetBoundingVolume() const {
				return boundingVolume;
			}

			bool IsActive() const {
				return isActive;
			}

			Transform& GetTransform() {
				return transform;
			}

			RenderObject* GetRenderObject() const {
				return renderObject;
			}

			PhysicsObject* GetPhysicsObject() const {
				return physicsObject;
			}

			void SetRenderObject(RenderObject* newObject) {
				renderObject = newObject;
			}

			void SetPhysicsObject(PhysicsObject* newObject) {
				physicsObject = newObject;
			}

			const string& GetName() const {
				return name;
			}

			void SetName(string s) {
				name = s;
			}

			virtual void OnCollisionBegin(GameObject* otherObject) {
				//std::cout << "OnCollisionBegin event occured!\n";
				if (name == "Pickup") {
					//std::cout << "Pickup hit" << std::endl;
					toDelete = true;
				}

				if (otherObject->GetName() == "Pickup") {
					//std::cout << "Points gained" << std::endl;
					points++;
					
				}
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				//std::cout << "OnCollisionEnd event occured!\n";
			}

			bool GetBroadphaseAABB(Vector3&outsize) const;

			void UpdateBroadphaseAABB();

			void SetWorldID(int newID) {
				worldID = newID;
			}

			int		GetWorldID() const {
				return worldID;
			}

			void AddChild(GameObject* g) { children.push_back(g); g->parent = this; }
			void Update();
			std::vector<GameObject*>::const_iterator GetChildIteratorStart() { return children.begin(); }
			std::vector<GameObject*>::const_iterator GetChildIteratorEnd() { return children.end(); }

			bool spinY;
			bool toDelete;
			bool dead;
			int points;
			bool game1fin;

			bool Chase;

		protected:
			Transform			transform;

			CollisionVolume*	boundingVolume;
			PhysicsObject*		physicsObject;
			RenderObject*		renderObject;

			bool	isActive;
			int		worldID;
			string	name;

			Vector3 broadphaseAABB;
			GameObject* parent;
			std::vector<GameObject*> children;
		};
	}
}

