#include "GameObject.h"
#include "CollisionDetection.h"

using namespace NCL::CSC8503;

GameObject::GameObject(string objectName)	{
	name			= objectName;
	worldID			= -1;
	isActive		= true;
	spinY = false;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	toDelete = false;
	points = 0;
	dead = false;
	game1fin = false;
	Chase = false;
}

GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
}

void GameObject::Update()
{
	if (parent){

		Matrix3 transform = Matrix3(parent->transform.GetOrientation());
		Matrix3 invTransform = Matrix3(parent->transform.GetOrientation().Conjugate());

		this->transform.SetPosition((transform * this->transform.GetOriginalPosition()) + parent->GetTransform().GetPosition()); // Sets position to parents position + this objects original position

		// Changes orientation to match parents
		this->GetTransform().SetOrientation(parent->GetTransform().GetOrientation());
	}

	for (vector<GameObject*>::iterator i = children.begin(); i != children.end(); ++i) {
		(*i)->Update();
	}
}
